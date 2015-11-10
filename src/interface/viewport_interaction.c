/*****************************************************************************
 * Copyright (c) 2014 Ted John
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
#include "../editor.h"
#include "../game.h"
#include "../localisation/localisation.h"
#include "../ride/ride.h"
#include "../ride/ride_data.h"
#include "../scenario.h"
#include "../world/banner.h"
#include "../world/footpath.h"
#include "../world/map.h"
#include "../world/scenery.h"
#include "../world/sprite.h"
#include "../input.h"
#include "viewport.h"
#include "../cheats.h"

static void viewport_interaction_remove_scenery(rct_map_element *mapElement, int x, int y);
static void viewport_interaction_remove_footpath(rct_map_element *mapElement, int x, int y);
static void viewport_interaction_remove_footpath_item(rct_map_element *mapElement, int x, int y);
static void viewport_interaction_remove_park_wall(rct_map_element *mapElement, int x, int y);
static void viewport_interaction_remove_large_scenery(rct_map_element *mapElement, int x, int y);
static rct_peep *viewport_interaction_get_closest_peep(int x, int y, int maxDistance);

/**
 *
 *  rct2: 0x006ED9D0
 */
int viewport_interaction_get_item_left(int x, int y, viewport_interaction_info *info)
{
	rct_s6_info *s6Info = (rct_s6_info*)0x00141F570;
	rct_map_element *mapElement;
	rct_sprite *sprite;
	rct_vehicle *vehicle;

	// No click input for title screen or scenario editor or track manager
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & (SCREEN_FLAGS_TITLE_DEMO | SCREEN_FLAGS_SCENARIO_EDITOR | SCREEN_FLAGS_TRACK_MANAGER))
		return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

	//
	if ((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER) && s6Info->editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER)
		return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

	rct_xy16 mapCoord = { 0 };
	get_map_coordinates_from_pos(x, y, VIEWPORT_INTERACTION_MASK_SPRITE & VIEWPORT_INTERACTION_MASK_RIDE & VIEWPORT_INTERACTION_MASK_PARK, &mapCoord.x, &mapCoord.y, &info->type, &info->mapElement, NULL);
	info->x = mapCoord.x;
	info->y = mapCoord.y;
	mapElement = info->mapElement;
	sprite = (rct_sprite*)mapElement;

	switch (info->type) {
	case VIEWPORT_INTERACTION_ITEM_SPRITE:
		switch (sprite->unknown.sprite_identifier) {
		case SPRITE_IDENTIFIER_VEHICLE:
			vehicle = &(sprite->vehicle);
			if (vehicle->ride_subtype != 255)
				vehicle_set_map_toolbar(vehicle);
			else
				info->type = VIEWPORT_INTERACTION_ITEM_NONE;
			break;
		case SPRITE_IDENTIFIER_PEEP:
			peep_set_map_tooltip(&sprite->peep);
			break;
		}
		break;
	case VIEWPORT_INTERACTION_ITEM_RIDE:
		ride_set_map_tooltip(mapElement);
		break;
	case VIEWPORT_INTERACTION_ITEM_PARK:
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = RCT2_GLOBAL(RCT2_ADDRESS_PARK_NAME, rct_string_id);
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint32) = RCT2_GLOBAL(RCT2_ADDRESS_PARK_NAME_ARGS, uint32);
		break;
	default:
		info->type = VIEWPORT_INTERACTION_ITEM_NONE;
		break;
	}

	// If nothing is under cursor, find a closeby peep
	if (info->type == VIEWPORT_INTERACTION_ITEM_NONE) {
		info->peep = viewport_interaction_get_closest_peep(x, y, 32);
		if (info->peep == NULL)
			return VIEWPORT_INTERACTION_ITEM_NONE;

		info->type = VIEWPORT_INTERACTION_ITEM_SPRITE;
		info->x = info->peep->x;
		info->y = info->peep->y;
		peep_set_map_tooltip(info->peep);
	}

	return info->type;
}

int viewport_interaction_left_over(int x, int y)
{
	viewport_interaction_info info;

	switch (viewport_interaction_get_item_left(x, y, &info)) {
	case VIEWPORT_INTERACTION_ITEM_SPRITE:
	case VIEWPORT_INTERACTION_ITEM_RIDE:
	case VIEWPORT_INTERACTION_ITEM_PARK:
		return 1;
	default:
		return 0;
	}
}

int viewport_interaction_left_click(int x, int y)
{
	viewport_interaction_info info;

	switch (viewport_interaction_get_item_left(x, y, &info)) {
	case VIEWPORT_INTERACTION_ITEM_SPRITE:
		switch (info.sprite->unknown.sprite_identifier) {
		case SPRITE_IDENTIFIER_VEHICLE:
			window_ride_open_vehicle(info.vehicle);
			break;
		case SPRITE_IDENTIFIER_PEEP:
			window_guest_open(info.peep);
			break;
		case SPRITE_IDENTIFIER_MISC:
			if (RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) == 0) {
				switch (info.sprite->unknown.misc_identifier) {
				case SPRITE_MISC_BALLOON:
					balloon_press(&info.sprite->balloon);
					break;
				case SPRITE_MISC_DUCK:
					duck_press(&info.sprite->duck);
					break;
				}
			}
			break;
		}
		return 1;
	case VIEWPORT_INTERACTION_ITEM_RIDE:
		window_ride_open_track(info.mapElement);
		return 1;
	case VIEWPORT_INTERACTION_ITEM_PARK:
		window_park_entrance_open();
		return 1;
	default:
		return 0;
	}
}

/**
 *
 *  rct2: 0x006EDE88
 */
int viewport_interaction_get_item_right(int x, int y, viewport_interaction_info *info)
{
	rct_s6_info *s6Info = (rct_s6_info*)0x00141F570;
	rct_map_element *mapElement;
	rct_sprite *sprite;
	rct_scenery_entry *sceneryEntry;
	rct_banner *banner;
	rct_ride *ride;
	int i, stationIndex;

	// No click input for title screen or track manager
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & (SCREEN_FLAGS_TITLE_DEMO | SCREEN_FLAGS_TRACK_MANAGER))
		return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

	//
	if ((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER) && s6Info->editor_step != EDITOR_STEP_ROLLERCOASTER_DESIGNER)
		return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

	rct_xy16 mapCoord = { 0 };
	get_map_coordinates_from_pos(x, y, ~(VIEWPORT_INTERACTION_MASK_TERRAIN & VIEWPORT_INTERACTION_MASK_WATER), &mapCoord.x, &mapCoord.y, &info->type, &info->mapElement, NULL);
	info->x = mapCoord.x;
	info->y = mapCoord.y;
	mapElement = info->mapElement;
	sprite = (rct_sprite*)mapElement;

	switch (info->type) {
	case VIEWPORT_INTERACTION_ITEM_SPRITE:
		if ((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) || mapElement->type != 0)
			return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

		mapElement += 6;
		ride = GET_RIDE(mapElement->type);
		if (ride->status == RIDE_STATUS_CLOSED) {
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1163;
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = ride->name;
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 4, uint32) = ride->name_arguments;
		}
		return info->type;

	case VIEWPORT_INTERACTION_ITEM_RIDE:
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR)
			return info->type = VIEWPORT_INTERACTION_ITEM_NONE;
		if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_PATH)
			return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

		ride = GET_RIDE(mapElement->properties.track.ride_index);
		if (ride->status != RIDE_STATUS_CLOSED)
			return info->type;

		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1163;

		if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_ENTRANCE) {
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) =
				mapElement->properties.track.type == ENTRANCE_TYPE_RIDE_ENTRANCE ? 1335 : 1337;
		} else if (mapElement->properties.track.type == 1 || mapElement->properties.track.type == 2 || mapElement->properties.track.type == 3) {
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = 1333;
		} else {
			if (!map_is_location_owned(info->x, info->y, mapElement->base_height << 4))
				return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = ride->name;
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 4, uint32) = ride->name_arguments;
			return info->type;
		}

		if (ride->num_stations > 1)
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16)++;

		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 4, uint16) = ride->name;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 6, uint32) = ride->name_arguments;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 10, uint16) = RideNameConvention[ride->type].station_name + 2;

		stationIndex = map_get_station(mapElement);
		for (i = stationIndex; i >= 0; i--)
			if (ride->station_starts[i] == 0xFFFF)
				stationIndex--;
		stationIndex++;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 12, uint16) = stationIndex;
		return info->type;

	case VIEWPORT_INTERACTION_ITEM_WALL:
		sceneryEntry = g_wallSceneryEntries[mapElement->properties.scenery.type];
		if (sceneryEntry->wall.var_0D != 255) {
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1163;
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = sceneryEntry->name;
			return info->type;
		}
		break;

	case VIEWPORT_INTERACTION_ITEM_LARGE_SCENERY:
		sceneryEntry = g_largeSceneryEntries[mapElement->properties.scenerymultiple.type & 0x3FF];
		if (sceneryEntry->large_scenery.var_11 != 255) {
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1163;
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = sceneryEntry->name;
			return info->type;
		}
		break;

	case VIEWPORT_INTERACTION_ITEM_BANNER:
		banner = &gBanners[mapElement->properties.banner.index];
		sceneryEntry = g_bannerSceneryEntries[banner->type];

		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1163;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = sceneryEntry->name;
		return info->type;
	}

	if ((RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & (INPUT_FLAG_6 | INPUT_FLAG_TOOL_ACTIVE)) != (INPUT_FLAG_6 | INPUT_FLAG_TOOL_ACTIVE))
		if (window_find_by_class(WC_RIDE_CONSTRUCTION) == NULL && window_find_by_class(WC_FOOTPATH) == NULL)
			return info->type = VIEWPORT_INTERACTION_ITEM_NONE;

	switch (info->type) {
	case VIEWPORT_INTERACTION_ITEM_SCENERY:
		sceneryEntry = g_smallSceneryEntries[mapElement->properties.scenery.type];
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1164;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = sceneryEntry->name;
		return info->type;

	case VIEWPORT_INTERACTION_ITEM_FOOTPATH:
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1164;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = 1425;
		if (mapElement->type & 1)
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = 1426;
		return info->type;

	case VIEWPORT_INTERACTION_ITEM_FOOTPATH_ITEM:
		sceneryEntry = g_pathBitSceneryEntries[(mapElement->properties.path.additions & 0x0F) - 1];
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1164;
		if (mapElement->flags & 0x20) {
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = 3124;
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 4, uint16) = sceneryEntry->name;
		} else {
			RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = sceneryEntry->name;
		}
		return info->type;

	case VIEWPORT_INTERACTION_ITEM_PARK:
		if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && !gCheatsSandboxMode)
			break;

		if (map_element_get_type(mapElement) != MAP_ELEMENT_TYPE_ENTRANCE)
			break;

		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1164;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = 3192;
		return info->type;

	case VIEWPORT_INTERACTION_ITEM_WALL:
		sceneryEntry = g_wallSceneryEntries[mapElement->properties.scenery.type];
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1164;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = sceneryEntry->name;
		return info->type;

	case VIEWPORT_INTERACTION_ITEM_LARGE_SCENERY:
		sceneryEntry = g_largeSceneryEntries[mapElement->properties.scenery.type & 0x3FF];
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1164;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = sceneryEntry->name;
		return info->type;
	}

	return info->type = VIEWPORT_INTERACTION_ITEM_NONE;
}

int viewport_interaction_right_over(int x, int y)
{
	viewport_interaction_info info;

	return viewport_interaction_get_item_right(x, y, &info) != 0;
}

/**
 *
 *  rct2: 0x006E8A62
 */
int viewport_interaction_right_click(int x, int y)
{
	rct_xy_element mapElement;
	viewport_interaction_info info;

	switch (viewport_interaction_get_item_right(x, y, &info)) {
	case VIEWPORT_INTERACTION_ITEM_NONE:
		return 0;

	case VIEWPORT_INTERACTION_ITEM_SPRITE:
		if (info.sprite->unknown.sprite_identifier == SPRITE_IDENTIFIER_VEHICLE)
			ride_construct(info.sprite->vehicle.ride);
		break;
	case VIEWPORT_INTERACTION_ITEM_RIDE:
		mapElement.x = info.x;
		mapElement.y = info.y;
		mapElement.element = info.mapElement;
		ride_modify(&mapElement);
		break;
	case VIEWPORT_INTERACTION_ITEM_SCENERY:
		viewport_interaction_remove_scenery(info.mapElement, info.x, info.y);
		break;
	case VIEWPORT_INTERACTION_ITEM_FOOTPATH:
		viewport_interaction_remove_footpath(info.mapElement, info.x, info.y);
		break;
	case VIEWPORT_INTERACTION_ITEM_FOOTPATH_ITEM:
		viewport_interaction_remove_footpath_item(info.mapElement, info.x, info.y);
		break;
	case VIEWPORT_INTERACTION_ITEM_PARK:
		viewport_interaction_remove_park_entrance(info.mapElement, info.x, info.y);
		break;
	case VIEWPORT_INTERACTION_ITEM_WALL:
		viewport_interaction_remove_park_wall(info.mapElement, info.x, info.y);
		break;
	case VIEWPORT_INTERACTION_ITEM_LARGE_SCENERY:
		viewport_interaction_remove_large_scenery(info.mapElement, info.x, info.y);
		break;
	case VIEWPORT_INTERACTION_ITEM_BANNER:
		window_banner_open(info.mapElement->properties.banner.index);
		break;
	}

	return 1;
}

/**
 *
 *  rct2: 0x006E08D2
 */
static void viewport_interaction_remove_scenery(rct_map_element *mapElement, int x, int y)
{
	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_CANT_REMOVE_THIS;
	game_do_command(
		x,
		(mapElement->type << 8) | 1,
		y,
		(mapElement->properties.scenery.type << 8) | mapElement->base_height,
		GAME_COMMAND_REMOVE_SCENERY,
		0,
		0
	);
}

/**
 *
 *  rct2: 0x006A614A
 */
static void viewport_interaction_remove_footpath(rct_map_element *mapElement, int x, int y)
{
	int z;
	rct_window *w;
	rct_map_element *mapElement2;

	z = mapElement->base_height;

	w = window_find_by_class(WC_FOOTPATH);
	if (w != NULL)
		footpath_provisional_update();

	mapElement2 = map_get_first_element_at(x / 32, y / 32);
	do {
		if (map_element_get_type(mapElement2) == MAP_ELEMENT_TYPE_PATH && mapElement2->base_height == z) {
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_CANT_REMOVE_FOOTPATH_FROM_HERE;
			footpath_remove(x, y, z, 1);
			break;
		}
	} while (!map_element_is_last_for_tile(mapElement2++));
}

/**
 *
 *  rct2: 0x006A61AB
 */
static void viewport_interaction_remove_footpath_item(rct_map_element *mapElement, int x, int y)
{
	int type;

	type = mapElement->properties.path.type >> 4;
	if (mapElement->type & 1)
		type |= 0x80;

	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_CANT_REMOVE_THIS;
	game_do_command(
		x,
		((mapElement->properties.path.type & 7) << 8) | 1,
		y,
		(type << 8) | mapElement->base_height,
		GAME_COMMAND_PLACE_PATH,
		0,
		0
	);
}

/**
 *
 *  rct2: 0x00666C0E
 */
void viewport_interaction_remove_park_entrance(rct_map_element *mapElement, int x, int y)
{
	int rotation = (mapElement->type + 1) & 3;
	switch (mapElement->properties.entrance.index & 0x0F) {
	case 1:
		x += TileDirectionDelta[rotation].x;
		y += TileDirectionDelta[rotation].y;
		break;
	case 2:
		x -= TileDirectionDelta[rotation].x;
		y -= TileDirectionDelta[rotation].y;
		break;
	}
	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_CANT_REMOVE_THIS;
	game_do_command(x, GAME_COMMAND_FLAG_APPLY, y, mapElement->base_height / 2, GAME_COMMAND_REMOVE_PARK_ENTRANCE, 0, 0);
}

/**
 *
 *  rct2: 0x006E57A9
 */
static void viewport_interaction_remove_park_wall(rct_map_element *mapElement, int x, int y)
{
	rct_scenery_entry* sceneryEntry;

	sceneryEntry = g_wallSceneryEntries[mapElement->properties.fence.type];
	if (sceneryEntry->wall.var_0D != 0xFF){
		window_sign_small_open(mapElement->properties.fence.item[0]);
	} else {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, rct_string_id) = STR_CANT_REMOVE_THIS;
		game_do_command(
			x,
			GAME_COMMAND_FLAG_APPLY,
			y,
			(mapElement->type & 0x3) | (mapElement->base_height << 8),
			GAME_COMMAND_REMOVE_FENCE,
			0,
			0
		);
	}
}

/**
 *
 *  rct2: 0x006B88DC
 */
static void viewport_interaction_remove_large_scenery(rct_map_element *mapElement, int x, int y)
{
	rct_scenery_entry* sceneryEntry;

	sceneryEntry = g_largeSceneryEntries[mapElement->properties.scenerymultiple.type & MAP_ELEMENT_LARGE_TYPE_MASK];

	if (sceneryEntry->large_scenery.var_11 != 0xFF){
		int id = (mapElement->type & 0xC0) |
			((mapElement->properties.scenerymultiple.colour[0] & 0xE0) >> 2) |
			((mapElement->properties.scenerymultiple.colour[1] & 0xE0) >> 5);
		window_sign_open(id);
	} else {
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, rct_string_id) = STR_CANT_REMOVE_THIS;
		game_do_command(
			x,
			1 | ((mapElement->type & 0x3) << 8),
			y,
			mapElement->base_height | ((mapElement->properties.scenerymultiple.type >> 10) << 8),
			GAME_COMMAND_REMOVE_LARGE_SCENERY,
			0,
			0
		);
	}
}

static rct_peep *viewport_interaction_get_closest_peep(int x, int y, int maxDistance)
{
	int distance, closestDistance;
	uint16 spriteIndex;
	rct_window *w;
	rct_viewport *viewport;
	rct_peep *peep, *closestPeep;

	w = window_find_from_point(x, y);
	if (w == NULL)
		return 0;

	viewport = w->viewport;
	if (viewport == NULL || viewport->zoom >= 2)
		return 0;

	x = ((x - viewport->x) << viewport->zoom) + viewport->view_x;
	y = ((y - viewport->y) << viewport->zoom) + viewport->view_y;

	closestPeep = NULL;
	closestDistance = 0xFFFF;
	FOR_ALL_PEEPS(spriteIndex, peep) {
		if (peep->sprite_left == (sint16)0x8000)
			continue;

		distance =
			abs(((peep->sprite_left + peep->sprite_right) / 2) - x) +
			abs(((peep->sprite_top + peep->sprite_bottom) / 2) - y);
		if (distance > maxDistance)
			continue;

		if (distance < closestDistance) {
			closestPeep = peep;
			closestDistance = distance;
		}
	}

	return closestPeep;
}

/**
 *
 *  rct2: 0x0068A15E
 */
void sub_68A15E(int screenX, int screenY, short *x, short *y, int *direction, rct_map_element **mapElement)
{
	sint16 my_x, my_y;
	int z = 0, interactionType;
	rct_map_element *myMapElement;
	rct_viewport *viewport;
	get_map_coordinates_from_pos(screenX, screenY, VIEWPORT_INTERACTION_MASK_TERRAIN & VIEWPORT_INTERACTION_MASK_WATER, &my_x, &my_y, &interactionType, &myMapElement, &viewport);

	if (interactionType == VIEWPORT_INTERACTION_ITEM_NONE) {
		*x = 0x8000;
		return;
	}

	RCT2_GLOBAL(0x00F1AD3E, uint8) = interactionType;
	RCT2_GLOBAL(0x00F1AD30, rct_map_element*) = myMapElement;

	if (interactionType == VIEWPORT_INTERACTION_ITEM_WATER) {
		z = myMapElement->properties.surface.terrain;
		z = (z & MAP_ELEMENT_WATER_HEIGHT_MASK) << 4;
	}

	RCT2_GLOBAL(0x00F1AD3C, uint16) = z;
	RCT2_GLOBAL(0x00F1AD34, sint16) = my_x;
	RCT2_GLOBAL(0x00F1AD36, sint16) = my_y;
	RCT2_GLOBAL(0x00F1AD38, sint16) = my_x + 31;
	RCT2_GLOBAL(0x00F1AD3A, sint16) = my_y + 31;

	rct_xy16 start_vp_pos = screen_coord_to_viewport_coord(viewport, screenX, screenY);
	rct_xy16 map_pos = { my_x + 16, my_y + 16 };

	for (int i = 0; i < 5; i++) {
		if (RCT2_GLOBAL(0x00F1AD3E, uint8) != 4) {
			z = map_element_height(map_pos.x, map_pos.y);
		} else {
			z = RCT2_GLOBAL(0x00F1AD3C, uint16);
		}
		map_pos = viewport_coord_to_map_coord(start_vp_pos.x, start_vp_pos.y, z);
		map_pos.x = clamp(RCT2_GLOBAL(0x00F1AD34, sint16), map_pos.x, RCT2_GLOBAL(0x00F1AD38, sint16));
		map_pos.y = clamp(RCT2_GLOBAL(0x00F1AD36, sint16), map_pos.y, RCT2_GLOBAL(0x00F1AD3A, sint16));
	}

	// Determine to which edge the cursor is closest
	int myDirection;
	int mod_x = map_pos.x & 0x1F;
	int mod_y = map_pos.y & 0x1F;
	if (mod_x < mod_y) {
		if (mod_x + mod_y < 32) {
			myDirection = 0;
		} else {
			myDirection = 1;
		}
	} else {
		if (mod_x + mod_y < 32) {
			myDirection = 3;
		} else {
			myDirection = 2;
		}
	}

	*x = map_pos.x & ~0x1F;
	*y = map_pos.y & ~0x1F;
	if (direction != NULL) *direction = myDirection;
	if (mapElement != NULL) *mapElement = myMapElement;
}
