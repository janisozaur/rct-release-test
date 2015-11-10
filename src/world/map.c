/*****************************************************************************
 * Copyright (c) 2014 Ted John, Peter Hill
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * This file is part of OpenRCT2.
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "../addresses.h"
#include "../audio/audio.h"
#include "../cheats.h"
#include "../config.h"
#include "../cursors.h"
#include "../game.h"
#include "../interface/window.h"
#include "../localisation/date.h"
#include "../localisation/localisation.h"
#include "../management/finance.h"
#include "../openrct2.h"
#include "../ride/ride_data.h"
#include "../ride/track.h"
#include "../ride/track_data.h"
#include "../scenario.h"
#include "banner.h"
#include "climate.h"
#include "footpath.h"
#include "map.h"
#include "map_animation.h"
#include "park.h"
#include "scenery.h"

/* Replaces 0x00993CCC & 0x00993CCE */
const rct_xy16 TileDirectionDelta[] = {
	{ -32,   0 },
	{   0, +32 },
	{ +32,   0 },
	{   0, -32 },
	{ -32, +32 },
	{ +32, +32 },
	{ +32, -32 },
	{ -32, -32 }
};

rct_map_element *gMapElements = (rct_map_element*)RCT2_ADDRESS_MAP_ELEMENTS;
rct_map_element **gMapElementTilePointers = (rct_map_element**)RCT2_ADDRESS_TILE_MAP_ELEMENT_POINTERS;
rct_xy16 *gMapSelectionTiles = (rct_xy16*)0x009DE596;
rct2_peep_spawn *gPeepSpawns = (rct2_peep_spawn*)RCT2_ADDRESS_PEEP_SPAWNS;

bool gLandMountainMode;
bool gLandPaintMode;
bool LandRightsMode;
bool gClearSmallScenery;
bool gClearLargeScenery;
bool gClearFootpath;

static void tiles_init();
static void update_path_wide_flags(int x, int y);
static void map_update_grass_length(int x, int y, rct_map_element *mapElement);
static void map_set_grass_length(int x, int y, rct_map_element *mapElement, int length);
static void sub_68AE2A(int x, int y);
static void translate_3d_to_2d(int rotation, int *x, int *y);
static void map_obstruction_set_error_text(rct_map_element *mapElement);

void rotate_map_coordinates(sint16 *x, sint16 *y, int rotation)
{
	int temp;

	switch (rotation) {
	case MAP_ELEMENT_DIRECTION_WEST:
		break;
	case MAP_ELEMENT_DIRECTION_NORTH:
		temp = *x;
		*x = *y;
		*y = -temp;
		break;
	case MAP_ELEMENT_DIRECTION_EAST:
		*x = -*x;
		*y = -*y;
		break;
	case MAP_ELEMENT_DIRECTION_SOUTH:
		temp = *y;
		*y = *x;
		*x = -temp;
		break;
	}
}

rct_xy16 coordinate_3d_to_2d(const rct_xyz16* coordinate_3d, int rotation){
	rct_xy16 coordinate_2d;

	switch (rotation){
	case 0:
		coordinate_2d.x = coordinate_3d->y - coordinate_3d->x;
		coordinate_2d.y = (coordinate_3d->y + coordinate_3d->x) / 2 - coordinate_3d->z;
		break;
	case 1:
		coordinate_2d.x = -coordinate_3d->y - coordinate_3d->x;
		coordinate_2d.y = (coordinate_3d->y - coordinate_3d->x) / 2 - coordinate_3d->z;
		break;
	case 2:
		coordinate_2d.x = -coordinate_3d->y + coordinate_3d->x;
		coordinate_2d.y = (-coordinate_3d->y - coordinate_3d->x) / 2 - coordinate_3d->z;
		break;
	case 3:
		coordinate_2d.x = coordinate_3d->y + coordinate_3d->x;
		coordinate_2d.y = (-coordinate_3d->y + coordinate_3d->x) / 2 - coordinate_3d->z;
		break;
	}
	return coordinate_2d;
}

void map_element_iterator_begin(map_element_iterator *it)
{
	it->x = 0;
	it->y = 0;
	it->element = map_get_first_element_at(0, 0);
}

int map_element_iterator_next(map_element_iterator *it)
{
	if (it->element == NULL) {
		it->element = map_get_first_element_at(it->x, it->y);
		return 1;
	}

	if (!map_element_is_last_for_tile(it->element)) {
		it->element++;
		return 1;
	}

	if (it->x < 255) {
		it->x++;
		it->element = map_get_first_element_at(it->x, it->y);
		return 1;
	}

	if (it->y < 255) {
		it->x = 0;
		it->y++;
		it->element = map_get_first_element_at(it->x, it->y);
		return 1;
	}

	return 0;
}

void map_element_iterator_restart_for_tile(map_element_iterator *it)
{
	it->element = NULL;
}

rct_map_element *map_get_first_element_at(int x, int y)
{
	if (x < 0 || y < 0 || x > 255 || y > 255) {
		log_error("Trying to access element outside of range");
		return NULL;
	}
	return TILE_MAP_ELEMENT_POINTER(x + y * 256);
}

void map_set_tile_elements(int x, int y, rct_map_element *elements)
{
	if (x < 0 || y < 0 || x > 255 || y > 255) {
		log_error("Trying to access element outside of range");
		return;
	}
	TILE_MAP_ELEMENT_POINTER(x + y * 256) = elements;
}

int map_element_is_last_for_tile(rct_map_element *element)
{
	return element->flags & MAP_ELEMENT_FLAG_LAST_TILE;
}

int map_element_get_type(rct_map_element *element)
{
	return element->type & MAP_ELEMENT_TYPE_MASK;
}

int map_element_get_direction(rct_map_element *element)
{
	return element->type & MAP_ELEMENT_DIRECTION_MASK;
}

int map_element_get_terrain(rct_map_element *element)
{
	int terrain = (element->properties.surface.terrain >> 5) & 7;
	if (element->type & 1)
		terrain |= (1 << 3);
	return terrain;
}

int map_element_get_terrain_edge(rct_map_element *element)
{
	int terrain_edge = (element->properties.surface.slope >> 5) & 7;
	if (element->type & 128)
		terrain_edge |= (1 << 3);
	return terrain_edge;
}

void map_element_set_terrain(rct_map_element *element, int terrain)
{
	// Bit 3 for terrain is stored in element.type bit 0
	if (terrain & 8)
		element->type |= 1;
	else
		element->type &= ~1;

	// Bits 0, 1, 2 for terrain are stored in element.terrain bit 5, 6, 7
	element->properties.surface.terrain &= ~0xE0;
	element->properties.surface.terrain |= (terrain & 7) << 5;
}

void map_element_set_terrain_edge(rct_map_element *element, int terrain)
{
	// Bit 3 for terrain is stored in element.type bit 7
	if (terrain & 8)
		element->type |= 128;
	else
		element->type &= ~128;

	// Bits 0, 1, 2 for terrain are stored in element.slope bit 5, 6, 7
	element->properties.surface.slope &= ~0xE0;
	element->properties.surface.slope |= (terrain & 7) << 5;
}

rct_map_element *map_get_surface_element_at(int x, int y)
{
	rct_map_element *mapElement = map_get_first_element_at(x, y);

	if (mapElement == NULL)
		return NULL;

	// Find the first surface element
	while (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SURFACE) {
		if (map_element_is_last_for_tile(mapElement))
			return NULL;

		mapElement++;
	}

	return mapElement;
}

rct_map_element* map_get_path_element_at(int x, int y, int z){
	rct_map_element *mapElement = map_get_first_element_at(x, y);

	if (mapElement == NULL)
		return NULL;

	uint8 mapFound = 0;
	// Find the path element at known z
	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_PATH)
			continue;
		if (mapElement->base_height != z)
			continue;

		return mapElement;
	} while (!map_element_is_last_for_tile(mapElement++));

	return NULL;
}
/**
 *
 *  rct2: 0x0068AB4C
 */
void map_init(int size)
{
	int i;
	rct_map_element *map_element;

	date_reset();
	RCT2_GLOBAL(0x0138B580, sint16) = 0;
	RCT2_GLOBAL(0x010E63B8, sint32) = 0;

	for (i = 0; i < MAX_TILE_MAP_ELEMENT_POINTERS; i++) {
		map_element = GET_MAP_ELEMENT(i);
		map_element->type = (MAP_ELEMENT_TYPE_SURFACE << 2);
		map_element->flags = MAP_ELEMENT_FLAG_LAST_TILE;
		map_element->base_height = 14;
		map_element->clearance_height = 14;
		map_element->properties.surface.slope = 0;
		map_element->properties.surface.grass_length = 1;
		map_element->properties.surface.ownership = 0;
		map_element->properties.surface.terrain = 0;

		map_element_set_terrain(map_element, TERRAIN_GRASS);
		map_element_set_terrain_edge(map_element, TERRAIN_EDGE_ROCK);
	}

	RCT2_GLOBAL(RCT2_ADDRESS_GRASS_SCENERY_TILEPOS, sint16) = 0;
	RCT2_GLOBAL(0x013CE774, sint16) = 0;
	RCT2_GLOBAL(0x013CE776, sint16) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16) = size * 32 - 32;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_MINUS_2, sint16) = size * 32 - 2;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE, sint16) = size;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, sint16) = size * 32 - 33;
	RCT2_GLOBAL(0x01359208, sint16) = 7;
	map_update_tile_pointers();
	map_remove_out_of_range_elements();
	climate_reset(CLIMATE_WARM);
}

/**
 *
 *  rct2: 0x0068AFFD
 */
void map_update_tile_pointers()
{
	int i, x, y;

	for (i = 0; i < MAX_TILE_MAP_ELEMENT_POINTERS; i++)
		TILE_MAP_ELEMENT_POINTER(i) = TILE_UNDEFINED_MAP_ELEMENT;

	rct_map_element *mapElement = RCT2_ADDRESS(RCT2_ADDRESS_MAP_ELEMENTS, rct_map_element);
	rct_map_element **tile = RCT2_ADDRESS(RCT2_ADDRESS_TILE_MAP_ELEMENT_POINTERS, rct_map_element*);
	for (y = 0; y < 256; y++) {
		for (x = 0; x < 256; x++) {
			*tile++ = mapElement;
			do { } while (!map_element_is_last_for_tile(mapElement++));
		}
	}

	// Possible next free map element
	RCT2_GLOBAL(0x0140E9A4, rct_map_element*) = mapElement;
}

/**
 * Return the absolute height of an element, given its (x,y) coordinates
 *
 *  ax: x
 *  cx: y
 *  dx: return remember to & with 0xFFFF if you don't want water affecting results
 *  rct2: 0x00662783
 */
int map_element_height(int x, int y)
{
	rct_map_element *mapElement;

	// Off the map
	if ((unsigned)x >= 8192 || (unsigned)y >= 8192)
		return 16;

	// Truncate subtile coordinates
	int x_tile = x & 0xFFFFFFE0;
	int y_tile = y & 0xFFFFFFE0;

	// Get the surface element for the tile
	mapElement = map_get_surface_element_at(x_tile / 32, y_tile / 32);

	uint32 height =
		((mapElement->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK) << 20) |
		(mapElement->base_height << 3);

	uint32 slope = (mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK);
	uint8 extra_height = (slope & 0x10) >> 4; // 0x10 is the 5th bit - sets slope to double height
	// Remove the extra height bit
	slope &= 0xF;

	sint8 quad, quad_extra; // which quadrant the element is in?
	                        // quad_extra is for extra height tiles

	uint8 xl, yl;	    // coordinates across this tile

	uint8 TILE_SIZE = 31;

	xl = x & 0x1f;
	yl = y & 0x1f;

	// Slope logic:
	// Each of the four bits in slope represents that corner being raised
	// slope == 15 (all four bits) is not used and slope == 0 is flat
	// If the extra_height bit is set, then the slope goes up two z-levels

	// We arbitrarily take the SW corner to be closest to the viewer

	// One corner up
	if ((slope == 1) || (slope == 2) || (slope == 4) || (slope == 8)) {
		switch (slope) {
		case 1:    // NE corner up
			quad = xl + yl - TILE_SIZE;
			break;
		case 2:    // SE corner up
			quad = xl - yl;
			break;
		case 4:   // SW corner up
			quad = TILE_SIZE - yl - xl;
			break;
		case 8:   // NW corner up
			quad = yl - xl;
			break;
		}
		// If the element is in the quadrant with the slope, raise its height
		if (quad > 0) {
			height += quad / 2;
		}
	}

	// One side up
	switch (slope) {
	case 3:   // E side up
		height += xl / 2 + 1;
		break;
	case 6:   // S side up
		height += (TILE_SIZE - yl) / 2;
		break;
	case 9:    // N side up
		height += yl / 2;
		height++;
		break;
	case 12:  // W side up
		height += (TILE_SIZE - xl) / 2;
		break;
	}

	// One corner down
	if ((slope == 7) || (slope == 11) || (slope == 13) || (slope == 14)) {
		switch (slope) {
		case 7:   // NW corner down
			quad_extra = xl + TILE_SIZE - yl;
			quad = xl - yl;
			break;
		case 11:  // SW corner down
			quad_extra = xl + yl;
			quad = xl + yl - TILE_SIZE - 1;
			break;
		case 13:  // SE corner down
			quad_extra = TILE_SIZE - xl + yl;
			quad = yl - xl;
			break;
		case 14:  // NE corner down
			quad_extra = (TILE_SIZE - xl) + (TILE_SIZE - yl);
			quad = TILE_SIZE - yl - xl - 1;
			break;
		}

		if (extra_height) {
			height += quad_extra / 2;
			height++;
			return height;
		}
		// This tile is essentially at the next height level
		height += 0x10;
		// so we move *down* the slope
		if (quad < 0) {
			height += quad / 2;
		}
	}

	// Valleys
	if ((slope == 5) || (slope == 10)) {
		switch (slope) {
		case 5:  // NW-SE valley
			if (xl + yl <= TILE_SIZE + 1) {
				return height;
			}
			quad = TILE_SIZE - xl - yl;
			break;
		case 10: // NE-SW valley
			quad = xl - yl;
			break;
		}
		if (quad > 0) {
			height += quad / 2;
		}
	}

	return height;
}

/**
 *
 *  rct2: 0x0068B089
 */
void sub_68B089()
{
	int i;
	rct_map_element *mapElementFirst, *mapElement;

	if (RCT2_GLOBAL(0x009DEA6F, uint8) & 1)
		return;

	i = RCT2_GLOBAL(0x0010E63B8, uint32);
	do {
		i++;
		if (i >= MAX_TILE_MAP_ELEMENT_POINTERS)
			i = 0;
	} while (TILE_MAP_ELEMENT_POINTER(i) == TILE_UNDEFINED_MAP_ELEMENT);
	RCT2_GLOBAL(0x0010E63B8, uint32) = i;

	mapElementFirst = mapElement = TILE_MAP_ELEMENT_POINTER(i);
	do {
		mapElement--;
		if (mapElement < (rct_map_element*)RCT2_ADDRESS_MAP_ELEMENTS)
			break;
	} while (mapElement->base_height == 255);
	mapElement++;

	if (mapElement == mapElementFirst)
		return;

	//
	TILE_MAP_ELEMENT_POINTER(i) = mapElement;
	do {
		*mapElement = *mapElementFirst;
		mapElementFirst->base_height = 255;

		mapElement++;
		mapElementFirst++;
	} while (!map_element_is_last_for_tile(mapElement - 1));

	// Update next free element?
	mapElement = RCT2_GLOBAL(0x0140E9A4, rct_map_element*);
	do {
		mapElement--;
	} while (mapElement->base_height == 255);
	mapElement++;
	RCT2_GLOBAL(0x0140E9A4, rct_map_element*) = mapElement;
}


/**
 * Checks if the tile at coordinate at height counts as connected.
 * @return 1 if connected, 0 otherwise
 */
int map_coord_is_connected(int x, int y, int z, uint8 faceDirection)
{
	rct_map_element *mapElement = map_get_first_element_at(x, y);

	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_PATH)
			continue;

		rct_map_element_path_properties props = mapElement->properties.path;
		uint8 pathType = props.type >> 2;
		uint8 pathDirection = props.type & 3;

		if (pathType & 1) {
			if (pathDirection == faceDirection) {
				if (z == mapElement->base_height + 2)
					return 1;
			} else if ((pathDirection ^ 2) == faceDirection && z == mapElement->base_height) {
				return 1;
			}
		} else {
			if (z == mapElement->base_height)
				return 1;
		}
	} while (!map_element_is_last_for_tile(mapElement++));

	return 0;
}

/**
 *
 *  rct2: 0x006A876D
 */
void sub_6A876D()
{
	int i, x, y;

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & (SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER))
		return;

	// Presumebly update_path_wide_flags is too computationally expensive to call for every
	// tile every update, so word_13CE774 and word_13CE776 store the x and y
	// progress. A maximum of 128 calls is done per update.
	x = RCT2_GLOBAL(0x013CE774, sint16);
	y = RCT2_GLOBAL(0x013CE776, sint16);
	for (i = 0; i < 128; i++) {
		update_path_wide_flags(x, y);

		// Next x, y tile
		x += 32;
		if (x >= 8192) {
			x = 0;
			y += 32;
			if (y >= 8192)
				y = 0;
		}
	}
	RCT2_GLOBAL(0x013CE774, sint16) = x;
	RCT2_GLOBAL(0x013CE776, sint16) = y;
}

/**
 *
 *  rct2: 0x006A8B12
 */
static void sub_6A8B12(int x, int y)
{
	rct_map_element *mapElement = map_get_first_element_at(x / 32, y / 32);
	do {
		if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH) {
			mapElement->type &= ~0x2;
		}
	} while (!map_element_is_last_for_tile(mapElement++));
}

/**
 *
 *  rct2: 0x006A8ACF
 */
static void sub_6A8ACF(int x, int y, uint8 height, rct_map_element **l)
{
	rct_map_element *mapElement = map_get_first_element_at(x / 32, y / 32);
	do {
		if ((map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH) &&
			!footpath_element_is_queue(mapElement) && !footpath_element_is_sloped(mapElement) &&
			(height == mapElement->base_height)) {
			*l = mapElement;
			return;
		}
	} while (!map_element_is_last_for_tile(mapElement++));
	*l = (rct_map_element*)0xFFFFFFFF;
}


/**
 *
 *  rct2: 0x006A87BB
 */
static void update_path_wide_flags(int x, int y)
{
	if (x < 0x20)
		return;
	if (y < 0x20)
		return;
	if (x > 0x1FDF)
		return;
	if (y > 0x1FDF)
		return;

	sub_6A8B12(x, y);
	x += 0x20;
	sub_6A8B12(x, y);
	y += 0x20;
	sub_6A8B12(x, y);
	x -= 0x20;
	sub_6A8B12(x, y);
	y -= 0x20;

	if (!(x & 0xE0))
		return;
	if (!(y & 0xE0))
		return;

	rct_map_element *mapElement = map_get_first_element_at(x / 32, y / 32);
	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_PATH)
			continue;

		if (footpath_element_is_queue(mapElement))
			continue;

		if (footpath_element_is_sloped(mapElement))
			continue;

		uint8 height = mapElement->base_height;

		// pathList is a list of elements, set by sub_6A8ACF adjacent to x,y
		// Spanned from 0x00F3EFA8 to 0x00F3EFC7 (8 elements) in the original
		rct_map_element *pathList[8];

		x -= 0x20;
		y -= 0x20;
		sub_6A8ACF(x, y, height, pathList + 0);
		y += 0x20;
		sub_6A8ACF(x, y, height, pathList + 1);
		y += 0x20;
		sub_6A8ACF(x, y, height, pathList + 2);
		x += 0x20;
		sub_6A8ACF(x, y, height, pathList + 3);
		x += 0x20;
		sub_6A8ACF(x, y, height, pathList + 4);
		y -= 0x20;
		sub_6A8ACF(x, y, height, pathList + 5);
		y -= 0x20;
		sub_6A8ACF(x, y, height, pathList + 6);
		x -= 0x20;
		sub_6A8ACF(x, y, height, pathList + 7);
		y += 0x20;

		uint8 F3EFA5 = 0;
		const rct_map_element *invalidPointer = (rct_map_element*)0xFFFFFFFF;

		if (mapElement->properties.path.edges & 8) {
			F3EFA5 |= 0x80;
			if (pathList[7] != invalidPointer) {
				if (footpath_element_is_wide(pathList[7])) {
					F3EFA5 &= ~0x80;
				}
			}
		}

		if (mapElement->properties.path.edges & 1) {
			F3EFA5 |= 0x2;
			if (pathList[1] != invalidPointer) {
				if (footpath_element_is_wide(pathList[1])) {
					F3EFA5 &= ~0x2;
				}
			}
		}

		if (mapElement->properties.path.edges & 2) {
			F3EFA5 |= 0x8;
			if (pathList[3] != invalidPointer) {
				if (footpath_element_is_wide(pathList[3])) {
					F3EFA5 &= ~0x8;
				}
			}
		}

		if (mapElement->properties.path.edges & 4) {
			F3EFA5 |= 0x20;
			if (pathList[5] != invalidPointer) {
				if (footpath_element_is_wide(pathList[5])) {
					F3EFA5 &= ~0x20;
				}
			}
		}

		if ((F3EFA5 & 0x80) && (pathList[7] != invalidPointer) && !(footpath_element_is_wide(pathList[7])))	{
			if ((F3EFA5 & 2) &&
				(pathList[0] != invalidPointer) && (!footpath_element_is_wide(pathList[0])) &&
				((pathList[0]->properties.path.edges & 6) == 6) && // N E
				(pathList[1] != invalidPointer) && (!footpath_element_is_wide(pathList[1]))) {
				F3EFA5 |= 0x1;
			}

			if ((F3EFA5 & 0x20) &&
				(pathList[6] != invalidPointer) && (!footpath_element_is_wide(pathList[6])) &&
				((pathList[6]->properties.path.edges & 3) == 3) && // N W
				(pathList[5] != invalidPointer) && (!footpath_element_is_wide(pathList[5]))) {
				F3EFA5 |= 0x40;
			}
		}


		if ((F3EFA5 & 0x8) && (pathList[3] != invalidPointer) && !(pathList[3]->type & 2))	{
			if ((F3EFA5 & 2) &&
				(pathList[2] != invalidPointer) && (!footpath_element_is_wide(pathList[2])) &&
				((pathList[2]->properties.path.edges & 0xC) == 0xC) &&
				(pathList[1] != invalidPointer) && (!footpath_element_is_wide(pathList[1]))) {
				F3EFA5 |= 0x4;
			}

			if ((F3EFA5 & 0x20) &&
				(pathList[4] != invalidPointer) && (!footpath_element_is_wide(pathList[4])) &&
				((pathList[4]->properties.path.edges & 9) == 9) &&
				(pathList[5] != invalidPointer) && (!footpath_element_is_wide(pathList[5]))) {
				F3EFA5 |= 0x10;
			}
		}

		if ((F3EFA5 & 0x80) && (F3EFA5 & (0x40 | 0x1)))
			F3EFA5 &= ~0x80;

		if ((F3EFA5 & 0x2) && (F3EFA5 & (0x4 | 0x1)))
			F3EFA5 &= ~0x2;

		if ((F3EFA5 & 0x8) && (F3EFA5 & (0x10 | 0x4)))
			F3EFA5 &= ~0x8;

		if ((F3EFA5 & 0x20) && (F3EFA5 & (0x40 | 0x10)))
			F3EFA5 &= ~0x20;

		if (!(F3EFA5 & (0x2 | 0x8 | 0x20 | 0x80))) {
			uint8 e = mapElement->properties.path.edges;
			if ((e != 0xAF) && (e != 0x5F) && (e != 0xEF))
				mapElement->type |= 2;
		}
	} while (!map_element_is_last_for_tile(mapElement++));
}

/**
 *
 *  rct2: 0x006A7B84
 */
int map_height_from_slope(int x, int y, int slope)
{
	if (!(slope & 4))
		return 0;

	switch (slope & 3) {
	case 0:
		return (31 - (x & 31)) / 2;
	case 1:
		return (y & 31) / 2;
	case 2:
		return (x & 31) / 2;
	case 3:
		return (31 - (y & 31)) / 2;
	}
	return 0;
}

/**
 *
 *  rct2: 0x00664F72
 */
int map_is_location_owned(int x, int y, int z)
{
	rct_map_element *mapElement;

	if (x < (256 * 32) && y < (256 * 32)) {
		mapElement = map_get_surface_element_at(x / 32, y / 32);
		if (mapElement->properties.surface.ownership & OWNERSHIP_OWNED)
			return 1;

		if (mapElement->properties.surface.ownership & OWNERSHIP_CONSTRUCTION_RIGHTS_OWNED) {
			z /= 8;
			if (z < mapElement->base_height || z - 2 > mapElement->base_height)
				return 1;
		}
	}

	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_LAND_NOT_OWNED_BY_PARK;
	return 0;
}

/**
 *
 *  rct2: 0x00664F2C
 */
int map_is_location_in_park(int x, int y)
{
	rct_map_element *mapElement;

	if (x < (256 * 32) && y < (256 * 32)) {
		mapElement = map_get_surface_element_at(x / 32, y / 32);
		if (mapElement == NULL)
			return 0;
		if (mapElement->properties.surface.ownership & OWNERSHIP_OWNED)
			return 1;
	}

	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_LAND_NOT_OWNED_BY_PARK;
	return 0;
}

bool map_is_location_owned_or_has_rights(int x, int y)
{
	rct_map_element *mapElement;

	if (x < (256 * 32) && y < (256 * 32)) {
		mapElement = map_get_surface_element_at(x / 32, y / 32);
		if (mapElement->properties.surface.ownership & OWNERSHIP_OWNED) return true;
		if (mapElement->properties.surface.ownership & OWNERSHIP_CONSTRUCTION_RIGHTS_OWNED) return true;
	}
	return false;
}

/**
 *
 *  rct2: 0x006E0E01
 */
void game_command_remove_scenery(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	int x = *eax;
	int y = *ecx;
	uint8 base_height = *edx;
	uint8 scenery_type = *edx >> 8;
	uint8 map_element_type = *ebx >> 8;
	money32 cost;

	rct_scenery_entry *entry = g_smallSceneryEntries[scenery_type];
	cost = entry->small_scenery.removal_price * 10;

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = base_height * 8;

	if (!(*ebx & 0x40) && RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !(*ebx & 0x40) && !gCheatsSandboxMode) {
		// Check if allowed to remove item
		if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_FORBID_TREE_REMOVAL) {
			if (entry->small_scenery.height > 64) {
				RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_FORBIDDEN_BY_THE_LOCAL_AUTHORITY;
				*ebx = MONEY32_UNDEFINED;
				return;
			}
		}

		// Check if the land is owned
		if (!map_is_location_owned(x, y, RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32))){
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}

	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
	while(map_element->type != map_element_type ||
		map_element->base_height != base_height ||
		map_element->properties.scenery.type != scenery_type ||
		((*ebx & GAME_COMMAND_FLAG_GHOST) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST))){
		map_element++;
		if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
			*ebx = 0;
			return;
		}
	}

	// Remove element
	if (*ebx & GAME_COMMAND_FLAG_APPLY) {
		map_invalidate_tile_full(x, y);
		map_element_remove(map_element);
	}
	*ebx = (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY ? 0 : cost);
}

/**
 *
 *  rct2: 0x006B8E1B
 */
void game_command_remove_large_scenery(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	uint8 base_height = *edx;
	uint8 scenerymultiple_index = *edx >> 8;
	uint8 map_element_direction = *ebx >> 8;
	int x = *eax;
	int y = *ecx;
	int z = map_element_height(x, y);
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = z;
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;

	if (!(*ebx & 0x40) && RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	uint8 element_found = 0;
	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
	do {
		if (map_element_get_type(map_element) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE)
			continue;

		if (map_element->base_height != base_height)
			continue;

		if ((map_element->properties.scenerymultiple.type >> 10) != scenerymultiple_index)
			continue;

		if ((map_element->type & MAP_ELEMENT_DIRECTION_MASK) != map_element_direction)
			continue;

		// If we are removing ghost elements
		if((*ebx & 0x40) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST))
			continue;

		element_found = 1;
		break;
	} while (!map_element_is_last_for_tile(map_element++));

	if (!element_found){
		*ebx = 0;
		return;
	}

	map_element_remove_banner_entry(map_element);

	int ecx2 = map_element->properties.scenerymultiple.type >> 10;
	rct_scenery_entry* scenery_entry = g_largeSceneryEntries[map_element->properties.scenerymultiple.type & 0x3FF];
	rct_xyz16 firstTile = {
		.x = scenery_entry->large_scenery.tiles[ecx2].x_offset,
		.y = scenery_entry->large_scenery.tiles[ecx2].y_offset,
		.z = (base_height * 8) - scenery_entry->large_scenery.tiles[ecx2].z_offset
	};

	rotate_map_coordinates(&firstTile.x, &firstTile.y, map_element_direction);

	firstTile.x = x - firstTile.x;
	firstTile.y = y - firstTile.y;

	for (int i = 0; scenery_entry->large_scenery.tiles[i].x_offset != -1; i++){

		rct_xyz16 currentTile = {
			.x = scenery_entry->large_scenery.tiles[i].x_offset,
			.y = scenery_entry->large_scenery.tiles[i].y_offset,
			.z = scenery_entry->large_scenery.tiles[i].z_offset
		};

		rotate_map_coordinates(&currentTile.x, &currentTile.y, map_element_direction);

		currentTile.x += firstTile.x;
		currentTile.y += firstTile.y;
		currentTile.z += firstTile.z;

		if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode){
			if (!map_is_location_owned(currentTile.x, currentTile.y, currentTile.z)){
				*ebx = MONEY32_UNDEFINED;
				return;
			}
		}

		// If not applying then no need to delete the actual element
		if (!(*ebx & GAME_COMMAND_FLAG_APPLY))
			continue;

		rct_map_element* sceneryElement = map_get_first_element_at(currentTile.x / 32, currentTile.y / 32);
		uint8 tile_not_found = 1;
		do
		{
			if (map_element_get_type(sceneryElement) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE)
				continue;

			if ((sceneryElement->type & MAP_ELEMENT_DIRECTION_MASK) != map_element_direction)
				continue;

			if ((sceneryElement->properties.scenerymultiple.type >> 10) != i)
				continue;

			if (sceneryElement->base_height != currentTile.z / 8)
				continue;

			// If we are removing ghost elements
			if ((*ebx & 0x40) && !(sceneryElement->flags & MAP_ELEMENT_FLAG_GHOST))
				continue;

			map_invalidate_tile_full(currentTile.x, currentTile.y);
			map_element_remove(sceneryElement);
			tile_not_found = 0;
			break;
		} while (!map_element_is_last_for_tile(sceneryElement++));

		if (tile_not_found){
			log_error("Tile not found when trying to remove element!");
		}
	}

	*ebx = scenery_entry->large_scenery.removal_price * 10;
	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY){
		*ebx = 0;
	}
	return;
}

/**
 *
 *  rct2: 0x006BA058
 */
void game_command_remove_banner(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	int x = *eax;
	int y = *ecx;
	uint8 base_height = *edx;
	uint8 banner_position = *edx >> 8;
	int z = base_height * 8;
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
 	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = z;
	if(!(*ebx & 0x40) && RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
		*ebx = MONEY32_UNDEFINED;
		return;
	}
	if(!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && !map_is_location_owned(x, y, z - 16)){
		*ebx = MONEY32_UNDEFINED;
		return;
	}
	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
	while(map_element->type != MAP_ELEMENT_TYPE_BANNER ||
		map_element->properties.banner.position != banner_position){
		map_element++;
		if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}
	rct_banner *banner = &gBanners[map_element->properties.banner.index];
	uint8 bannerType = banner->type;
	if (*ebx & GAME_COMMAND_FLAG_APPLY) {
		map_element_remove_banner_entry(map_element);
		map_invalidate_tile_zoom1(x, y, z, z + 32);
		map_element_remove(map_element);
	}
	rct_scenery_entry *scenery_entry = (rct_scenery_entry*)object_entry_groups[OBJECT_TYPE_BANNERS].chunks[bannerType];
	*ebx = (scenery_entry->banner.price * -3) / 4;
	if(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY){
		*ebx = 0;
	}
}

/**
 *
 *  rct2: 0x006E0F26
 */
void game_command_set_scenery_colour(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	int x = *eax;
	int y = *ecx;
	uint8 base_height = *edx;
	uint8 scenery_type = *edx >> 8;
	uint8 map_element_type = *ebx >> 8;
	uint8 color1 = *ebp;
	uint8 color2 = *ebp >> 8;
	int z = base_height * 8;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = z;
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode){
		if (!map_is_location_owned(x, y, z)){
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}

	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
	while(map_element->type != map_element_type ||
		map_element->base_height != base_height ||
		map_element->properties.scenery.type != scenery_type){
		map_element++;
		if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
			*ebx = 0;
			return;
		}
	}

	if((*ebx & 0x40) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST)){
		*ebx = 0;
		return;
	}
	if(*ebx & GAME_COMMAND_FLAG_APPLY){
		map_element->properties.scenery.colour_1 &= 0xE0;
		map_element->properties.scenery.colour_1 |= color1;
		map_element->properties.scenery.colour_2 &= 0xE0;
		map_element->properties.scenery.colour_2 |= color2;
		map_invalidate_tile_full(x, y);
	}

	*ebx = 0;
}

/**
 *
 *  rct2: 0x006E56B5
 */
void game_command_set_fence_colour(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	int x = *eax;
	int y = *ecx;
	uint8 map_element_direction = *edx;
	uint8 base_height = *edx >> 8;
	uint8 color1 = *ebx >> 8;
	uint8 color2 = *ebp;
	uint8 color3 = *ebp >> 8;
	int z = base_height * 8;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = z;
	if((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) || map_is_location_in_park(x, y) || gCheatsSandboxMode){
		rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
		while(map_element_get_type(map_element) != MAP_ELEMENT_TYPE_FENCE ||
			map_element->base_height != base_height ||
			(map_element->type & MAP_ELEMENT_DIRECTION_MASK) != map_element_direction||
			((*ebx & 0x40) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST))){
			map_element++;
			if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
				*ebx = 0;
				return;
			}
		}
		if((*ebx & 0x40) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST)){
			*ebx = 0;
			return;
		}
		if(*ebx & GAME_COMMAND_FLAG_APPLY){
			rct_scenery_entry* scenery_entry = RCT2_ADDRESS(RCT2_ADDRESS_WALL_SCENERY_ENTRIES, rct_scenery_entry*)[map_element->properties.fence.type];
			map_element->properties.fence.item[1] &= 0xE0;
			map_element->properties.fence.item[1] |= color1;
			map_element->flags &= 0x9F;
			map_element->properties.fence.item[1] &= 0x1F;
			map_element->properties.fence.item[1] |= (color2 & 0x7) * 32;
			map_element->flags |= (color2 & 0x18) * 4;
			if(scenery_entry->wall.flags & 0x80){
				map_element->properties.fence.item[0] = color3;
			}
			map_invalidate_tile_zoom1(x, y, z, z + 0x48);
		}
		*ebx = 0;
	} else {
		*ebx = MONEY32_UNDEFINED;
	}
}

/**
 *
 *  rct2: 0x006B909A
 */
void game_command_set_large_scenery_colour(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	int x = *eax;
	int y = *ecx;
	uint8 map_element_direction = *ebx >> 8;
	uint8 base_height = *edx;
	uint8 scenerymultiple_index = *edx >> 8;
	uint8 color1 = *ebp;
	uint8 color2 = *ebp >> 8;
	int z = map_element_height(x, y);
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = z;


	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
	while(map_element_get_type(map_element) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE ||
		map_element->base_height != base_height ||
		map_element->properties.scenerymultiple.type >> 10 != scenerymultiple_index ||
		(map_element->type & MAP_ELEMENT_DIRECTION_MASK) != map_element_direction){
		map_element++;
		if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
			*ebx = 0;
			return;
		}
	}
	if((*ebx & 0x40) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST)){
		*ebx = 0;
		return;
	}
	int ecx2 = map_element->properties.scenerymultiple.type >> 10;
	rct_scenery_entry* scenery_entry = RCT2_ADDRESS(RCT2_ADDRESS_LARGE_SCENERY_ENTRIES, rct_scenery_entry*)[map_element->properties.scenerymultiple.type & 0x3FF];
	int x2 = scenery_entry->large_scenery.tiles[ecx2].x_offset;
	int y2 = scenery_entry->large_scenery.tiles[ecx2].y_offset;
	int z2 = (base_height * 8) - scenery_entry->large_scenery.tiles[ecx2].z_offset;
	switch(map_element->type & MAP_ELEMENT_DIRECTION_MASK){
		case MAP_ELEMENT_DIRECTION_WEST:
			break;
		case MAP_ELEMENT_DIRECTION_NORTH:{
			int temp = x2;
			x2 = y2;
			y2 = -temp;
			}break;
		case MAP_ELEMENT_DIRECTION_EAST:
			x2 = -x2;
			y2 = -y2;
			break;
		case MAP_ELEMENT_DIRECTION_SOUTH:{
			int temp = y2;
			y2 = x2;
			x2 = -temp;
			}break;
	}
	x2 = -x2 + x;
	y2 = -y2 + y;
	int i = 0;
	while(1){
		if(scenery_entry->large_scenery.tiles[i].x_offset == -1){
			*ebx = 0;
			return;
		}
		int x3 = scenery_entry->large_scenery.tiles[i].x_offset;
		int y3 = scenery_entry->large_scenery.tiles[i].y_offset;
		int z3 = scenery_entry->large_scenery.tiles[i].z_offset;
		switch(map_element->type & MAP_ELEMENT_DIRECTION_MASK){
			case MAP_ELEMENT_DIRECTION_WEST:
				break;
			case MAP_ELEMENT_DIRECTION_NORTH:{
				int temp = x3;
				x3 = y3;
				y3 = -temp;
				}break;
			case MAP_ELEMENT_DIRECTION_EAST:
				x3 = -x3;
				y3 = -y3;
				break;
			case MAP_ELEMENT_DIRECTION_SOUTH:{
				int temp = y3;
				y3 = x3;
				x3 = -temp;
				}break;
		}
		x3 += x2;
		y3 += y2;
		z3 += z2;
		if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode){
			if (!map_is_location_owned(x3, y3, z3)){
				*ebx = MONEY32_UNDEFINED;
				return;
			}
		}

		if(*ebx & GAME_COMMAND_FLAG_APPLY){
			rct_map_element* map_element = map_get_first_element_at(x3 / 32, y3 / 32);
			while(map_element_get_type(map_element) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE ||
				(map_element->type & MAP_ELEMENT_DIRECTION_MASK) != map_element_direction ||
				map_element->properties.scenerymultiple.type >> 10 != i ||
				map_element->base_height != base_height){
				map_element++;
			}
			map_element->properties.scenerymultiple.colour[0] &= 0xE0;
			map_element->properties.scenerymultiple.colour[0] |= color1;
			map_element->properties.scenerymultiple.colour[1] &= 0xE0;
			map_element->properties.scenerymultiple.colour[1] |= color2;
			map_invalidate_tile_full(x3, y3);
		}

		i++;
	}
	*ebx = 0;
}

/**
 *
 *  rct2: 0x006BA16A
 */
void game_command_set_banner_colour(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	int x = *eax;
	int y = *ecx;
	uint8 base_height = *edx;
	uint8 banner_position = *edx >> 8;
	uint8 color = *ebp;
	int z = (base_height * 8);
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = z;

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode){
		if (!map_is_location_owned(x, y, z - 16)){
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}

	if(*ebx & GAME_COMMAND_FLAG_APPLY){
		rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
		while(map_element->type != MAP_ELEMENT_TYPE_BANNER ||
			map_element->properties.banner.position != banner_position){
			map_element++;
			if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
				*ebx = MONEY32_UNDEFINED;
				return;
			}
		}
		rct_window* window = window_find_by_number(WC_BANNER, map_element->properties.banner.index);
		if(window){
			window_invalidate(window);
		}
		gBanners[map_element->properties.banner.index].colour = color;
		map_invalidate_tile_zoom1(x, y, z, z + 32);
	}

	*ebx = 0;
}

// This will cause clear scenery to remove paths
// This should be a flag for the game command which can be set via a checkbox on the clear scenery window.
// #define CLEAR_SCENERY_REMOVES_PATHS

/**
 *
 *  rct2: 0x0068DFE4
 */
money32 map_clear_scenery_from_tile(int x, int y, int clear, int flags)
{
	int type;
	money32 cost, totalCost;
	rct_map_element *mapElement;

	totalCost = 0;

restart_from_beginning:
	mapElement = map_get_first_element_at(x, y);
	do {
		type = map_element_get_type(mapElement);
		switch (type) {
		case MAP_ELEMENT_TYPE_PATH:
			if (clear & (1 << 2)) {
				int eax = x * 32;
				int ebx = flags;
				int ecx = y * 32;
				int edx = mapElement->base_height;
				int edi = 0, ebp = 0;
				cost = game_do_command(eax, ebx, ecx, edx, GAME_COMMAND_REMOVE_PATH, edi, ebp);

				if (cost == MONEY32_UNDEFINED)
					return MONEY32_UNDEFINED;

				totalCost += cost;
				if (flags & 1)
					goto restart_from_beginning;
			} break;
		case MAP_ELEMENT_TYPE_SCENERY:
			if (clear & (1 << 0)) {
				int eax = x * 32;
				int ebx = (mapElement->type << 8) | flags;
				int ecx = y * 32;
				int edx = (mapElement->properties.scenery.type << 8) | (mapElement->base_height);
				int edi = 0, ebp = 0;
				cost = game_do_command(eax, ebx, ecx, edx, GAME_COMMAND_REMOVE_SCENERY, edi, ebp);

				if (cost == MONEY32_UNDEFINED)
					return MONEY32_UNDEFINED;

				totalCost += cost;
				if (flags & 1)
					goto restart_from_beginning;

			} break;
		case MAP_ELEMENT_TYPE_FENCE:
			if (clear & (1 << 0)) {
				int eax = x * 32;
				int ebx = flags;
				int ecx = y * 32;
				int edx = (mapElement->base_height << 8) | (mapElement->type & MAP_ELEMENT_DIRECTION_MASK);
				int edi = 0, ebp = 0;
				cost = game_do_command(eax, ebx, ecx, edx, GAME_COMMAND_REMOVE_FENCE, edi, ebp);

				if (cost == MONEY32_UNDEFINED)
					return MONEY32_UNDEFINED;

				totalCost += cost;
				if (flags & 1)
					goto restart_from_beginning;

			} break;
		case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
			if (clear & (1 << 1)) {
				int eax = x * 32;
				int ebx = flags | ((mapElement->type & MAP_ELEMENT_DIRECTION_MASK) << 8);
				int ecx = y * 32;
				int edx = mapElement->base_height | ((mapElement->properties.scenerymultiple.type >> 10) << 8);
				int edi = 0, ebp = 0;
				cost = game_do_command(eax, ebx, ecx, edx, GAME_COMMAND_REMOVE_LARGE_SCENERY, edi, ebp);

				if (cost == MONEY32_UNDEFINED)
					return MONEY32_UNDEFINED;

				totalCost += cost;
				if (flags & 1)
					goto restart_from_beginning;

			} break;
			break;
		}
	} while (!map_element_is_last_for_tile(mapElement++));

	return totalCost;
}

money32 map_clear_scenery(int x0, int y0, int x1, int y1, int clear, int flags)
{
	int x, y, z;
	money32 totalCost, cost;
	bool noValidTiles;

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;

	x = (x0 + x1) / 2 + 16;
	y = (y0 + y1) / 2 + 16;
	z = map_element_height(x, y);
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = z;

	x0 = max(x0, 32);
	y0 = max(y0, 32);
	x1 = min(x1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));
	y1 = min(y1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));

	noValidTiles = true;
	totalCost = 0;
	for (y = y0; y <= y1; y += 32) {
		for (x = x0; x <= x1; x += 32) {
			if ((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) || gCheatsSandboxMode || map_is_location_owned_or_has_rights(x, y)) {
				cost = map_clear_scenery_from_tile(x / 32, y / 32, clear, flags);
				if (cost != MONEY32_UNDEFINED) {
					noValidTiles = false;
					totalCost += cost;
				}
			} else {
				RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_LAND_NOT_OWNED_BY_PARK;
			}
		}
	}

	return noValidTiles ? MONEY32_UNDEFINED : totalCost;
}

/**
 *
 *  rct2: 0x0068DF91
 */
void game_command_clear_scenery(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	*ebx = map_clear_scenery(
		(sint16)(*eax & 0xFFFF),
		(sint16)(*ecx & 0xFFFF),
		(sint16)(*edi & 0xFFFF),
		(sint16)(*ebp & 0xFFFF),
		*edx,
		*ebx & 0xFF
	);
}

/* rct2: 0x00663CCD */
money32 map_change_surface_style(int x0, int y0, int x1, int y1, uint8 surfaceStyle, uint8 edgeStyle, uint8 flags)
{
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;

	x0 = max(x0, 32);
	y0 = max(y0, 32);
	x1 = min(x1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));
	y1 = min(y1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));

	int xMid, yMid;

	xMid = (x0 + x1) / 2 + 16;
	yMid = (y0 + y1) / 2 + 16;

	int heightMid = map_element_height(xMid, yMid);

	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = xMid;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = yMid;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = heightMid;
	RCT2_GLOBAL(0x009E32B4, uint32) = 0;

	money32 cost = 0;

	if (RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode) {
		cost += RCT2_GLOBAL(0x009E32B4, uint32);
		return (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY) ? 0 : cost;
	}

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_FORBID_LANDSCAPE_CHANGES) {
		cost += RCT2_GLOBAL(0x009E32B4, uint32);
		return (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY) ? 0 : cost;
	}

	for (int x = x0; x <= x1; x += 32) {
		for (int y = y0; y <= y1; y += 32) {
			if (x > 0x1FFF) continue;
			if (y > 0x1FFF) continue;

			if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode) {
				if (!map_is_location_in_park(x, y)) continue;
			}

			rct_map_element* mapElement = map_get_surface_element_at(x / 32, y / 32);

			if (surfaceStyle != 0xFF){
				uint8 cur_terrain = (
					(mapElement->type&MAP_ELEMENT_DIRECTION_MASK) << 3) |
					(mapElement->properties.surface.terrain >> 5);

				if (surfaceStyle != cur_terrain) {
					RCT2_GLOBAL(0x009E32B4, uint32) += RCT2_ADDRESS(0x0097B8B8, uint32)[surfaceStyle & 0x1F];
					if (flags & 1){
						mapElement->properties.surface.terrain &= MAP_ELEMENT_WATER_HEIGHT_MASK;
						mapElement->type &= MAP_ELEMENT_QUADRANT_MASK | MAP_ELEMENT_TYPE_MASK;

						//Save the new terrain
						mapElement->properties.surface.terrain |= surfaceStyle << 5;

						//Save the new direction mask
						mapElement->type |= (surfaceStyle >> 3) & MAP_ELEMENT_DIRECTION_MASK;

						map_invalidate_tile_full(x, y);
						footpath_remove_litter(x, y, map_element_height(x, y));
					}
				}
			}

			if (edgeStyle != 0xFF) {
				uint8 currentEdge =
					((mapElement->type & 0x80) >> 4) |
					(mapElement->properties.surface.slope >> 5);

				if (edgeStyle != currentEdge){
					cost++;

					if (flags & 1){
						mapElement->properties.surface.slope &= MAP_ELEMENT_SLOPE_MASK;
						mapElement->type &= 0x7F;

						//Save edge style
						mapElement->properties.surface.slope |= edgeStyle << 5;

						//Save ???
						mapElement->type |= (edgeStyle << 4) & 0x80;
						map_invalidate_tile_full(x, y);
					}
				}
			}

			if (flags & 1) {
				if (!(mapElement->properties.surface.terrain & MAP_ELEMENT_SURFACE_TERRAIN_MASK)) {
					if (!(mapElement->type & MAP_ELEMENT_DIRECTION_MASK)) {
						if ((mapElement->properties.surface.grass_length & 7) != GRASS_LENGTH_CLEAR_0) {
							mapElement->properties.surface.grass_length = GRASS_LENGTH_CLEAR_0;
							map_invalidate_tile_full(x, y);
						}
					}
				}
			}
		}
	}

	cost *= 100;
	cost += RCT2_GLOBAL(0x009E32B4, uint32);
	return (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY) ? 0 : cost;
}

/* rct2: 0x00663CCD */
void game_command_change_surface_style(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	*ebx = map_change_surface_style(
		(sint16)(*eax & 0xFFFF),
		(sint16)(*ecx & 0xFFFF),
		(sint16)(*edi & 0xFFFF),
		(sint16)(*ebp & 0xFFFF),
		*edx & 0xFF,
		(*edx & 0xFF00) >> 8,
		*ebx & 0xFF
	);
}

//0x00981A1E
const uint8 map_element_raise_styles[5][32] = {
	{ 0x01, 0x1B, 0x03, 0x1B, 0x05, 0x21, 0x07, 0x21, 0x09, 0x1B, 0x0B, 0x1B, 0x0D, 0x21, 0x20, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x23, 0x18, 0x19, 0x1A, 0x3B, 0x1C, 0x29, 0x24, 0x1F },
	{ 0x02, 0x03, 0x17, 0x17, 0x06, 0x07, 0x17, 0x17, 0x0A, 0x0B, 0x22, 0x22, 0x0E, 0x20, 0x22, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x37, 0x18, 0x19, 0x1A, 0x23, 0x1C, 0x28, 0x26, 0x1F },
	{ 0x04, 0x05, 0x06, 0x07, 0x1E, 0x24, 0x1E, 0x24, 0x0C, 0x0D, 0x0E, 0x20, 0x1E, 0x24, 0x1E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x26, 0x18, 0x19, 0x1A, 0x21, 0x1C, 0x2C, 0x3E, 0x1F },
	{ 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x20, 0x1D, 0x1D, 0x28, 0x28, 0x1D, 0x1D, 0x28, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x22, 0x18, 0x19, 0x1A, 0x29, 0x1C, 0x3D, 0x2C, 0x1F },
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x22, 0x20, 0x20, 0x20, 0x21, 0x20, 0x28, 0x24, 0x20 },
};

//0x00981ABE
const uint8 map_element_lower_styles[5][32] = {
	{ 0x2E, 0x00, 0x2E, 0x02, 0x3E, 0x04, 0x3E, 0x06, 0x2E, 0x08, 0x2E, 0x0A, 0x3E, 0x0C, 0x3E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x06, 0x18, 0x19, 0x1A, 0x0B, 0x1C, 0x0C, 0x3E, 0x1F },
	{ 0x2D, 0x2D, 0x00, 0x01, 0x2D, 0x2D, 0x04, 0x05, 0x3D, 0x3D, 0x08, 0x09, 0x3D, 0x3D, 0x0C, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x07, 0x18, 0x19, 0x1A, 0x09, 0x1C, 0x3D, 0x0C, 0x1F },
	{ 0x2B, 0x3B, 0x2B, 0x3B, 0x00, 0x01, 0x02, 0x03, 0x2B, 0x3B, 0x2B, 0x3B, 0x08, 0x09, 0x0A, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x03, 0x18, 0x19, 0x1A, 0x3B, 0x1C, 0x09, 0x0E, 0x1F },
	{ 0x27, 0x27, 0x37, 0x37, 0x27, 0x27, 0x37, 0x37, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x37, 0x18, 0x19, 0x1A, 0x03, 0x1C, 0x0D, 0x06, 0x1F },
	{ 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x0D, 0x0E, 0x00 },
};

static money32 map_set_land_height(int flags, int x, int y, int height, int style, int selectionType)
{
	if (RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
		return MONEY32_UNDEFINED;
	}

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode) {
		if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_FORBID_LANDSCAPE_CHANGES) {
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_FORBIDDEN_BY_THE_LOCAL_AUTHORITY;
			return MONEY32_UNDEFINED;
		}
	}

	if (x > RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16) || y > RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16)) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_OFF_EDGE_OF_MAP;
		return MONEY32_UNDEFINED;
	}

	if (height < 2) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_TOO_LOW;
		return MONEY32_UNDEFINED;
	}

	if (height > 62) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_TOO_HIGH;
		return MONEY32_UNDEFINED;
	} else if (height == 62 && (style & 0x1F) != 0) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_TOO_HIGH;
		return MONEY32_UNDEFINED;
	}

	if (height == 60 && (style & 0x10)) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_TOO_HIGH;
		return MONEY32_UNDEFINED;
	}

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode) {
		if (!map_is_location_in_park(x, y)) {
			return MONEY32_UNDEFINED;
		}
	}

	int eax = x, ebx = flags, ecx = y, edx = (style << 8) | height, esi = 0, edi = selectionType << 5, ebp = 0;
	RCT2_CALLFUNC_X(0x006639FE, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
	return ebx;
}

void game_command_set_land_height(int *eax, int *ebx, int *ecx, int *edx, int *esi, int *edi, int *ebp)
{
	*ebx = map_set_land_height(
		*ebx & 0xFF,
		*eax & 0xFFFF,
		*ecx & 0xFFFF,
		*edx & 0xFF,
		(*edx >> 8) & 0xFF,
		*edi >> 5
	);
}

money32 map_set_land_ownership(uint8 flags, sint16 x1, sint16 y1, sint16 x2, sint16 y2, uint8 newOwnership) {
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LAND_PURCHASE * 4;

	if (!(flags & GAME_COMMAND_FLAG_APPLY))
		return 0;

	RCT2_GLOBAL(0x009E2E28, uint8) = 0;

	for (sint16 y = y1; y <= y2; y += 32) {
		for (sint16 x = x1; x <= x2; x += 32) {
			if (x > RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16))
				continue;
			if (y > RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16))
				continue;

			map_buy_land_rights(x, y, x2, y2, 6, flags | (newOwnership << 8));
		}
	}

	if (!(RCT2_GLOBAL(0x9E2E28, uint8) & 1)) {
		return 0;
	}

	sint16 x = clamp(0, x1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16));
	sint16 y = clamp(0, y1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16));

	x += 16;
	y += 16;

	sint16 z = map_element_height(x, y) & 0xFFFF;
	sound_play_panned(SOUND_PLACE_ITEM, 0x8001, x, y, z);
	return 0;
}

/* rct2: 0x006648E3*/
void game_command_set_land_ownership(int *eax, int *ebx, int *ecx, int *edx, int *esi, int *edi, int *ebp)
{
	*ebx = map_set_land_ownership(
		*ebx & 0xFF,
		*eax & 0xFFFF,
		*ecx & 0xFFFF,
		*edi & 0xFFFF,
		*ebp & 0xFFFF,
		*edx & 0xFF
		);
}


money32 raise_land(int flags, int x, int y, int z, int ax, int ay, int bx, int by, int selectionType)
{
	money32 cost = 0;

	if ((flags & GAME_COMMAND_FLAG_APPLY) && RCT2_GLOBAL(0x009A8C28, uint8) == 1) {
		sound_play_panned(SOUND_PLACE_ITEM, 0x8001, x, y, z);
	}

	uint8 min_height = 0xFF;

	ax = max(ax, 32);
	ay = max(ay, 32);
	bx = min(bx, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));
	by = min(by, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));

	// find lowest map element in selection
	for (int yi = ay; yi <= by; yi += 32) {
		for (int xi = ax; xi <= bx; xi += 32) {
			rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL && min_height > map_element->base_height) {
				min_height = map_element->base_height;
			}
		}
	}

	for (int yi = ay; yi <= by; yi += 32) {
		for (int xi = ax; xi <= bx; xi += 32) {
			rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL) {
				uint8 height = map_element->base_height;
				if (height <= min_height){
					uint8 newStyle = map_element_raise_styles[selectionType][map_element->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK];
					if (newStyle & 0x20) { // needs to be raised
						height += 2;
						newStyle &= ~0x20;
					}
					money32 tileCost = map_set_land_height(flags, xi, yi, height, newStyle, selectionType);
					if (tileCost == MONEY32_UNDEFINED)
						return MONEY32_UNDEFINED;

					cost += tileCost;
				}
			}
		}
	}

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = x;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = y;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = z;
	return cost;
}

money32 lower_land(int flags, int x, int y, int z, int ax, int ay, int bx, int by, int selectionType)
{
	money32 cost = 0;

	if ((flags & GAME_COMMAND_FLAG_APPLY) && RCT2_GLOBAL(0x009A8C28, uint8) == 1) {
		sound_play_panned(SOUND_PLACE_ITEM, 0x8001, x, y, z);
	}

	uint8 max_height = 0;

	ax = max(ax, 32);
	ay = max(ay, 32);
	bx = min(bx, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));
	by = min(by, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));

	// find highest map element in selection
	for (int yi = ay; yi <= by; yi += 32) {
		for (int xi = ax; xi <= bx; xi += 32) {
			rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL) {
				uint8 base_height = map_element->base_height;
				if (map_element->properties.surface.slope & 0xF)
					base_height += 2;
				if (map_element->properties.surface.slope & 0x10)
					base_height += 2;
				if (max_height < base_height)
					max_height = base_height;
			}
		}
	}

	for (int yi = ay; yi <= by; yi += 32) {
		for (int xi = ax; xi <= bx; xi += 32) {
			rct_map_element *map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL) {
				uint8 height = map_element->base_height;
				if (map_element->properties.surface.slope & 0xF)
					height += 2;
				if (map_element->properties.surface.slope & 0x10)
					height += 2;
				if (height >= max_height) {
					height =  map_element->base_height;
					uint8 newStyle = map_element_lower_styles[selectionType][map_element->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK];
					if (newStyle & 0x20) { // needs to be lowered
						height -= 2;
						newStyle &= ~0x20;
					}
					money32 tileCost = map_set_land_height(flags, xi, yi, height, newStyle, selectionType);
					if (tileCost == MONEY32_UNDEFINED)
						return MONEY32_UNDEFINED;

					cost += tileCost;
				}
			}
		}
	}

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = x;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = y;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = z;
	return cost;
}

money32 raise_water(sint16 x0, sint16 y0, sint16 x1, sint16 y1, uint8 flags)
{
	money32 cost = 0;

	uint8 max_height = 0xFF;

	x0 = max(x0, 32);
	y0 = max(y0, 32);
	x1 = min(x1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));
	y1 = min(y1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));

	for (int yi = y0; yi <= y1; yi += 32) {
		for (int xi = x0; xi <= x1; xi += 32) {
			rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL) {
				uint8 height = map_element->base_height;
				if (map_element->properties.surface.terrain & 0x1F)
					height = (map_element->properties.surface.terrain & 0x1F) * 2;
				if (max_height > height)
					max_height = height;
			}
		}
	}

	for (int yi = y0; yi <= y1; yi += 32) {
		for (int xi = x0; xi <= x1; xi += 32) {
			rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL) {
				if (map_element->base_height <= max_height){
					uint8 height = (map_element->properties.surface.terrain & 0x1F);
					if (height != 0) {
						height *= 2;
						if (height > max_height)
							continue;
						height += 2;
					} else {
						height = map_element->base_height + 2;
					}

					money32 tileCost = game_do_command(xi, flags, yi, (max_height << 8) + height, GAME_COMMAND_SET_WATER_HEIGHT, 0, 0);
					if (tileCost == MONEY32_UNDEFINED)
						return MONEY32_UNDEFINED;

					cost += tileCost;
				}
			}
		}
	}

	if (flags & GAME_COMMAND_FLAG_APPLY) {
		int x = ((x0 + x1) / 2) + 16;
		int y = ((y0 + y1) / 2) + 16;
		int z = map_element_height(x, y);
		sint16 water_height_z = z >> 16;
		sint16 base_height_z = z;
		z = water_height_z;
		if (z != 0)
			z = base_height_z;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = x;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = y;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = z;
		sound_play_panned(SOUND_LAYING_OUT_WATER, 0x8001, x, y, z);
	}

	return cost;
}

money32 lower_water(sint16 x0, sint16 y0, sint16 x1, sint16 y1, uint8 flags)
{
	money32 cost = 0;

	uint8 min_height = 0;

	x0 = max(x0, 32);
	y0 = max(y0, 32);
	x1 = min(x1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));
	y1 = min(y1, RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16));

	for (int yi = y0; yi <= y1; yi += 32){
		for (int xi = x0; xi <= x1; xi += 32){
			rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL) {
				uint8 height = map_element->properties.surface.terrain & 0x1F;
				if (height != 0) {
					height *= 2;
					if (height > min_height)
						min_height = height;
				}
			}
		}
	}

	for (int yi = y0; yi <= y1; yi += 32) {
		for (int xi = x0; xi <= x1; xi += 32) {
			rct_map_element* map_element = map_get_surface_element_at(xi / 32, yi / 32);
			if (map_element != NULL) {
				uint8 height = (map_element->properties.surface.terrain & 0x1F);
				if (height != 0) {
					height *= 2;
					if (height < min_height)
						continue;
					height -= 2;
					int tileCost = game_do_command(xi, flags, yi, (min_height << 8) + height, GAME_COMMAND_SET_WATER_HEIGHT, 0, 0);
					if (tileCost == MONEY32_UNDEFINED)
						return MONEY32_UNDEFINED;
					cost += tileCost;
				}
			}
		}
	}

	if (flags & GAME_COMMAND_FLAG_APPLY) {
		int x = ((x0 + x1) / 2) + 16;
		int y = ((y0 + y1) / 2) + 16;
		int z = map_element_height(x, y);
		sint16 water_height_z = z >> 16;
		sint16 base_height_z = z;
		z = water_height_z;
		if (z == 0)
			z = base_height_z;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = x;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = y;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = z;
		sound_play_panned(SOUND_LAYING_OUT_WATER, 0x8001, x, y, z);
	}

	return cost;
}

/**
 *
 *  rct2: 0x0068C542
 */
void game_command_raise_land(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	*ebx = raise_land(
		*ebx,
		*eax,
		*ecx,
		map_element_height(*eax, *ecx),
		(sint16)(*edx & 0xFFFF),
		(sint16)(*ebp & 0xFFFF),
		*edx >> 16,
		*ebp >> 16,
		*edi & 0xFFFF
	);
}

/**
 *
 *  rct2: 0x0068C6D1
 */
void game_command_lower_land(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	*ebx = lower_land(
		*ebx,
		*eax,
		*ecx,
		map_element_height(*eax, *ecx),
		(sint16)(*edx & 0xFFFF),
		(sint16)(*ebp & 0xFFFF),
		*edx >> 16,
		*ebp >> 16,
		*edi & 0xFFFF
	);
}

static int map_get_corner_height(rct_map_element *mapElement, int direction)
{
	int z = mapElement->base_height;
	int slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
	switch (direction) {
	case 0:
		if (slope & 1) {
			z += 2;
			if (slope == 27) {
				z += 2;
			}
		}
		break;
	case 1:
		if (slope & 2) {
			z += 2;
			if (slope == 23) {
				z += 2;
			}
		}
		break;
	case 2:
		if (slope & 4) {
			z += 2;
			if (slope == 30) {
				z += 2;
			}
		}
		break;
	case 3:
		if (slope & 8) {
			z += 2;
			if (slope == 29) {
				z += 2;
			}
		}
		break;
	}
	return z;
}

/**
 *
 *  rct2: 0x0068C3B2 slope 1, style 0
 *  rct2: 0x0068C47A slope 2, style 1
 *  rct2: 0x0068C222 slope 4, style 2
 *  rct2: 0x0068C2EA slope 8, style 3
 */
static money32 smooth_land_tile(int direction, uint8 flags, int x, int y, int targetBaseZ, int minBaseZ)
{
	// Check if inside map bounds
	if (x < 0 || y < 0 || x >= (256 * 32) || y >= (256 * 32)) {
		return MONEY32_UNDEFINED;
	}

	// Get height of tile
	rct_map_element *mapElement = map_get_surface_element_at(x >> 5, y >> 5);
	int baseZ = map_get_corner_height(mapElement, direction);

	// Check if tile is same height as target tile
	if (baseZ == targetBaseZ) {
		// No need to raise or lower
		return MONEY32_UNDEFINED;
	}

	uint8 style;
	if (targetBaseZ <= baseZ) {
		baseZ = baseZ - targetBaseZ;
		if (baseZ <= minBaseZ) {
			return MONEY32_UNDEFINED;
		}
		targetBaseZ = mapElement->base_height;
		int slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
		style = map_element_lower_styles[direction][slope];
		if (style & 0x20) {
			targetBaseZ -= 2;
			style &= ~0x20;
		}
	} else {
		baseZ = targetBaseZ - baseZ;
		if (baseZ <= minBaseZ) {
			return MONEY32_UNDEFINED;
		}
		targetBaseZ = mapElement->base_height;
		int slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
		style = map_element_raise_styles[direction][slope];
		if ((style & 0x20) != 0) {
			targetBaseZ += 2;
			style &= ~0x20;
		}
	}

	return game_do_command(x, flags, y, targetBaseZ | (style << 8), GAME_COMMAND_SET_LAND_HEIGHT, 0, 0);
}

money32 smooth_land(int flags, int centreX, int centreY, int mapLeft, int mapTop, int mapRight, int mapBottom, int command)
{
	// Cap bounds to map
	mapLeft = max(mapLeft, 32);
	mapTop = max(mapTop, 32);
	mapRight = min(mapRight, 255 * 32);
	mapBottom = min(mapBottom, 255 * 32);

	int commandType;
	int centreZ = map_element_height(centreX, centreY);
	int mapLeftRight = mapLeft | (mapRight << 16);
	int mapTopBottom = mapTop | (mapBottom << 16);

	// Play sound (only once)
	if ((flags & GAME_COMMAND_FLAG_APPLY) && RCT2_GLOBAL(0x009A8C28, uint8) == 1) {
		sound_play_panned(SOUND_PLACE_ITEM, 0x8001, centreX, centreY, centreZ);
	}

	money32 totalCost = 0;

	// First raise / lower the centre tile
	money32 result;
	commandType = command == 1 ? GAME_COMMAND_RAISE_LAND : GAME_COMMAND_LOWER_LAND;
	result = game_do_command(centreX, flags, centreY, mapLeftRight, commandType, 4, mapTopBottom);
	if (result != MONEY32_UNDEFINED) {
		totalCost += result;
	}

	int x, y, z;
	rct_map_element *mapElement;

	x = mapLeft;
	y = mapTop;
	mapElement = map_get_surface_element_at(x >> 5, y >> 5);
	int slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
	if (slope != 0) {
		commandType = command == 0xFFFF ? GAME_COMMAND_RAISE_LAND : GAME_COMMAND_LOWER_LAND;
		result = game_do_command(centreX, flags, centreY, mapLeftRight, commandType, 4, mapTopBottom);
		if (result != MONEY32_UNDEFINED) {
			totalCost += result;
		}

		x = mapLeft;
		y = mapTop;
		mapElement = map_get_surface_element_at(x >> 5, y >> 5);
		slope = mapElement->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
		if (slope != 0) {
			RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
			RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = centreX;
			RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = centreY;
			RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = centreZ;
			return totalCost * 4;
		}
	}

	x = mapLeft;
	y = mapTop;
	int size = ((mapRight - mapLeft) >> 5) + 1;
	int initialMinZ = -2;

	for (; size <= 256; size += 2) {
		initialMinZ += 2;
		int minZ = initialMinZ * 2;
		x -= 32;
		y -= 32;

		// Corner (North-West)
		mapElement = map_get_surface_element_at(mapLeft >> 5, mapTop >> 5);
		z = map_get_corner_height(mapElement, 2);
		result = smooth_land_tile(0, flags, x, y, z, minZ);
		if (result != MONEY32_UNDEFINED) {
			totalCost += result;
		}
		y += 32;

		// Side (West)
		for (int i = 0; i < size; i++) {
			int y2 = clamp(mapTop, y, mapBottom);
			mapElement = map_get_surface_element_at(mapLeft >> 5, y2 >> 5);
			z = map_get_corner_height(mapElement, 2);
			result = smooth_land_tile(1, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}
			minZ -= 2;
			if (y >= mapTop) {
				minZ += 2;
				if (y > mapBottom) {
					minZ += 2;
				}
			}
			z = map_get_corner_height(mapElement, 3);
			result = smooth_land_tile(0, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}

			y += 32;
		}

		// Corner (South-West)
		mapElement = map_get_surface_element_at(mapLeft >> 5, mapBottom >> 5);
		z = map_get_corner_height(mapElement, 3);
		result = smooth_land_tile(1, flags, x, y, z, minZ);
		if (result != MONEY32_UNDEFINED) {
			totalCost += result;
		}
		x += 32;

		// Side (South)
		for (int i = 0; i < size; i++) {
			int x2 = clamp(mapLeft, x, mapRight);
			mapElement = map_get_surface_element_at(x2 >> 5, mapBottom >> 5);
			z = map_get_corner_height(mapElement, 3);
			result = smooth_land_tile(2, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}
			minZ -= 2;
			if (x >= mapLeft) {
				minZ += 2;
				if (x > mapRight) {
					minZ += 2;
				}
			}
			z = map_get_corner_height(mapElement, 0);
			result = smooth_land_tile(1, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}

			x += 32;
		}

		// Corner (South-East)
		mapElement = map_get_surface_element_at(mapRight >> 5, mapBottom >> 5);
		z = map_get_corner_height(mapElement, 0);
		result = smooth_land_tile(2, flags, x, y, z, minZ);
		if (result != MONEY32_UNDEFINED) {
			totalCost += result;
		}
		y -= 32;

		// Side (East)
		for (int i = 0; i < size; i++) {
			int y2 = clamp(mapTop, y, mapBottom);
			mapElement = map_get_surface_element_at(mapRight >> 5, y2 >> 5);
			z = map_get_corner_height(mapElement, 0);
			result = smooth_land_tile(3, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}
			minZ -= 2;
			if (y <= mapBottom) {
				minZ += 2;
				if (y < mapTop) {
					minZ += 2;
				}
			}
			z = map_get_corner_height(mapElement, 1);
			result = smooth_land_tile(2, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}

			y -= 32;
		}

		// Corner (North-East)
		mapElement = map_get_surface_element_at(mapRight >> 5, mapTop >> 5);
		z = map_get_corner_height(mapElement, 1);
		result = smooth_land_tile(3, flags, x, y, z, minZ);
		if (result != MONEY32_UNDEFINED) {
			totalCost += result;
		}
		x -= 32;

		// Side (North)
		for (int i = 0; i < size; i++) {
			int x2 = clamp(mapLeft, x, mapRight);
			mapElement = map_get_surface_element_at(x2 >> 5, mapTop >> 5);
			z = map_get_corner_height(mapElement, 1);
			result = smooth_land_tile(0, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}
			minZ -= 2;
			if (x <= mapRight) {
				minZ += 2;
				if (x < mapLeft) {
					minZ += 2;
				}
			}
			z = map_get_corner_height(mapElement, 2);
			result = smooth_land_tile(3, flags, x, y, z, minZ);
			if (result != MONEY32_UNDEFINED) {
				totalCost += result;
			}

			x -= 32;
		}
	}

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = centreX;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = centreY;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = centreZ;
	return totalCost * 4;
}

/**
 *
 *  rct2: 0x0068BC01
 */
void game_command_smooth_land(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	int flags = *ebx & 0xFF;
	int centreX = *eax & 0xFFFF;
	int centreY = *ecx & 0xFFFF;
	int mapLeft = (sint16)(*edx & 0xFFFF);
	int mapTop = (sint16)(*ebp & 0xFFFF);
	int mapRight = (sint16)(*edx >> 16);
	int mapBottom = (sint16)(*ebp >> 16);
	int command = *edi;
	*ebx = smooth_land(flags, centreX, centreY, mapLeft, mapTop, mapRight, mapBottom, command);
}

/**
 *
 *  rct2: 0x006E66A0
 */
void game_command_raise_water(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	*ebx = raise_water(
		(sint16)(*eax & 0xFFFF),
		(sint16)(*ecx & 0xFFFF),
		(sint16)(*edi & 0xFFFF),
		(sint16)(*ebp & 0xFFFF),
		(uint8)*ebx
	);
}

/**
 *
 *  rct2: 0x006E6878
 */
void game_command_lower_water(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	*ebx = lower_water(
		(sint16)(*eax & 0xFFFF),
		(sint16)(*ecx & 0xFFFF),
		(sint16)(*edi & 0xFFFF),
		(sint16)(*ebp & 0xFFFF),
		(uint8)*ebx
	);
}

/**
 *
 *  rct2: 0x006E650F
 */
void game_command_set_water_height(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	int x = *eax;
	int y = *ecx;
	uint8 base_height = *edx;
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, sint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, sint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = base_height * 8;
	if(RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
		*ebx = MONEY32_UNDEFINED;
		return;
	}
	if(!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_FORBID_LANDSCAPE_CHANGES){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_FORBIDDEN_BY_THE_LOCAL_AUTHORITY;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if(base_height < 2){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_TOO_LOW;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if(base_height >= 58){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_TOO_HIGH;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if(x >= RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16) || y >= RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16)){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_OFF_EDGE_OF_MAP;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if(!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && !map_is_location_in_park(x, y)){
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if(*ebx & GAME_COMMAND_FLAG_APPLY){
		int element_height = map_element_height(x, y);
		footpath_remove_litter(x, y, element_height);
		map_remove_walls_at_z(x, y, element_height);
	}

	rct_map_element* map_element = map_get_surface_element_at(x / 32, y / 32);
	int z1 = map_element->base_height;
	int z2 = base_height;
	if(map_element->properties.surface.terrain & 0x1F){
		z1 = (map_element->properties.surface.terrain & 0x1F) * 2;
	}
	if(z2 > z1){
		int temp = z1;
		z1 = z2;
		z2 = temp;
	}

	if(map_can_construct_at(x, y, z1, z2, 0xFF)){
		if(map_element->type & 0x40){
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 0;
			*ebx = MONEY32_UNDEFINED;
			return;
		}
		if(*ebx & GAME_COMMAND_FLAG_APPLY){
			int new_terrain = map_element->properties.surface.terrain & 0xE0;
			if(base_height > map_element->base_height){
				new_terrain |= (base_height / 2);
			}
			map_element->properties.surface.terrain = new_terrain;
			map_invalidate_tile_full(x, y);
		}
		*ebx = 250;
		if(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY){
			*ebx = 0;
		}
	}else{
		*ebx = MONEY32_UNDEFINED;
	}
}

/**
 *
 *  rct2: 0x006E5597
 */
void game_command_remove_fence(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	int x = *eax;
	int y = *ecx;
	uint8 base_height = (*edx >> 8);
	uint8 direction = *edx;

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	if(!(*ebx & 0x40) && RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
		*ebx = MONEY32_UNDEFINED;
		return;
	}
	if(!(*ebx & 0x40) && !(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && !map_is_location_owned(x, y, base_height * 8)){
		*ebx = MONEY32_UNDEFINED;
		return;
	}
	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
	while(map_element_get_type(map_element) != MAP_ELEMENT_TYPE_FENCE ||
	map_element->base_height != base_height ||
	(map_element->type & MAP_ELEMENT_DIRECTION_MASK) != direction ||
	((*ebx & 0x40) && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST))){
		map_element++;
		if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
			*ebx = 0;
			return;
		}
	}

	if (!(*ebx & GAME_COMMAND_FLAG_APPLY)){
		*ebx = 0;
		return;
	}

	map_element_remove_banner_entry(map_element);
	map_invalidate_tile_zoom1(x, y, map_element->base_height * 8, (map_element->base_height * 8) + 72);
	map_element_remove(map_element);
	*ebx = 0;
}

/**
 *
 *  rct2: 0x006B9E6D
 */
void game_command_place_banner(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	int x = (uint16)*eax;
	int y = (uint16)*ecx;
	uint8 base_height = *edx;
	uint8 edge = *edx >> 8;
	uint8 colour = *edi;
	uint8 type = *ebx >> 8;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint32) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint32) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint32) = base_height * 16;
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	if(RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) == 0 || gConfigCheat.build_in_pause_mode){
		if(sub_68B044() && x < 8192 && y < 8192){
			rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
			int dl = base_height * 2;
			int ch = (base_height - 1) * 2;
			while(map_element_get_type(map_element) != MAP_ELEMENT_TYPE_PATH ||
			(map_element->base_height != dl && map_element->base_height != ch) ||
			!(map_element->properties.path.edges & (1 << edge))){
				map_element++;
				if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
					RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CAN_ONLY_BE_BUILT_ACROSS_PATHS;
					*ebx = MONEY32_UNDEFINED;
					return;
				}
			}
			if(!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && !map_is_location_owned(x, y, base_height * 16)){
				*ebx = MONEY32_UNDEFINED;
				return;
			}
			map_element = map_get_first_element_at(x / 32, y / 32);
			dl = (base_height + 1) * 2;
			while(map_element->type != MAP_ELEMENT_TYPE_BANNER ||
			map_element->base_height != dl ||
			(map_element->properties.banner.position & 0x3) != edge){
				map_element++;
				if((map_element - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE){
					int banner_index = create_new_banner(*ebx);
					if(banner_index == BANNER_NULL){
						*ebx = MONEY32_UNDEFINED;
						return;
					}
					*edi = banner_index;
					if(*ebx & GAME_COMMAND_FLAG_APPLY){
						rct_map_element* new_map_element = map_element_insert(x / 32, y / 32, (base_height + 1) * 2, 0);
						gBanners[banner_index].type = type;
						gBanners[banner_index].colour = colour;
						gBanners[banner_index].x = x / 32;
						gBanners[banner_index].y = y / 32;
						new_map_element->type = MAP_ELEMENT_TYPE_BANNER;
						new_map_element->clearance_height = new_map_element->base_height + 2;
						new_map_element->properties.banner.position = edge;
						new_map_element->properties.banner.flags = 0xFF;
						new_map_element->properties.banner.unused = 0;
						new_map_element->properties.banner.index = banner_index;
						if(*ebx & 0x40){
							new_map_element->flags |= 0x10;
						}
						map_invalidate_tile_full(x, y);
						map_animation_create(0x0A, x, y, new_map_element->base_height);
					}
					rct_scenery_entry *scenery_entry = (rct_scenery_entry*)object_entry_groups[OBJECT_TYPE_BANNERS].chunks[type];
					*ebx = scenery_entry->banner.price;
					if(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY){
						*ebx = 0;
					}
					return;
				}
			}
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_BANNER_SIGN_IN_THE_WAY;
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}else{
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
	}
	*ebx = MONEY32_UNDEFINED;
}

/**
 *
 *  rct2: 0x006E08F4
 */
void game_command_place_scenery(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	int x = (uint16)*eax;
	int y = (uint16)*ecx;
	uint8 color2 = *edi >> 16;
	uint8 rotation = *edi;
	int z = *ebp;
	uint8 scenery_type = *ebx >> 8;
	uint8 quadrant = *edx;
	uint8 color1 = *edx >> 8;
	int F64F1D = 0;
	int F64EC8 = z;
	int base_height = map_element_height(x, y);
	if(base_height & 0xFFFF0000){
		base_height >>= 16;
	}
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) = x;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) = y;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = base_height;
	if(F64EC8){
		base_height = F64EC8;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, uint16) = base_height;
	}
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, uint16) += 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, uint16) += 16;
	if(RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) == 0 || gConfigCheat.build_in_pause_mode){
		if(sub_68B044()){
			if(RCT2_GLOBAL(0x009D8150, uint8) & 1 || (x <= RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16) && y <= RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16))){
				rct_scenery_entry* scenery_entry = (rct_scenery_entry*)object_entry_groups[OBJECT_TYPE_SMALL_SCENERY].chunks[scenery_type];
				if(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG_FULL_TILE || !(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG9)){
					if(scenery_entry->small_scenery.flags & (SMALL_SCENERY_FLAG9 | SMALL_SCENERY_FLAG24 | SMALL_SCENERY_FLAG25)){
						quadrant = 0;
					}
				}
				int x2 = x;
				int y2 = y;
				if(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG_FULL_TILE){
					x2 += 16;
					y2 += 16;
				}else{
					x2 += RCT2_ADDRESS(0x009A3E74, uint8)[(quadrant & 3) * 2] - 1;
					y2 += RCT2_ADDRESS(0x009A3E75, uint8)[(quadrant & 3) * 2] - 1;
				}
				int base_height2 = map_element_height(x2, y2);
				if(base_height2 & 0xFFFF0000){
					base_height2 >>= 16;
					if(F64EC8 == 0){
						F64F1D = 1;
					}
				}
				if(F64EC8 == 0){
					F64EC8 = base_height2;
				}
				if(!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode && !map_is_location_owned(x, y, F64EC8)){
					*ebx = MONEY32_UNDEFINED;
					return;
				}
				if(*ebx & GAME_COMMAND_FLAG_APPLY && !(*ebx & 0x40)){
					footpath_remove_litter(x, y, F64EC8);
					if(!gCheatsDisableClearanceChecks && (scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG_ALLOW_WALLS)) {
						map_remove_walls_at(x, y, F64EC8, F64EC8 + scenery_entry->small_scenery.height);
					}
				}
				rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
				while(map_element_get_type(map_element) != MAP_ELEMENT_TYPE_SURFACE){
					map_element++;
				}
				if(!gCheatsDisableClearanceChecks && (map_element->properties.surface.terrain & 0x1F)){
					int water_height = ((map_element->properties.surface.terrain & 0x1F) * 16) - 1;
					if(water_height > F64EC8){
						RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CANT_BUILD_THIS_UNDERWATER;
						*ebx = MONEY32_UNDEFINED;
						return;
					}
				}
				if(!gCheatsDisableClearanceChecks && !(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG18)){
					if(F64F1D != 0){
						RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CAN_ONLY_BUILD_THIS_ON_LAND;
						*ebx = MONEY32_UNDEFINED;
						return;
					}
					if(map_element->properties.surface.terrain & 0x1F){
						if(((map_element->properties.surface.terrain & 0x1F) * 16) > F64EC8){
							RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CAN_ONLY_BUILD_THIS_ON_LAND;
							*ebx = MONEY32_UNDEFINED;
							return;
						}
					}
				}
				if(gCheatsDisableClearanceChecks || !(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG_REQUIRE_FLAT_SURFACE) || z != 0 || F64F1D != 0 || !(map_element->properties.surface.slope & 0x1F)){
					if(gCheatsDisableSupportLimits || scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG18 || z == 0){
					l_6E0B78: ;
						int bp = quadrant;
						int zLow = F64EC8 / 8;
						int zHigh = zLow + ((scenery_entry->small_scenery.height + 7) / 8);
						int bl = 0xF;
						if(!(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG_FULL_TILE)){
							bp ^= 2;
							bl = 1;
							bl <<= bp;
						}
						if(!(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG24)){
							if(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG9 && scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG_FULL_TILE){
								if(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG25){
									bp ^= 2;
									bp += rotation;
									bp &= 3;
									bl = 0xBB;
									bl = rol8(bl, bp);
									bl &= 0xF;
								}else{
									bp += rotation;
									bp &= 1;
									bl = 0xA;
									bl >>= bp;
								}
							}
						}else{
							bp ^= 2;
							bp += rotation;
							bp &= 3;
							bl = 0x33;
							bl = rol8(bl, bp);
							bl &= 0xF;
						}
						if(z == 0){
							bl |= 0xF0;
						}
						RCT2_GLOBAL(0x00F64F22, uint16) = x;
						RCT2_GLOBAL(0x00F64F24, uint16) = y;
						RCT2_GLOBAL(0x00F64F1E, uint32) = (uint32)(ebx - 1); //0x006E0D6E uses [F64F1E+4] to read ebx value
						if(gCheatsDisableClearanceChecks || map_can_construct_with_clear_at(x, y, zLow, zHigh, (void*)0x006E0D6E, bl)){
							RCT2_GLOBAL(0x00F64F14, uint8) = RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) & 0x3;
							if(*ebx & GAME_COMMAND_FLAG_APPLY){
								int flags = (bl & 0xf);
								rct_map_element* new_map_element = map_element_insert(x / 32, y / 32, zLow, flags);
								RCT2_GLOBAL(RCT2_ADDRESS_SCENERY_MAP_ELEMENT, rct_map_element*) = new_map_element;
								uint8 type = quadrant << 6;
								type |= MAP_ELEMENT_TYPE_SCENERY;
								type |= rotation;
								new_map_element->type = type;
								new_map_element->properties.scenery.type = scenery_type;
								new_map_element->properties.scenery.age = 0;
								new_map_element->properties.scenery.colour_1 = color1;
								new_map_element->properties.scenery.colour_2 = color2;
								new_map_element->clearance_height = new_map_element->base_height + ((scenery_entry->small_scenery.height + 7) / 8);
								if(z != 0){
									new_map_element->properties.scenery.colour_1 |= 0x20;
								}
								if(*ebx & 0x40){
									new_map_element->flags |= 0x10;
								}
								map_invalidate_tile_full(x, y);
								if(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG_ANIMATED){
									map_animation_create(2, x, y, new_map_element->base_height);
								}
							}
							*ebx = (scenery_entry->small_scenery.price * 10);
							if(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY){
								*ebx = 0;
							}
							return;
						}
					}else{
						if(F64F1D == 0){
							if((map_element->properties.surface.terrain & 0x1F) || (map_element->base_height * 8) != F64EC8){
								RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_LEVEL_LAND_REQUIRED;
							}else{
								goto l_6E0B78;
							}
						}else{
							RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CAN_ONLY_BUILD_THIS_ON_LAND;
						}
					}
				}else{
					RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_LEVEL_LAND_REQUIRED;
				}
			}
		}
	}else{
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
	}
	*ebx = MONEY32_UNDEFINED;
}

static bool map_is_location_at_edge(int x, int y)
{
	return x < 32 || y < 32 || x >= ((256 - 1) * 32) || y >= ((256 - 1) * 32);
}

/**
 *
 *  rct2: 0x006E5CBA
 */
static bool map_place_fence_check_obstruction_with_track(rct_scenery_entry *wall, int x, int y, int z0, int z1, int edge, rct_map_element *trackElement)
{
	const rct_preview_track *trackBlock;
	int z, direction;

	int trackType = trackElement->properties.track.type;
	int sequence = trackElement->properties.track.sequence & 0x0F;
	int typeAndSequence = (trackType << 4) | sequence;
	direction = (edge - trackElement->type) & 3;
	rct_ride *ride = GET_RIDE(trackElement->properties.track.ride_index);

	if (ride_type_has_flag(ride->type, RIDE_TYPE_FLAG_FLAT_RIDE)) {
		if (RCT2_ADDRESS(0x0099AA94, uint8)[typeAndSequence] & (1 << direction)) {
			if (!ride_type_has_flag(ride->type, RIDE_TYPE_FLAG_18)) {
				return true;
			}
		}
	} else {
		if (RCT2_ADDRESS(0x00999A94, uint8)[typeAndSequence] & (1 << direction)) {
			if (!ride_type_has_flag(ride->type, RIDE_TYPE_FLAG_18)) {
				return true;
			}
		}
	}

	if (!(wall->wall.flags & WALL_SCENERY_FLAG5)) {
		return false;
	}

	if (!(RideData4[ride->type].flags & RIDE_TYPE_FLAG4_0)) {
		return false;
	}

	rct_ride_type *rideEntry = GET_RIDE_ENTRY(ride->subtype);
	if (rideEntry->flags & RIDE_ENTRY_FLAG_16) {
		return false;
	}

	RCT2_GLOBAL(0x0141F725, uint8) |= 1;
	if (z0 & 1) {
		return false;
	}

	if (sequence == 0) {
		if (RCT2_GLOBAL(0x0099BA64 + (trackType * 16), uint8) & 0x40) {
			return false;
		}

		if (gTrackDefinitions[trackType].bank_start == 0) {
			if (!(RCT2_ADDRESS(0x009968BB, uint8)[trackType * 10] & 4)) {
				direction = (trackElement->type & 3) ^ 2;
				if (direction == edge) {
					trackBlock = &TrackBlocks[trackType][sequence];
					z = RCT2_GLOBAL(0x009968BD + (trackType * 10), uint8);
					z = trackElement->base_height + ((z - trackBlock->z) * 8);
					if (z == z0) {
						return true;
					}
				}
			}
		}
	}

	trackBlock = &TrackBlocks[trackType][sequence + 1];
	if (trackBlock->index != 0xFF) {
		return false;
	}

	if (gTrackDefinitions[trackType].bank_end != 0) {
		return false;
	}

	direction = RCT2_ADDRESS(0x009968BC, uint8)[trackType * 10];
	if (direction & 4) {
		return false;
	}

	direction = (trackElement->type + direction) & 3;
	if (direction != edge) {
		return false;
	}

	trackBlock = &TrackBlocks[trackType][sequence];
	z = RCT2_GLOBAL(0x009968BF + (trackType * 10), uint8);
	z = trackElement->base_height + ((z - trackBlock->z) * 8);
	if (z != z0) {
		return false;
	}

	return true;
}

/**
 *
 *  rct2: 0x006E5C1A
 */
static bool map_place_fence_check_obstruction(rct_scenery_entry *wall, int x, int y, int z0, int z1, int edge)
{
	int entryType, sequence;
	rct_scenery_entry *entry;
	rct_large_scenery_tile *tile;

	RCT2_GLOBAL(0x0141F725, uint8) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) = 1;
	if (map_is_location_at_edge(x, y)) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_OFF_EDGE_OF_MAP;
		return false;
	}

	rct_map_element *mapElement = map_get_first_element_at(x / 32, y / 32);
	do {
		int elementType = map_element_get_type(mapElement);
		if (elementType == MAP_ELEMENT_TYPE_SURFACE) continue;
		if (z0 >= mapElement->clearance_height) continue;
		if (z1 <= mapElement->base_height) continue;
		if (elementType == MAP_ELEMENT_TYPE_FENCE) {
			int direction = mapElement->type & 3;
			if (edge == direction) {
				map_obstruction_set_error_text(mapElement);
				return false;
			}
			continue;
		}
		if ((mapElement->flags & 0x0F) == 0) continue;

		switch (elementType) {
		case MAP_ELEMENT_TYPE_ENTRANCE:
			map_obstruction_set_error_text(mapElement);
			return false;
		case MAP_ELEMENT_TYPE_PATH:
			if (mapElement->properties.path.edges & (1 << edge)) {
				map_obstruction_set_error_text(mapElement);
				return false;
			}
			break;
		case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
			entryType = mapElement->properties.scenerymultiple.type & 0x3FF;
			sequence = mapElement->properties.scenerymultiple.type >> 10;
			entry = g_largeSceneryEntries[entryType];
			tile = &entry->large_scenery.tiles[sequence];

			int direction = ((edge - mapElement->type) & 3) + 8;
			if (!(tile->var_7 & (1 << direction))) {
				map_obstruction_set_error_text(mapElement);
				return false;
			}
			break;
		case MAP_ELEMENT_TYPE_SCENERY:
			entryType = mapElement->properties.scenery.type;
			entry = g_smallSceneryEntries[entryType];
			if (entry->small_scenery.flags & SMALL_SCENERY_FLAG_ALLOW_WALLS) {
				map_obstruction_set_error_text(mapElement);
				return false;
			}
			break;
		case MAP_ELEMENT_TYPE_TRACK:
			if (!map_place_fence_check_obstruction_with_track(wall, x, y, z0, z1, edge, mapElement)) {
				return false;
			}
			break;
		}
	} while (!map_element_is_last_for_tile(mapElement++));
	return true;
}

/**
 *
 *  rct2: 0x006E519A
 */
void game_command_place_fence(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp){
	rct_xyz16 position = {
		.x = *eax & 0xFFFF,
		.y = *ecx & 0xFFFF,
		.z = *edi & 0xFFFF
	};

	uint8 flags = *ebx & 0xFF;
	uint8 fence_type = (*ebx >> 8) & 0xFF;
	uint8 primary_colour = (*edx >> 8) & 0xFF;
	uint8 secondary_colour = *ebp & 0xFF;
	uint8 tertiary_colour = (*ebp >> 8) & 0xFF;
	uint8 edge = *edx & 0xFF;

	// *not used*
	RCT2_GLOBAL(0x00141F726, uint8) = secondary_colour;
	// *not used*
	RCT2_GLOBAL(0x00141F727, uint8) = tertiary_colour;
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	// Banner index *not used*
	RCT2_GLOBAL(0x00141F728, uint8) = 0xFF;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, sint16) = position.x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, sint16) = position.y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, sint16) = position.z;

	if (position.z == 0){
		RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, sint16) = map_element_height(position.x, position.y) & 0xFFFF;
	}

	if (RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0 && !gConfigCheat.build_in_pause_mode){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) &&
		!(flags & (1 << 7)) && !gCheatsSandboxMode){

		if (position.z == 0){
			if (!map_is_location_in_park(position.x, position.y)){
				*ebx = MONEY32_UNDEFINED;
				return;
			}
		}
		else if (!map_is_location_owned(position.x, position.y, position.z)){
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}

	uint8 bp = 0;
	if (position.z == 0){
		rct_map_element* map_element = map_get_surface_element_at(position.x / 32, position.y / 32);
		if (map_element == NULL){
			*ebx = MONEY32_UNDEFINED;
			return;
		}
		position.z = map_element->base_height * 8;

		uint8 slope = map_element->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
		bp = RCT2_ADDRESS(0x009A3FEC, uint8)[slope + (edge & 3) * 32];
		if (bp & (1 << 0)){
			position.z += 16;
			bp &= ~(1 << 0);
		}
	}

	RCT2_GLOBAL(0x00141F721, uint16) = position.z / 8;
	RCT2_GLOBAL(0x00141F720, uint8) = primary_colour;
	RCT2_GLOBAL(0x00141F723, uint8) = bp;

	rct_map_element* map_element = map_get_surface_element_at(position.x / 32, position.y / 32);
	if (map_element == NULL){
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if (map_element->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK){
		uint16 water_height = map_element->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK;
		water_height *= 16;

		if (position.z < water_height){
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CANT_BUILD_THIS_UNDERWATER;
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}

	if (position.z / 8 < map_element->base_height){
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CAN_ONLY_BUILD_THIS_ABOVE_GROUND;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if (!(bp & 0xC0)){
		uint8 new_edge = (edge + 2) & 3;
		bp += 2;
		if (map_element->properties.surface.slope & (1 << new_edge)){
			if (position.z / 8 < bp){
				RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CAN_ONLY_BUILD_THIS_ABOVE_GROUND;
				*ebx = MONEY32_UNDEFINED;
				return;
			}

			if (map_element->properties.surface.slope & (1 << 4)){
				new_edge = (new_edge - 1) & 3;

				if (map_element->properties.surface.slope & (1 << new_edge)){
					new_edge = (new_edge + 2) & 3;
					if (map_element->properties.surface.slope & (1 << new_edge)){
						bp += 2;
						if (position.z / 8 < bp){
							RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CAN_ONLY_BUILD_THIS_ABOVE_GROUND;
							*ebx = MONEY32_UNDEFINED;
							return;
						}
						bp -= 2;
					}
				}
			}
		}

		new_edge = (edge + 3) & 3;
		if (map_element->properties.surface.slope & (1 << new_edge)){
			if (position.z / 8 < bp){
				RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CAN_ONLY_BUILD_THIS_ABOVE_GROUND;
				*ebx = MONEY32_UNDEFINED;
				return;
			}

			if (map_element->properties.surface.slope & (1 << 4)){
				new_edge = (new_edge - 1) & 3;

				if (map_element->properties.surface.slope & (1 << new_edge)){
					new_edge = (new_edge + 2) & 3;
					if (map_element->properties.surface.slope & (1 << new_edge)){
						bp += 2;
						if (position.z / 8 < bp){
							RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CAN_ONLY_BUILD_THIS_ABOVE_GROUND;
							*ebx = MONEY32_UNDEFINED;
							return;
						}
					}
				}
			}
		}
	}
	int banner_index = 0xFF;
	rct_scenery_entry* fence = g_wallSceneryEntries[fence_type];
	if (fence->wall.var_0D != 0xFF){
		banner_index = create_new_banner(fence->wall.var_0D);

		RCT2_GLOBAL(0x00141F728, uint8) = banner_index;
		if (banner_index == 0xFF){
			*ebx = MONEY32_UNDEFINED;
			return;
		}

		rct_banner* banner = &gBanners[banner_index];
		if (flags & GAME_COMMAND_FLAG_APPLY){
			banner->flags |= (1 << 3);
			banner->type = 0;
			banner->x = position.x / 32;
			banner->y = position.y / 32;

			int rideIndex = banner_get_closest_ride_index(position.x, position.y, position.z);
			if (rideIndex != -1) {
				banner->colour = rideIndex & 0xFF;
				banner->flags |= BANNER_FLAG_2;
			}
		}
	}

	bp = RCT2_GLOBAL(0x00141F723, uint8);

	RCT2_GLOBAL(0x00141F722, uint8) = position.z / 8;
	if (bp & 0xC0){
		if (fence->wall.flags & WALL_SCENERY_FLAG3){
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = 3133;
			*ebx = MONEY32_UNDEFINED;
			return;
		}
		RCT2_GLOBAL(0x00141F722, uint8) += 2;
	}
	RCT2_GLOBAL(0x00141F722, uint8) += fence->wall.height;

	if (!(flags & (1 << 7)) && !gCheatsDisableClearanceChecks){
		if (!map_place_fence_check_obstruction(fence, position.x, position.y, RCT2_GLOBAL(0x00141F721, uint8), RCT2_GLOBAL(0x00141F722, uint8), edge)) {
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}

	if (!sub_68B044()){
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	if (flags & GAME_COMMAND_FLAG_APPLY){
		map_element = map_element_insert(position.x / 32, position.y / 32, position.z / 8, 0);

		map_animation_create(MAP_ANIMATION_TYPE_WALL, position.x, position.y, position.z / 8);

		map_element->clearance_height = RCT2_GLOBAL(0x00141F722, uint8);

		map_element->type = bp | edge | MAP_ELEMENT_TYPE_FENCE;

		map_element->properties.fence.item[1] = primary_colour;
		map_element->properties.fence.item[1] |= (secondary_colour & 7) << 5;
		map_element->flags |= (secondary_colour & 0x18) << 2;

		if (RCT2_GLOBAL(0x00141F725, uint8) & 1){
			map_element->properties.fence.item[2] |= (1 << 2);
		}

		map_element->properties.fence.type = fence_type;
		if (banner_index != 0xFF){
			map_element->properties.fence.item[0] = banner_index;
		}

		if (fence->wall.flags & WALL_SCENERY_HAS_TERNARY_COLOUR){
			map_element->properties.fence.item[0] = tertiary_colour;
		}

		if (flags & (1 << 6)){
			map_element->flags |= MAP_ELEMENT_FLAG_GHOST;
		}

		RCT2_GLOBAL(RCT2_ADDRESS_SCENERY_MAP_ELEMENT, rct_map_element*) = map_element;
		map_invalidate_tile_zoom1(position.x, position.y, map_element->base_height * 8, map_element->base_height * 8 + 72);
	}

	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY){
		*ebx = 0;
	}
	else{
		*ebx = fence->wall.price;
	}
}

money32 map_place_fence(
	int type, int x, int y, int z, int edge, int primaryColour, int secondaryColour, int tertiaryColour, int flags
) {
	int eax, ebx, ecx, edx, esi, edi, ebp;

	eax = x;
	ebx = flags | (type << 8);
	ecx = y;
	edx = edge | (primaryColour << 8);
	edi = z;
	ebp = secondaryColour | (tertiaryColour << 8);
	game_command_place_fence(&eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
	return ebx;
}

/**
 *
 *  rct2: 0x006B893C
 */
void game_command_place_large_scenery(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp)
{
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LANDSCAPING * 4;
	int x = (sint16)*eax;
	int y = (sint16)*ecx;
	int z = (sint16)*ebp;
	uint8 color1 = *edx;
	uint8 color2 = *edx >> 8;
	uint8 flags = *ebx;
	uint8 rotation = *ebx >> 8;
	uint8 entry_index = *edi;
	int base_height = map_element_height(x, y);
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, sint16) = x + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, sint16) = y + 16;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, sint16) = base_height;
	RCT2_GLOBAL(0x00F64F14, uint8) = 0;
	uint8 banner_id = 0xFF;
	RCT2_GLOBAL(0x00F4389A, uint32) = 0;
	if(RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) == 0 || gConfigCheat.build_in_pause_mode){
		if(sub_68B044()){
			rct_scenery_entry* scenery_entry = RCT2_ADDRESS(RCT2_ADDRESS_LARGE_SCENERY_ENTRIES, rct_scenery_entry*)[entry_index];
			if(scenery_entry->large_scenery.var_11 != 0xFF){
				banner_id = create_new_banner(flags);
				if (banner_id == MAX_BANNERS) {
					*ebx = MONEY32_UNDEFINED;
					return;
				}
				if (flags & GAME_COMMAND_FLAG_APPLY) {
					rct_banner* banner = &gBanners[banner_id];
					banner->flags |= BANNER_FLAG_1;
					banner->type = 0;
					banner->x = x / 32;
					banner->y = y / 32;

					int rideIndex = banner_get_closest_ride_index(x, y, z);
					if (rideIndex != -1) {
						banner->colour = rideIndex;
						banner->flags |= BANNER_FLAG_2;
					}
				}
			}
			rct_large_scenery_tile* tile = scenery_entry->large_scenery.tiles;
			sint16 F43884 = 0xFFFF;
			do{
				if(tile->x_offset == (sint16)0xFFFF){
					break;
				}
				int x2 = tile->x_offset;
				int y2 = tile->y_offset;
				switch(rotation){
					case 0:{
					}break;
					case 1:{
						int temp = x2;
						x2 = y2;
						y2 = -temp;
					}break;
					case 2:{
						x2 = -x2;
						y2 = -y2;
					}break;
					case 3:{
						int temp = y2;
						y2 = x2;
						x2 = -temp;
					}break;
				}
				x2 += x;
				y2 += y;
				if(x2 >= 0x1FFF || y2 >= 0x1FFF || x2 < 0 || y2 < 0){
					tile++;
					continue;
				}
				rct_map_element* map_element = map_get_surface_element_at(x2 / 32, y2 / 32);
				if(map_element != NULL){
					int height = map_element->base_height * 8;
					if(map_element->properties.scenerymultiple.type & 0xF){
						height += 16;
						if(map_element->properties.scenerymultiple.type & 0x10){
							height += 16;
						}
					}
					if(height > F43884){
						F43884 = height;
					}
				}
				tile++;
			}while(1);
			if(z != 0){
				F43884 = z;
			}
			RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, sint16) = F43884;
			tile = scenery_entry->large_scenery.tiles;
			uint8 tile_num = 0;
			do{
				if(tile->x_offset == (sint16)0xFFFF){
					break;
				}
				int x2 = tile->x_offset;
				int y2 = tile->y_offset;
				switch(rotation){
					case 0:{
					}break;
					case 1:{
						int temp = x2;
						x2 = y2;
						y2 = -temp;
					}break;
					case 2:{
						x2 = -x2;
						y2 = -y2;
					}break;
					case 3:{
						int temp = y2;
						y2 = x2;
						x2 = -temp;
					}break;
				}
				int zLow = (tile->z_offset + F43884) / 8;
				int zHigh = (tile->var_6 / 8) + zLow;
				int bx = tile->var_7 >> 12;
				bx <<= rotation;
				uint8 bl = bx;
				uint8 bh = bl >> 4;
				bl &= 0xF;
				bl |= bh;
				uint8 F43887 = bl;
				x2 += x;
				y2 += y;
				RCT2_GLOBAL(0x00F43892, sint16) = x2;
				RCT2_GLOBAL(0x00F43894, sint16) = y2;
				RCT2_GLOBAL(0x00F43896, uint32) = (uint32)(ebx - 3); // this is how ebx flags var is passed to 0x006B8D88
				if(gCheatsDisableClearanceChecks || map_can_construct_with_clear_at(x2, y2, zLow, zHigh, (void*)0x006B8D88, bl)){
					if(!(RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) & ELEMENT_IS_UNDERWATER) && !(RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) & 2)){
						int b = RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) & 0x3;
						if (!gCheatsDisableClearanceChecks) {
							if (RCT2_GLOBAL(0x00F64F14, uint8) && !(RCT2_GLOBAL(0x00F64F14, uint8) & b)){
								RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CANT_BUILD_PARTLY_ABOVE_AND_PARTLY_BELOW_GROUND;
								*ebx = MONEY32_UNDEFINED;
								return;
							}
						}
						RCT2_GLOBAL(0x00F64F14, uint8) = b;
						if(x2 >= RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) || y2 >= RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16)){
							RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_OFF_EDGE_OF_MAP;
							*ebx = MONEY32_UNDEFINED;
							return;
						}
						if((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) || map_is_location_owned(x2, y2, zLow * 8) || gCheatsSandboxMode){
							if(*ebx & GAME_COMMAND_FLAG_APPLY){
								if(!(*ebx & 0x40)){
									footpath_remove_litter(x2, y2, zLow * 8);
									if (!gCheatsDisableClearanceChecks) {
										map_remove_walls_at(x2, y2, zLow * 8, zHigh * 8);
									}
								}
								rct_map_element *new_map_element = map_element_insert(x2 / 32, y2 / 32, zLow, F43887);
								map_animation_create(0xB, x2, y2, zLow);

								new_map_element->clearance_height = zHigh;
								new_map_element->type = MAP_ELEMENT_TYPE_SCENERY_MULTIPLE | rotation;
								int bx = tile_num;
								bx <<= 10;
								bx |= entry_index;
								new_map_element->properties.scenerymultiple.type = bx;
								new_map_element->properties.scenerymultiple.colour[0] = color1;
								new_map_element->properties.scenerymultiple.colour[1] = color2;
								if(banner_id != 0xFF){
									uint8 bh = banner_id;
									bh &= 0xC0;
									new_map_element->type |= bh;
									bh = banner_id;
									bh &= 0x38;
									bh <<= 2;
									new_map_element->properties.scenerymultiple.colour[0] |= bh;
									uint8 bl = banner_id;
									bl &= 7;
									bl <<= 5;
									new_map_element->properties.scenerymultiple.colour[1] |= bl;
								}

								if(*ebx & 0x40){
									new_map_element->flags |= MAP_ELEMENT_FLAG_GHOST;
								}
								if(tile_num == 0){
									RCT2_GLOBAL(RCT2_ADDRESS_SCENERY_MAP_ELEMENT, rct_map_element*) = new_map_element;
								}
								map_invalidate_tile_full(x2, y2);
							}
						}else{
							*ebx = MONEY32_UNDEFINED;
							return;
						}
					}else{
						*ebx = MONEY32_UNDEFINED;
						return;
					}
				}else{
					*ebx = MONEY32_UNDEFINED;
					return;
				}
				tile++;
				tile_num++;
			}while(1);
			*ebx = (scenery_entry->large_scenery.price * 10) + RCT2_GLOBAL(0x00F4389A, uint32);
			if(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY){
				*ebx = 0;
			}
			return;
		}
	}else{
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = STR_CONSTRUCTION_NOT_POSSIBLE_WHILE_GAME_IS_PAUSED;
	}
	*ebx = MONEY32_UNDEFINED;
}

int map_get_station(rct_map_element *mapElement)
{
	return (mapElement->properties.track.sequence & 0x70) >> 4;
}

/**
 *
 *  rct2: 0x0068B280
 */
void map_element_remove(rct_map_element *mapElement)
{
	if (!map_element_is_last_for_tile(mapElement)){
		do{
			*mapElement = *(mapElement + 1);
		} while (!map_element_is_last_for_tile(++mapElement));
	}
	(mapElement - 1)->flags |= MAP_ELEMENT_FLAG_LAST_TILE;
	mapElement->base_height = 0xFF;

	if ((mapElement + 1) == RCT2_GLOBAL(0x00140E9A4, rct_map_element*)){
		RCT2_GLOBAL(0x00140E9A4, rct_map_element*)--;
	}
}

/**
 *
 *  rct2: 0x00675A8E
 */
void map_remove_all_rides()
{
	map_element_iterator it;

	map_element_iterator_begin(&it);
	do {
		switch (map_element_get_type(it.element)) {
		case MAP_ELEMENT_TYPE_PATH:
			if (it.element->type & 1) {
				it.element->properties.path.type &= ~8;
				it.element->properties.path.addition_status = 255;
			}
			break;
		case MAP_ELEMENT_TYPE_ENTRANCE:
			if (it.element->properties.entrance.type == ENTRANCE_TYPE_PARK_ENTRANCE)
				break;

			// fall-through
		case MAP_ELEMENT_TYPE_TRACK:
			sub_6A7594();
			footpath_remove_edges_at(it.x * 32, it.y * 32, it.element);
			map_element_remove(it.element);
			map_element_iterator_restart_for_tile(&it);
			break;
		}
	} while (map_element_iterator_next(&it));
}

/**
 *
 *  rct2: 0x0068AB1B
 */
void map_invalidate_map_selection_tiles()
{
	rct_xy16 *position;

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_FLAGS, uint16) & (1 << 1)))
		return;

	for (position = gMapSelectionTiles; position->x != -1; position++)
		map_invalidate_tile_full(position->x, position->y);
}

void map_get_bounding_box(int ax, int ay, int bx, int by, int *left, int *top, int *right, int *bottom)
{
	int x, y;
	x = ax;
	y = ay;
	translate_3d_to_2d(get_current_rotation(), &x, &y);
	*left = x;
	*right = x;
	*top = y;
	*bottom = y;
	x = bx;
	y = ay;
	translate_3d_to_2d(get_current_rotation(), &x, &y);
	if (x < *left) *left = x;
	if (x > *right) *right = x;
	if (y > *bottom) *bottom = y;
	if (y < *top) *top = y;
	x = bx;
	y = by;
	translate_3d_to_2d(get_current_rotation(), &x, &y);
	if (x < *left) *left = x;
	if (x > *right) *right = x;
	if (y > *bottom) *bottom = y;
	if (y < *top) *top = y;
	x = ax;
	y = by;
	translate_3d_to_2d(get_current_rotation(), &x, &y);
	if (x < *left) *left = x;
	if (x > *right) *right = x;
	if (y > *bottom) *bottom = y;
	if (y < *top) *top = y;
}

/**
 *
 *  rct2: 0x0068AAE1
 */
void map_invalidate_selection_rect()
{
	int x0, y0, x1, y1, left, right, top, bottom;
	rct_viewport *viewport;

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_FLAGS, uint16) & (1 << 0)))
		return;

	x0 = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_A_X, sint16) + 16;
	y0 = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_A_Y, sint16) + 16;
	x1 = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_B_X, sint16) + 16;
	y1 = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_B_Y, sint16) + 16;
	map_get_bounding_box(x0, y0, x1, y1, &left, &top, &right, &bottom);
	left -= 32;
	right += 32;
	bottom += 32;
	top -= 32 + 2080;

	viewport = RCT2_GLOBAL(RCT2_ADDRESS_ACTIVE_VIEWPORT_PTR_ARRAY, rct_viewport*);
	while (viewport->width != 0) {
		viewport_invalidate(viewport, left, top, right, bottom);
		viewport++;
	}
}

/**
 *
 *  rct2: 0x0068B111
 */
void map_reorganise_elements()
{
	platform_set_cursor(CURSOR_ZZZ);

	rct_map_element* new_map_elements = rct2_malloc(0x30000 * sizeof(rct_map_element));
	rct_map_element* new_elements_pointer = new_map_elements;

	if (new_map_elements == NULL || new_map_elements == (rct_map_element*)-1){
		error_string_quit(4370, 0xFFFF);
		return;
	}

	uint32 num_elements;

	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 256; x++) {
			rct_map_element *startElement = map_get_first_element_at(x, y);
			rct_map_element *endElement = startElement;
			while (!map_element_is_last_for_tile(endElement++));

			num_elements = endElement - startElement;
			memcpy(new_elements_pointer, startElement, num_elements * sizeof(rct_map_element));
			new_elements_pointer += num_elements;
		}
	}

	num_elements = (new_elements_pointer - new_map_elements);
	memcpy(RCT2_ADDRESS(RCT2_ADDRESS_MAP_ELEMENTS, rct_map_element), new_map_elements, num_elements * sizeof(rct_map_element));
	memset(RCT2_ADDRESS(RCT2_ADDRESS_MAP_ELEMENTS, rct_map_element) + num_elements, 0, (0x30000 - num_elements) * sizeof(rct_map_element));

	rct2_free(new_map_elements);

	map_update_tile_pointers();
}

/**
 *
 *  rct2: 0x0068B044
 */
int sub_68B044()
{
	if (RCT2_GLOBAL(0x00140E9A4, rct_map_element*) <= RCT2_ADDRESS(RCT2_ADDRESS_MAP_ELEMENTS_END, rct_map_element))
		return 1;

	for (int i = 1000; i != 0; --i)
		sub_68B089();

	if (RCT2_GLOBAL(0x00140E9A4, rct_map_element*) <= RCT2_ADDRESS(RCT2_ADDRESS_MAP_ELEMENTS_END, rct_map_element))
		return 1;

	map_reorganise_elements();

	if (RCT2_GLOBAL(0x00140E9A4, rct_map_element*) <= RCT2_ADDRESS(RCT2_ADDRESS_MAP_ELEMENTS_END, rct_map_element))
		return 1;
	else{
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = 894;
		return 0;
	}
}

/**
 *
 *  rct2: 0x0068B1F6
 */
rct_map_element *map_element_insert(int x, int y, int z, int flags)
{
	rct_map_element *originalMapElement, *newMapElement, *insertedElement;

	sub_68B044();

	newMapElement = RCT2_GLOBAL(0x00140E9A4, rct_map_element*);
	originalMapElement = TILE_MAP_ELEMENT_POINTER(y * 256 + x);

	// Set tile index pointer to point to new element block
	TILE_MAP_ELEMENT_POINTER(y * 256 + x) = newMapElement;

	// Copy all elements that are below the insert height
	while (z >= originalMapElement->base_height) {
		// Copy over map element
		*newMapElement = *originalMapElement;
		originalMapElement->base_height = 255;
		originalMapElement++;
		newMapElement++;

		if ((newMapElement - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE) {
			// No more elements above the insert element
			(newMapElement - 1)->flags &= ~MAP_ELEMENT_FLAG_LAST_TILE;
			flags |= MAP_ELEMENT_FLAG_LAST_TILE;
			break;
		}
	}

	// Insert new map element
	insertedElement = newMapElement;
	newMapElement->base_height = z;
	newMapElement->flags = flags;
	newMapElement->clearance_height = z;
	*((uint32*)&newMapElement->properties) = 0;
	newMapElement++;

	// Insert rest of map elements above insert height
	if (!(flags & MAP_ELEMENT_FLAG_LAST_TILE)) {
		do {
			// Copy over map element
			*newMapElement = *originalMapElement;
			originalMapElement->base_height = 255;
			originalMapElement++;
			newMapElement++;
		} while (!((newMapElement - 1)->flags & MAP_ELEMENT_FLAG_LAST_TILE));
	}

	RCT2_GLOBAL(0x00140E9A4, rct_map_element*) = newMapElement;
	return insertedElement;
}

/**
 *
 *  rct2: 0x0068BB18
 */
static void map_obstruction_set_error_text(rct_map_element *mapElement)
{
	rct_string_id errorStringId;
	rct_ride *ride;
	rct_scenery_entry *sceneryEntry;

	errorStringId = STR_OBJECT_IN_THE_WAY;
	switch (map_element_get_type(mapElement)) {
	case MAP_ELEMENT_TYPE_SURFACE:
		errorStringId = STR_RAISE_OR_LOWER_LAND_FIRST;
		break;
	case MAP_ELEMENT_TYPE_PATH:
		errorStringId = STR_FOOTPATH_IN_THE_WAY;
		break;
	case MAP_ELEMENT_TYPE_TRACK:
		ride = GET_RIDE(mapElement->properties.track.ride_index);
		errorStringId = STR_X_IN_THE_WAY;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS + 0, uint16) = ride->name;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS + 2, uint32) = ride->name_arguments;
		break;
	case MAP_ELEMENT_TYPE_SCENERY:
		sceneryEntry = g_smallSceneryEntries[mapElement->properties.scenery.type];
		errorStringId = STR_X_IN_THE_WAY;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS + 0, uint16) = sceneryEntry->name;
		break;
	case MAP_ELEMENT_TYPE_ENTRANCE:
		switch (mapElement->properties.entrance.type) {
		case ENTRANCE_TYPE_RIDE_ENTRANCE:
			errorStringId = STR_RIDE_ENTRANCE_IN_THE_WAY;
			break;
		case ENTRANCE_TYPE_RIDE_EXIT:
			errorStringId = STR_RIDE_EXIT_IN_THE_WAY;
			break;
		case ENTRANCE_TYPE_PARK_ENTRANCE:
			errorStringId = STR_PARK_ENTRANCE_IN_THE_WAY;
			break;
		}
		break;
	case MAP_ELEMENT_TYPE_FENCE:
		sceneryEntry = g_wallSceneryEntries[mapElement->properties.scenery.type];
		errorStringId = STR_X_IN_THE_WAY;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS + 0, uint16) = sceneryEntry->name;
		break;
	case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
		sceneryEntry = g_largeSceneryEntries[mapElement->properties.scenery.type];
		errorStringId = STR_X_IN_THE_WAY;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS + 0, uint16) = sceneryEntry->name;
		break;
	}

	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = errorStringId;
}

/**
 *
 *  rct2: 0x0068B932
 */
int map_can_construct_with_clear_at(int x, int y, int zLow, int zHigh, void *clearFunc, uint8 bl)
{
	// return (RCT2 CALLPROC X(0x0068B932, x, bl, y, (zHigh << 8) | zLow, 0, 0, (int)clearFunc) & 0x100) == 0;
	RCT2_GLOBAL(0x00F1AD40, void*) = clearFunc;
	RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) = 1;
	if (x >= RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16) || y >= RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16) || x < 32 || y < 32) {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_OFF_EDGE_OF_MAP;
		return false;
	}
	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);
	do {
		if (map_element_get_type(map_element) != MAP_ELEMENT_TYPE_SURFACE) {
			if (zLow < map_element->clearance_height && zHigh > map_element->base_height && !(map_element->flags & MAP_ELEMENT_FLAG_GHOST)) {
				if (map_element->flags & (bl & 0x0F)) {
					goto loc_68BABC;
				}
			}
			continue;
		}
		int water_height = ((map_element->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK) * 2);
		if (water_height && water_height > zLow && map_element->base_height < zHigh) {
			RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) |= 4;
			if (water_height < zHigh) {
				goto loc_68BAE6;
			}
		}
		loc_68B9B7:
		if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_FORBID_HIGH_CONSTRUCTION) {
			int al = zHigh - map_element->base_height;
			if (al >= 0) {
				if (al > 18) {
					RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_LOCAL_AUTHORITY_WONT_ALLOW_CONSTRUCTION_ABOVE_TREE_HEIGHT;
					return false;
				}
			}
		}
		if ((bl & 0xF0) != 0xF0) {
			if (map_element->base_height >= zHigh) {
				// loc_68BA81
				RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) |= 2;
				RCT2_GLOBAL(RCT2_ADDRESS_ELEMENT_LOCATION_COMPARED_TO_GROUND_AND_WATER, uint8) &= 0xFE;
			} else {
				int al = map_element->base_height;
				int ah = al;
				int cl = al;
				int ch = al;
				uint8 slope = map_element->properties.surface.slope & MAP_ELEMENT_SLOPE_MASK;
				if (slope & 1) {
					al += 2;
					if (slope == 0x1B)
						al += 2;
				}
				if (slope & 2) {
					ah += 2;
					if (slope == 0x17)
						ah += 2;
				}
				if (slope & 4) {
					cl += 2;
					if (slope == 0x1E)
						cl += 2;
				}
				if (slope & 8) {
					ch += 2;
					if (slope == 0x1D)
						ch += 2;
				}
				int bh = zLow + 4;
				if ((!(bl & 1) || ((bl & 0x10 || zLow >= al) && bh >= al)) &&
					(!(bl & 2) || ((bl & 0x20 || zLow >= ah) && bh >= ah)) &&
					(!(bl & 4) || ((bl & 0x40 || zLow >= cl) && bh >= cl)) &&
					(!(bl & 8) || ((bl & 0x80 || zLow >= ch) && bh >= ch))) {
					continue;
				}
				loc_68BABC:
				if (RCT2_GLOBAL(0x00F1AD40, void*) != (void*)0xFFFFFFFF) {
					int zero = 0;
					if (!(RCT2_CALLFUNC_X((int)RCT2_GLOBAL(0x00F1AD40, void*), &zero, &zero, &zero, &zero, (int*)&map_element, &zero, &zero) & 0x100)) {
						continue;
					}
				}
				if (map_element != (rct_map_element*)0xFFFFFFF) {
					map_obstruction_set_error_text(map_element);
				}
				return false;
				loc_68BAE6:
				if (RCT2_GLOBAL(0x00F1AD40, void*) != (void*)0xFFFFFFFF) {
					int zero = 0;
					if (!(RCT2_CALLFUNC_X((int)RCT2_GLOBAL(0x00F1AD40, void*), &zero, &zero, &zero, &zero, (int*)&map_element, &zero, &zero) & 0x100)) {
						goto loc_68B9B7;
					}
				}
				if (map_element != (rct_map_element*)0xFFFFFFF) {
					RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, rct_string_id) = STR_CANNOT_BUILD_PARTLY_ABOVE_AND_PARTLY_BELOW_WATER;
				}
				return false;
			}
		}
	} while (!map_element_is_last_for_tile(map_element++));
	return true;
}

/**
 *
 *  rct2: 0x0068B93A
 */
int map_can_construct_at(int x, int y, int zLow, int zHigh, uint8 bl)
{
	return map_can_construct_with_clear_at(x, y, zLow, zHigh, (void*)0xFFFFFFFF, bl);
}

/**
 *
 *  rct2: 0x006E5935
 */
void map_remove_intersecting_walls(int x, int y, int z0, int z1, int direction)
{
	rct_map_element *mapElement;

	mapElement = map_get_first_element_at(x >> 5, y >> 5);
	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_FENCE)
			continue;

		if (mapElement->clearance_height <= z0 || mapElement->base_height >= z1)
			continue;

		if (direction != (mapElement->type & 3))
			continue;

		map_element_remove_banner_entry(mapElement);
		map_invalidate_tile_zoom1(x, y, mapElement->base_height * 8, mapElement->base_height * 8 + 72);
		map_element_remove(mapElement);
		mapElement--;
	} while (!map_element_is_last_for_tile(mapElement++));
}

/**
 * Updates grass length, scenery age and jumping fountains.
 *
 *  rct2: 0x006646E1
 */
void map_update_tiles()
{
	int ignoreScreenFlags = SCREEN_FLAGS_SCENARIO_EDITOR | SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & ignoreScreenFlags)
		return;

	// Update 43 more tiles
	for (int j = 0; j < 43; j++) {
		int x = 0;
		int y = 0;

		uint16 interleaved_xy = RCT2_GLOBAL(RCT2_ADDRESS_GRASS_SCENERY_TILEPOS, sint16);
		for (int i = 0; i < 8; i++) {
			x = (x << 1) | (interleaved_xy & 1);
			interleaved_xy >>= 1;
			y = (y << 1) | (interleaved_xy & 1);
			interleaved_xy >>= 1;
		}

		rct_map_element *mapElement = map_get_surface_element_at(x, y);
		if (mapElement != NULL) {
			map_update_grass_length(x * 32, y * 32, mapElement);
			scenery_update_tile(x * 32, y * 32);
		}

		RCT2_GLOBAL(RCT2_ADDRESS_GRASS_SCENERY_TILEPOS, sint16)++;
		RCT2_GLOBAL(RCT2_ADDRESS_GRASS_SCENERY_TILEPOS, sint16) &= 0xFFFF;
	}
}

/**
 *
 *  rct2: 0x006647A1
 */
static void map_update_grass_length(int x, int y, rct_map_element *mapElement)
{
	// Check if tile is grass
	if ((mapElement->properties.surface.terrain & 0xE0) && !(mapElement->type & 3))
		return;

	int grassLength = mapElement->properties.surface.grass_length & 7;

	// Check if grass is underwater or outside park
	int waterHeight = (mapElement->properties.surface.terrain & 0x1F) * 2;
	if (waterHeight > mapElement->base_height || !map_is_location_in_park(x, y)) {
		if (grassLength != GRASS_LENGTH_CLEAR_0)
			map_set_grass_length(x, y, mapElement, GRASS_LENGTH_CLEAR_0);

		return;
	}

	// Grass can't grow any further
	if (grassLength == GRASS_LENGTH_CLUMPS_2)
		return;

	int z0 = mapElement->base_height;
	int z1 = mapElement->base_height + 2;
	if (mapElement->properties.surface.slope & 0x10)
		z1 += 2;

	// Check objects above grass
	rct_map_element *mapElementAbove = mapElement;
	for (;;) {
		if (mapElementAbove->flags & MAP_ELEMENT_FLAG_LAST_TILE) {
			// Grow grass
			if (mapElement->properties.surface.grass_length < 0xF0) {
				mapElement->properties.surface.grass_length += 0x10;
			} else {
				mapElement->properties.surface.grass_length += 0x10;
				mapElement->properties.surface.grass_length ^= 8;
				if (mapElement->properties.surface.grass_length & 8) {
					// Random growth rate
					mapElement->properties.surface.grass_length |= scenario_rand() & 0x70;
				} else {
					// Increase length
					map_set_grass_length(x, y, mapElement, grassLength + 1);
				}
			}
		} else {
			mapElementAbove++;
			if (map_element_get_type(mapElementAbove) == MAP_ELEMENT_TYPE_FENCE)
				continue;
			if (z0 >= mapElementAbove->clearance_height)
				continue;
			if (z1 < mapElementAbove->base_height)
				continue;
		}
		break;
	}
}

static void map_set_grass_length(int x, int y, rct_map_element *mapElement, int length)
{
	int z0, z1;

	mapElement->properties.surface.grass_length = length;
	z0 = mapElement->base_height * 8;
	z1 = z0 + 16;
	map_invalidate_tile(x, y, z0, z1);
}

void sub_6A7594()
{
	RCT2_GLOBAL(0x00F3EFF4, uint32) = 0x00F3EFF8;
}

int map_element_get_banner_index(rct_map_element *mapElement)
{
	rct_scenery_entry* sceneryEntry;

	switch (map_element_get_type(mapElement)) {
	case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
		sceneryEntry = g_largeSceneryEntries[mapElement->properties.scenerymultiple.type & 0x3FF];
		if (sceneryEntry->large_scenery.var_11 == 0xFF)
			return -1;

		return
			(mapElement->type & MAP_ELEMENT_QUADRANT_MASK) |
			((mapElement->properties.scenerymultiple.colour[0] & 0xE0) >> 2) |
			((mapElement->properties.scenerymultiple.colour[1] & 0xE0) >> 5);
	case MAP_ELEMENT_TYPE_FENCE:
		sceneryEntry = g_wallSceneryEntries[mapElement->properties.fence.type];
		if (sceneryEntry->wall.var_0D == 0xFF)
			return -1;

		return mapElement->properties.fence.item[0];
	case MAP_ELEMENT_TYPE_BANNER:
		return mapElement->properties.banner.index;
	default:
		return -1;
	}
}

void map_element_remove_banner_entry(rct_map_element *mapElement)
{
	int bannerIndex = map_element_get_banner_index(mapElement);
	if (bannerIndex == -1)
		return;

	rct_banner* banner = &gBanners[bannerIndex];
	if (banner->type != BANNER_NULL) {
		window_close_by_number(WC_BANNER, bannerIndex);
		banner->type = BANNER_NULL;
		user_string_free(banner->string_idx);
	}
}

/**
 * Removes elements that are out of the map size range and crops the park perimeter.
 *  rct2: 0x0068ADBC
 */
void map_remove_out_of_range_elements()
{
	int mapMaxXY = RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16);

	for (int y = 0; y < (256 * 32); y += 32) {
		for (int x = 0; x < (256 * 32); x += 32) {
			if (x == 0 || y == 0 || x >= mapMaxXY || y >= mapMaxXY) {
				sub_68AE2A(x, y);
			} else if (x >= mapMaxXY - 32 || y >= mapMaxXY - 32) {
				RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) += 32;
				map_buy_land_rights(x, y, x, y, 6, GAME_COMMAND_FLAG_APPLY);
				RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) -= 32;
			}
		}
	}
}

/**
 * Copies the terrain and slope from the edge of the map to the new tiles. Used when increasing the size of the map.
 *  rct2: 0x0068AC15
 */
void map_extend_boundary_surface()
{
	rct_map_element *existingMapElement, *newMapElement;
	int x, y, z, slope;

	y = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE, uint16) - 2;
	for (x = 0; x < 256; x++) {
		existingMapElement = map_get_surface_element_at(x, y - 1);
		newMapElement = map_get_surface_element_at(x, y);

		newMapElement->type = (newMapElement->type & 0x7C) | (existingMapElement->type & 0x83);
		newMapElement->properties.surface.slope = existingMapElement->properties.surface.slope & 0xE0;
		newMapElement->properties.surface.terrain = existingMapElement->properties.surface.terrain;
		newMapElement->properties.surface.grass_length = existingMapElement->properties.surface.grass_length;
		newMapElement->properties.surface.ownership = 0;

		z = existingMapElement->base_height;
		slope = existingMapElement->properties.surface.slope & 9;
		if (slope == 9) {
			z += 2;
			slope = 0;
			if (existingMapElement->properties.surface.slope & 0x10) {
				slope = 1;
				if (existingMapElement->properties.surface.slope & 0x04) {
					slope = 8;
					if (existingMapElement->properties.surface.slope & 0x02) {
						slope = 0;
					}
				}
			}
		}
		if (slope & 1) slope |= 2;
		if (slope & 8) slope |= 4;

		newMapElement->properties.surface.slope |= slope;
		newMapElement->base_height = z;
		newMapElement->clearance_height = z;
	}

	x = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE, uint16) - 2;
	for (y = 0; y < 256; y++) {
		existingMapElement = map_get_surface_element_at(x - 1, y);
		newMapElement = map_get_surface_element_at(x, y);

		newMapElement->type = (newMapElement->type & 0x7C) | (existingMapElement->type & 0x83);
		newMapElement->properties.surface.slope = existingMapElement->properties.surface.slope & 0xE0;
		newMapElement->properties.surface.terrain = existingMapElement->properties.surface.terrain;
		newMapElement->properties.surface.grass_length = existingMapElement->properties.surface.grass_length;
		newMapElement->properties.surface.ownership = 0;

		z = existingMapElement->base_height;
		slope = existingMapElement->properties.surface.slope & 3;
		if (slope == 3) {
			z += 2;
			slope = 0;
			if (existingMapElement->properties.surface.slope & 0x10) {
				slope = 1;
				if (existingMapElement->properties.surface.slope & 0x04) {
					slope = 2;
					if (existingMapElement->properties.surface.slope & 0x08) {
						slope = 0;
					}
				}
			}
		}
		if (slope & 1) slope |= 8;
		if (slope & 2) slope |= 4;

		newMapElement->properties.surface.slope |= slope;
		newMapElement->base_height = z;
		newMapElement->clearance_height = z;
	}

}

static void sub_68AE2A(int x, int y)
{
	for (;;) {
		rct2_peep_spawn *peepSpawns = RCT2_ADDRESS(RCT2_ADDRESS_PEEP_SPAWNS, rct2_peep_spawn);
		for (int i = 0; i < 2; i++) {
			if ((peepSpawns[i].x & 0xFFE0) == x && (peepSpawns[i].y & 0xFFE0) == y) {
				peepSpawns[i].x = 0xFFFF;
			}
		}

		rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
	next_element:
		switch (map_element_get_type(mapElement)) {
		case MAP_ELEMENT_TYPE_SURFACE:
			mapElement->base_height = 2;
			mapElement->clearance_height = 2;
			mapElement->properties.surface.slope = 0;
			mapElement->properties.surface.terrain = 0;
			mapElement->properties.surface.grass_length = 1;
			mapElement->properties.surface.ownership = 0;
			if (!map_element_is_last_for_tile(mapElement++))
				goto next_element;

			return;
		case MAP_ELEMENT_TYPE_ENTRANCE:
			viewport_interaction_remove_park_entrance(mapElement, x, y);
			break;
		case MAP_ELEMENT_TYPE_FENCE:
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, rct_string_id) = STR_CANT_REMOVE_THIS;
			game_do_command(
				x,
				GAME_COMMAND_FLAG_APPLY,
				y,
				(mapElement->type & MAP_ELEMENT_DIRECTION_MASK) | (mapElement->base_height << 8),
				GAME_COMMAND_REMOVE_FENCE,
				0,
				0
			);
			break;
		case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, rct_string_id) = STR_CANT_REMOVE_THIS;
			game_do_command(
				x,
				(GAME_COMMAND_FLAG_APPLY) | (mapElement->type & MAP_ELEMENT_DIRECTION_MASK),
				y,
				(mapElement->base_height) | (((mapElement->properties.scenerymultiple.type >> 8) >> 2) << 8),
				GAME_COMMAND_REMOVE_LARGE_SCENERY,
				0,
				0
			);
			break;
		case MAP_ELEMENT_TYPE_BANNER:
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, rct_string_id) = STR_CANT_REMOVE_THIS;
			game_do_command(
				x,
				GAME_COMMAND_FLAG_APPLY,
				y,
				(mapElement->base_height) | ((mapElement->properties.banner.position & 3) << 8),
				GAME_COMMAND_REMOVE_BANNER,
				0,
				0
			);
			break;
		default:
			map_element_remove(mapElement);
			break;
		}
	}
}

int map_get_highest_z(int tileX, int tileY)
{
	rct_map_element *mapElement;
	int z;

	mapElement = map_get_surface_element_at(tileX, tileY);
	if (mapElement == NULL)
		return -1;

	z = mapElement->base_height * 8;

	// Raise z so that is above highest point of land and water on tile
	if ((mapElement->properties.surface.slope & 0x0F) != 0)
		z += 16;
	if ((mapElement->properties.surface.slope & 0x10) != 0)
		z += 16;

	z = max(z, (mapElement->properties.surface.terrain & 0x1F) * 16);
	return z;
}

bool map_element_is_underground(rct_map_element *mapElement)
{
	do {
		mapElement++;
		if (map_element_is_last_for_tile(mapElement - 1))
			return false;
	} while (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SURFACE);
	return true;
}

rct_map_element *map_get_large_scenery_segment(int x, int y, int z, int direction, int sequence)
{
	rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE)
			continue;
		if (mapElement->base_height != z)
			continue;
		if ((mapElement->properties.scenerymultiple.type >> 10) != sequence)
			continue;
		if ((mapElement->type & MAP_ELEMENT_DIRECTION_MASK) != direction)
			continue;

		return mapElement;
	} while (!map_element_is_last_for_tile(mapElement++));
	return NULL;
}

bool map_large_scenery_get_origin(
	int x, int y, int z, int direction, int sequence,
	int *outX, int *outY, int *outZ
) {
	rct_map_element *mapElement;
	rct_scenery_entry *sceneryEntry;
	rct_large_scenery_tile *tile;
	sint16 offsetX, offsetY;

	mapElement = map_get_large_scenery_segment(x, y, z, direction, sequence);
	if (mapElement == NULL)
		return false;

	sceneryEntry = g_largeSceneryEntries[(mapElement->properties.scenerymultiple.type) & 0x3FF];
	tile = &sceneryEntry->large_scenery.tiles[sequence];

	offsetX = tile->x_offset;
	offsetY = tile->y_offset;
	rotate_map_coordinates(&offsetX, &offsetY, direction);

	*outX = x - offsetX;
	*outY = y - offsetY;
	*outZ = (z * 8) - tile->z_offset;
	return true;
}

/**
 *
 *  rct2: 0x006B9B05
 */
void sign_set_colour(int x, int y, int z, int direction, int sequence, uint8 mainColour, uint8 textColour)
{
	rct_map_element *mapElement;
	rct_scenery_entry *sceneryEntry;
	rct_large_scenery_tile *sceneryTiles, *tile;
	sint16 offsetX, offsetY;
	int x0, y0, z0;

	// Get the given segment of the large scenery element
	mapElement = map_get_large_scenery_segment(x, y, z, direction, sequence);
	if (mapElement == NULL)
		return;

	// Get the origin position of the large scenery element
	sceneryEntry = g_largeSceneryEntries[(mapElement->properties.scenerymultiple.type) & 0x3FF];
	sceneryTiles = sceneryEntry->large_scenery.tiles;
	tile = &sceneryTiles[sequence];
	offsetX = tile->x_offset;
	offsetY = tile->y_offset;
	rotate_map_coordinates(&offsetX, &offsetY, direction);
	x0 = x - offsetX;
	y0 = y - offsetY;
	z0 = (z * 8) - tile->z_offset;

	// Iterate through each tile of the large scenery element
	sequence = 0;
	for (tile = sceneryTiles; tile->x_offset != -1; tile++, sequence++) {
		offsetX = tile->x_offset;
		offsetY = tile->y_offset;
		rotate_map_coordinates(&offsetX, &offsetY, direction);

		x = x0 + offsetX;
		y = y0 + offsetY;
		z = (z0 + tile->z_offset) / 8;
		mapElement = map_get_large_scenery_segment(x, y, z, direction, sequence);
		if (mapElement != NULL) {
			mapElement->properties.scenerymultiple.colour[0] &= 0xE0;
			mapElement->properties.scenerymultiple.colour[1] &= 0xE0;
			mapElement->properties.scenerymultiple.colour[0] |= mainColour;
			mapElement->properties.scenerymultiple.colour[1] |= textColour;

			map_invalidate_tile(x, y, mapElement->base_height * 8 , mapElement->clearance_height * 8);
		}
	}
}

/**
 *
 *  rct2: 0x006E588E
 */
void map_remove_walls_at(int x, int y, int z0, int z1)
{
	rct_map_element *mapElement;

	z0 /= 8;
	z1 /= 8;
repeat:
	mapElement = map_get_first_element_at(x >> 5, y >> 5);
	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_FENCE)
			continue;
		if (z0 >= mapElement->clearance_height)
			continue;
		if (z1 <= mapElement->base_height)
			continue;

		map_element_remove_banner_entry(mapElement);
		map_invalidate_tile_zoom1(x, y, mapElement->base_height, mapElement->base_height + 72);
		map_element_remove(mapElement);
		goto repeat;
	} while (!map_element_is_last_for_tile(mapElement++));
}

/**
 *
 *  rct2: 0x006E57E6
 */
void map_remove_walls_at_z(int x, int y, int z)
{
	map_remove_walls_at(x, y, z, z + 48);
}

static void translate_3d_to_2d(int rotation, int *x, int *y)
{
	int rx, ry;

	switch (rotation & 3) {
	case 0:
		rx = (*y) - (*x);
		ry = (*x) + (*y);
		break;
	case 1:
		rx = -(*x) - (*y);
		ry = (*y) - (*x);
		break;
	case 2:
		rx = (*x) - (*y);
		ry = -(*x) - (*y);
		break;
	case 3:
		rx = (*x) + (*y);
		ry = (*x) - (*y);
		break;
	}
	ry /= 2;

	*x = rx;
	*y = ry;
}

void map_invalidate_tile_under_zoom(int x, int y, int z0, int z1, int maxZoom)
{
	int x1, y1, x2, y2;
	rct_viewport *viewport;

	x += 16;
	y += 16;
	translate_3d_to_2d(get_current_rotation(), &x, &y);

	x1 = x - 32;
	y1 = y - 32 - z1;
	x2 = x + 32;
	y2 = y + 32 - z0;

	viewport = RCT2_GLOBAL(RCT2_ADDRESS_ACTIVE_VIEWPORT_PTR_ARRAY, rct_viewport*);
	while (!gOpenRCT2Headless && viewport->width != 0) {
		if (maxZoom == -1 || viewport->zoom <= maxZoom) {
			viewport_invalidate(viewport, x1, y1, x2, y2);
		}
		viewport++;
	}
}

/**
 *
 *  rct2: 0x006EC847
 */
void map_invalidate_tile(int x, int y, int z0, int z1)
{
	map_invalidate_tile_under_zoom(x, y, z0, z1, -1);
}

/**
 *
 *  rct2: 0x006ECB60
 */
void map_invalidate_tile_zoom1(int x, int y, int z0, int z1)
{
	map_invalidate_tile_under_zoom(x, y, z0, z1, 1);
}

/**
 *
 *  rct2: 0x006EC9CE
 */
void map_invalidate_tile_zoom0(int x, int y, int z0, int z1)
{
	map_invalidate_tile_under_zoom(x, y, z0, z1, 0);
}

/**
 *
 *  rct2: 0x006EC6D7
 */
void map_invalidate_tile_full(int x, int y)
{
	map_invalidate_tile(x, y, 0, 2080);
}

void map_invalidate_element(int x, int y, rct_map_element *mapElement)
{
	map_invalidate_tile(x, y, mapElement->base_height, mapElement->clearance_height);
}

int map_get_tile_side(int mapX, int mapY)
{
	int subMapX = mapX & (32 - 1);
	int subMapY = mapY & (32 - 1);
	return (subMapX < subMapY) ?
		((subMapX + subMapY) < 32 ? 0 : 1):
		((subMapX + subMapY) < 32 ? 3 : 2);
}

int map_get_tile_quadrant(int mapX, int mapY)
{
	int subMapX = mapX & (32 - 1);
	int subMapY = mapY & (32 - 1);
	return (subMapX > 16) ?
		(subMapY < 16 ? 1 : 0):
		(subMapY < 16 ? 2 : 3);
}

/* rct2: 0x00693BFF */
bool map_surface_is_blocked(sint16 x, sint16 y){
	rct_map_element *mapElement;
	if (x >= 8192 || y >= 8192)
		return true;

	mapElement = map_get_surface_element_at(x / 32, y / 32);

	sint16 water_height = mapElement->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK;
	water_height *= 2;
	if (water_height > mapElement->base_height)
		return true;

	sint16 base_z = mapElement->base_height;
	sint16 clear_z = mapElement->base_height + 2;
	if (mapElement->properties.surface.slope & (1 << 4))
		clear_z += 2;

	while (!map_element_is_last_for_tile(mapElement++)){
		if (clear_z >= mapElement->clearance_height)
			continue;

		if (base_z < mapElement->base_height)
			continue;

		if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH ||
			map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_FENCE)
			continue;

		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY)
			return true;

		rct_scenery_entry* scenery = g_smallSceneryEntries[mapElement->properties.scenery.type];
		if (scenery->small_scenery.flags & SMALL_SCENERY_FLAG_FULL_TILE)
			return true;
	}
	return false;
}

/* Clears all map elements, to be used before generating a new map */
void map_clear_all_elements()
{
	for (int y = 0; y < (256 * 32); y += 32) {
		for (int x = 0; x < (256 * 32); x += 32) {
			sub_68AE2A(x, y);
		}
	}
}

money32 place_park_entrance(int flags, sint16 x, sint16 y, sint16 z, uint8 direction) {
	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_LAND_PURCHASE * 4;

	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_X, sint16) = x;
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Y, sint16) = y;
	// ??
	RCT2_GLOBAL(RCT2_ADDRESS_COMMAND_MAP_Z, sint16) = (z & 0xFF) << 4;

	if (!sub_68B044()) {
		return MONEY32_UNDEFINED;
	}

	if (x <= 32 || y <= 32 || x >= (RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16) - 32) || y >= (RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, sint16) - 32)) {
		RCT2_GLOBAL(0x00141E9AC, rct_string_id) = 3215;
		return MONEY32_UNDEFINED;
	}

	sint8 entranceNum = -1;
	for (uint8 i = 0; i < 4; ++i) {
		if (RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_X, sint16)[i] == (sint16)0x8000) {
			entranceNum = i;
			break;
		}
	}

	if (entranceNum == -1) {
		RCT2_GLOBAL(0x00141E9AC, rct_string_id) = 3227;
		return MONEY32_UNDEFINED;
	}

	if (flags & GAME_COMMAND_FLAG_APPLY) {
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_X, sint16)[entranceNum] = x;
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_Y, sint16)[entranceNum] = y;
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_Z, sint16)[entranceNum] = (z & 0xFF) << 4;
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_DIRECTION, uint8)[entranceNum] = direction;
	}

	sint8 zLow = (z & 0xFF) * 2;
	sint8 zHigh = zLow + 12;
	if (!map_can_construct_at(x, y, zLow, zHigh, 0xF)) {
		return MONEY32_UNDEFINED;
	}

	if (flags & GAME_COMMAND_FLAG_APPLY) {

		if (!(flags & GAME_COMMAND_FLAG_GHOST)) {
			rct_map_element* surfaceElement = map_get_surface_element_at(x / 32, y / 32);
			surfaceElement->properties.surface.ownership = 0;
		}

		rct_map_element* newElement = map_element_insert(x / 32, y / 32, zLow, 0xF);
		newElement->clearance_height = zHigh;

		if (flags & GAME_COMMAND_FLAG_GHOST) {
			newElement->flags |= MAP_ELEMENT_FLAG_GHOST;
		}

		newElement->type = MAP_ELEMENT_TYPE_ENTRANCE;
		newElement->type |= direction;
		newElement->properties.entrance.index = 0;
		newElement->properties.entrance.type = ENTRANCE_TYPE_PARK_ENTRANCE;
		newElement->properties.entrance.path_type = RCT2_GLOBAL(RCT2_ADDRESS_SELECTED_PATH_ID, uint16) & 0xFF;

		if (!(flags & GAME_COMMAND_FLAG_GHOST)) {
			footpath_connect_edges(x, y, newElement, 1);
		}

		update_park_fences(x, y);
		update_park_fences(x - 32, y);
		update_park_fences(x + 32, y);
		update_park_fences(x, y - 32);
		update_park_fences(x, y + 32);

		map_invalidate_tile(x, y, newElement->base_height, newElement->clearance_height);

		map_animation_create(MAP_ANIMATION_TYPE_PARK_ENTRANCE, x, y, zLow);
	}

	x += TileDirectionDelta[(direction - 1) & 0x3].x;
	y += TileDirectionDelta[(direction - 1) & 0x3].y;

	if (!map_can_construct_at(x, y, zLow, zHigh, 0xF)) {
		return MONEY32_UNDEFINED;
	}

	if (flags & GAME_COMMAND_FLAG_APPLY) {

		if (!(flags & GAME_COMMAND_FLAG_GHOST)) {
			rct_map_element* surfaceElement = map_get_surface_element_at(x / 32, y / 32);
			surfaceElement->properties.surface.ownership = 0;
		}

		rct_map_element* newElement = map_element_insert(x / 32, y / 32, zLow, 0xF);
		newElement->clearance_height = zHigh;

		if (flags & GAME_COMMAND_FLAG_GHOST) {
			newElement->flags |= MAP_ELEMENT_FLAG_GHOST;
		}

		newElement->type = MAP_ELEMENT_TYPE_ENTRANCE;
		newElement->type |= direction;
		newElement->properties.entrance.index = 1;
		newElement->properties.entrance.type = ENTRANCE_TYPE_PARK_ENTRANCE;

		update_park_fences(x, y);
		update_park_fences(x - 32, y);
		update_park_fences(x + 32, y);
		update_park_fences(x, y - 32);
		update_park_fences(x, y + 32);

		map_invalidate_tile(x, y, newElement->base_height, newElement->clearance_height);
	}

	x += TileDirectionDelta[(direction + 1) & 0x3].x * 2;
	y += TileDirectionDelta[(direction + 1) & 0x3].y * 2;

	if (!map_can_construct_at(x, y, zLow, zHigh, 0xF)) {
		return MONEY32_UNDEFINED;
	}

	if (flags & GAME_COMMAND_FLAG_APPLY) {

		if (!(flags & GAME_COMMAND_FLAG_GHOST)) {
			rct_map_element* surfaceElement = map_get_surface_element_at(x / 32, y / 32);
			surfaceElement->properties.surface.ownership = 0;
		}

		rct_map_element* newElement = map_element_insert(x / 32, y / 32, zLow, 0xF);
		newElement->clearance_height = zHigh;

		if (flags & GAME_COMMAND_FLAG_GHOST) {
			newElement->flags |= MAP_ELEMENT_FLAG_GHOST;
		}

		newElement->type = MAP_ELEMENT_TYPE_ENTRANCE;
		newElement->type |= direction;
		newElement->properties.entrance.index = 2;
		newElement->properties.entrance.type = ENTRANCE_TYPE_PARK_ENTRANCE;

		update_park_fences(x, y);
		update_park_fences(x - 32, y);
		update_park_fences(x + 32, y);
		update_park_fences(x, y - 32);
		update_park_fences(x, y + 32);

		map_invalidate_tile(x, y, newElement->base_height, newElement->clearance_height);
	}

	return 0;
}

/* rct2: 0x006666E7 */
void game_command_place_park_entrance(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp) {
	*ebx = place_park_entrance(
		*ebx & 0xFF,
		*eax & 0xFFFF,
		*ecx & 0xFFFF,
		*edx & 0xFFFF,
		(*ebx >> 8) & 0xFF);
}

void game_command_set_banner_name(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp) {
	static char newName[128];

	rct_banner* banner = &gBanners[*ecx];

	int nameChunkIndex = *eax & 0xFFFF;

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_RIDE_RUNNING_COSTS * 4;
	int nameChunkOffset = nameChunkIndex - 1;
	if (nameChunkOffset < 0)
		nameChunkOffset = 2;
	nameChunkOffset *= 12;
	RCT2_GLOBAL(newName + nameChunkOffset + 0, uint32) = *edx;
	RCT2_GLOBAL(newName + nameChunkOffset + 4, uint32) = *ebp;
	RCT2_GLOBAL(newName + nameChunkOffset + 8, uint32) = *edi;

	if (nameChunkIndex != 0) {
		*ebx = 0;
		return;
	}

	utf8 *buffer = RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, uint8);
	utf8 *dst = buffer;
	dst = utf8_write_codepoint(dst, FORMAT_COLOUR_CODE_START + banner->text_colour);
	strncpy(dst, newName, 32);

	rct_string_id stringId = user_string_allocate(128, buffer);
	if (stringId) {
		rct_string_id prev_string_id = banner->string_idx;
		banner->string_idx = stringId;
		user_string_free(prev_string_id);
		rct_window* w = window_bring_to_front_by_number(WC_BANNER, *ecx);
		if (w) {
			window_invalidate(w);
		}
	} else {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 2984;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	*ebx = 0;
}

void game_command_set_sign_name(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp) {
	static char newName[128];

	rct_banner* banner = &gBanners[*ecx];
	int x = banner->x << 5;
	int y = banner->y << 5;

	int nameChunkIndex = *eax & 0xFFFF;

	RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) = RCT_EXPENDITURE_TYPE_RIDE_RUNNING_COSTS * 4;
	int nameChunkOffset = nameChunkIndex - 1;
	if (nameChunkOffset < 0)
		nameChunkOffset = 2;
	nameChunkOffset *= 12;
	RCT2_GLOBAL(newName + nameChunkOffset + 0, uint32) = *edx;
	RCT2_GLOBAL(newName + nameChunkOffset + 4, uint32) = *ebp;
	RCT2_GLOBAL(newName + nameChunkOffset + 8, uint32) = *edi;

	if (nameChunkIndex != 0) {
		*ebx = 0;
		return;
	}

	if (newName[0] != 0) {
		rct_string_id string_id = user_string_allocate(128, newName);
		if (string_id != 0) {
			rct_string_id prev_string_id = banner->string_idx;
			banner->string_idx = string_id;
			user_string_free(prev_string_id);

			banner->flags &= ~(BANNER_FLAG_2);
			gfx_invalidate_screen();
		} else {
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 2984;
			*ebx = MONEY32_UNDEFINED;
			return;
		}
	}
	else{
		int rideIndex = banner_get_closest_ride_index(x, y, 16);
		if (rideIndex == -1) {
			*ebx = 0;
			return;
		}

		banner->colour = rideIndex;
		banner->flags |= BANNER_FLAG_2;

		rct_string_id prev_string_id = banner->string_idx;
		banner->string_idx = 778;
		user_string_free(prev_string_id);
		gfx_invalidate_screen();
	}

	*ebx = 0;
}

void game_command_set_banner_style(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp) {
	rct_banner* banner = &gBanners[*ecx];

	banner->colour = (uint8)*edx;
	banner->text_colour = (uint8)*edi;

	banner->flags = (uint8)*ebp;

	int x = banner->x << 5;
	int y = banner->y << 5;

	rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);

	while (1){
		if ((map_element_get_type(map_element) == MAP_ELEMENT_TYPE_BANNER) &&
			(map_element->properties.banner.index == *ecx)) break;
		map_element++;
	}

	map_element->properties.banner.flags = 0xFF;
	if (banner->flags & BANNER_FLAG_NO_ENTRY){
		map_element->properties.banner.flags &= ~(1 << map_element->properties.banner.position);
	}

	int colourCodepoint = FORMAT_COLOUR_CODE_START + banner->text_colour;

	uint8 buffer[256];
	format_string(buffer, banner->string_idx, 0);
	int firstCodepoint = utf8_get_next(buffer, NULL);
	if (firstCodepoint >= FORMAT_COLOUR_CODE_START && firstCodepoint <= FORMAT_COLOUR_CODE_END) {
		utf8_write_codepoint(buffer, colourCodepoint);
	} else {
		utf8_insert_codepoint(buffer, colourCodepoint);
	}

	rct_string_id stringId = user_string_allocate(128, buffer);
	if (stringId != 0) {
		rct_string_id prev_string_id = banner->string_idx;
		banner->string_idx = stringId;
		user_string_free(prev_string_id);
		rct_window* w = window_bring_to_front_by_number(WC_BANNER, *ecx);
		if (w) {
			window_invalidate(w);
		}
	} else {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 2984;
		*ebx = MONEY32_UNDEFINED;
		return;
	}

	*ebx = 0;
}

void game_command_set_sign_style(int* eax, int* ebx, int* ecx, int* edx, int* esi, int* edi, int* ebp) {
	rct_banner *banner = &gBanners[*ecx];
	int x = banner->x << 5;
	int y = banner->y << 5;

	banner->colour = (uint8)*edx;
	banner->text_colour = (uint8)*edi;
	
	if (*ebp == 0) { // small sign
		
		rct_map_element* map_element = map_get_first_element_at(x / 32, y / 32);

		while (1){
			if (map_element_get_type(map_element) == MAP_ELEMENT_TYPE_FENCE) {
				rct_scenery_entry* scenery_entry = g_wallSceneryEntries[map_element->properties.fence.type];
				if (scenery_entry->wall.var_0D != 0xFF){
					if (map_element->properties.fence.item[0] == *ecx)
						break;
				}
			}
			map_element++;
		}
		map_element->flags &= 0x9F;
		map_element->properties.fence.item[1] =
			banner->colour |
			((banner->text_colour & 0x7) << 5);
		map_element->flags |= ((banner->text_colour & 0x18) << 2);

		map_invalidate_tile(x, y, map_element->base_height * 8, map_element->clearance_height * 8);
	} else { // large sign
		rct_map_element *mapElement = banner_get_map_element(*ecx);
		if (mapElement == NULL || map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_SCENERY_MULTIPLE) {
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 2984;
			*ebx = MONEY32_UNDEFINED;
			return;
		}

		sign_set_colour(
			banner->x * 32,
			banner->y * 32,
			mapElement->base_height,
			mapElement->type & 3,
			mapElement->properties.scenerymultiple.type >> 10,
			banner->colour,
			banner->text_colour
		);
	}

	rct_window* w = window_bring_to_front_by_number(WC_BANNER, *ecx);
	if (w) {
		window_invalidate(w);
	}

	*ebx = 0;
}

/**
 * Gets the map element at x, y, z.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 */
rct_map_element *map_get_track_element_at(int x, int y, int z)
{
	rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
		if (mapElement->base_height != z) continue;

		return mapElement;
	} while (!map_element_is_last_for_tile(mapElement++));

	return NULL;
}

/**
 * Gets the map element at x, y, z.
 * @param x x units, not tiles.
 * @param y y units, not tiles.
 * @param z Base height.
 */
rct_map_element *map_get_track_element_at_of_type(int x, int y, int z, int trackType)
{
	rct_map_element *mapElement = map_get_first_element_at(x >> 5, y >> 5);
	do {
		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_TRACK) continue;
		if (mapElement->base_height != z) continue;
		if (mapElement->properties.track.type != trackType) continue;

		return mapElement;
	} while (!map_element_is_last_for_tile(mapElement++));

	return NULL;
}
