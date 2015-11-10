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

#include "addresses.h"
#include "config.h"
#include "editor.h"
#include "interface/viewport.h"
#include "interface/window.h"
#include "localisation/localisation.h"
#include "management/finance.h"
#include "object.h"
#include "rct1.h"
#include "ride/ride.h"
#include "scenario.h"
#include "util/sawyercoding.h"
#include "util/util.h"
#include "world/climate.h"
#include "world/footpath.h"
#include "world/map.h"
#include "world/map_animation.h"
#include "world/scenery.h"

typedef struct {
	const rct_object_entry* entries;
	int count;
} RCT1DefaultObjectsGroup;

static const uint8 RCT1TerrainConvertTable[16];
static const uint8 RCT1TerrainEdgeConvertTable[16];
static const uint8 RCT1PathTypeConversionTable[96];
static const uint8 RCT1PathAdditionConversionTable[15];
static const RCT1DefaultObjectsGroup RCT1DefaultObjects[10];

static void rct1_remove_rides();
static void rct1_load_default_objects();
static void rct1_fix_terrain();
static void rct1_fix_scenery();
static void rct1_fix_entrance_positions();
static void rct1_reset_research();

static void rct1_process_scenario_flags();
static void rct1_reset_park_entrance_path_type();
static void rct1_clear_extra_sprite_entries();
static void rct1_clear_extra_tile_entries();
static void rct1_fix_colours();
static void rct1_fix_z();
static void rct1_fix_paths();
static void rct1_fix_walls();
static void sub_69E891();

bool rct1_read_sc4(const char *path, rct1_s4 *s4)
{
	uint8 *buffer, *decodedBuffer;
	long length, decodedLength;
	bool success;

	if (!readentirefile(path, (void**)&buffer, (int*)&length)) {
		RCT2_GLOBAL(RCT2_ADDRESS_ERROR_TYPE, uint8) = 255;
		RCT2_GLOBAL(RCT2_ADDRESS_ERROR_STRING_ID, uint16) = 3011;
		return 0;
	}

	int fileType = sawyercoding_detect_file_type(buffer, length);

	decodedBuffer = malloc(sizeof(rct1_s4));
	decodedLength = (fileType & FILE_VERSION_MASK) == FILE_VERSION_RCT1 ?
		sawyercoding_decode_sv4(buffer, decodedBuffer, length) :
		sawyercoding_decode_sc4(buffer, decodedBuffer, length);
	if (decodedLength == sizeof(rct1_s4)) {
		memcpy(s4, decodedBuffer, sizeof(rct1_s4));
		success = true;
	} else {
		success = false;
	}

	free(buffer);
	free(decodedBuffer);
	return success;
}

bool rct1_read_sv4(const char *path, rct1_s4 *s4)
{
	uint8 *buffer, *decodedBuffer;
	long length, decodedLength;
	bool success;

	if (!readentirefile(path, (void**)&buffer, (int*)&length)) {
		RCT2_GLOBAL(RCT2_ADDRESS_ERROR_TYPE, uint8) = 255;
		RCT2_GLOBAL(RCT2_ADDRESS_ERROR_STRING_ID, uint16) = 3011;
		return 0;
	}

	decodedBuffer = malloc(sizeof(rct1_s4));
	decodedLength = sawyercoding_decode_sv4(buffer, decodedBuffer, length);
	if (decodedLength == sizeof(rct1_s4)) {
		memcpy(s4, decodedBuffer, sizeof(rct1_s4));
		success = true;
	} else {
		success = false;
	}

	free(buffer);
	free(decodedBuffer);
	return success;
}

/**
 *
 *  rct2: 0x0069EEA0
 */
void rct1_import_s4(rct1_s4 *s4)
{
	int i;
	rct_banner *banner;

	memcpy((void*)RCT2_ADDRESS_CURRENT_MONTH_YEAR, &s4->month, 16);
	memset((void*)RCT2_ADDRESS_MAP_ELEMENTS, 0, 0x30000 * sizeof(rct_map_element));
	memcpy((void*)RCT2_ADDRESS_MAP_ELEMENTS, s4->map_elements, sizeof(s4->map_elements));
	memcpy((void*)0x010E63B8, &s4->unk_counter, 4 + sizeof(s4->sprites));

	for (i = 0; i < MAX_BANNERS; i++)
		gBanners[i].type = 255;

	memcpy((void*)0x013573BC, &s4->next_sprite_index, 12424);

	for (i = 0; i < MAX_BANNERS; i++) {
		banner = &gBanners[i];
		if (banner->type != 255 && banner->string_idx != 3458)
			banner->string_idx = 778;
	}

	memcpy((void*)0x0135A8F4, &s4->string_table, 0x2F51C);
	memset((void*)RCT2_ADDRESS_STAFF_MODE_ARRAY, 0, 204);
	memcpy((void*)0x0138B580, &s4->map_animations, 0x258F2);
	memcpy((void*)0x013C6A72, &s4->patrol_areas, sizeof(s4->patrol_areas));

	char *esi = (char*)0x13C6A72;
	char *edi = (char*)RCT2_ADDRESS_STAFF_PATROL_AREAS;
	int ebx, edx = 116;
	do {
		ebx = 32;
		do {
			memcpy(edi, esi, 4); esi += 4; edi += 4;
			memset(edi, 0, 4); edi += 4;
		} while (--ebx > 0);
		memset(edi, 0, 64); edi += 64;
	} while (--edx > 0);
	edi += 0xA800;

	edx = 4;
	do {
		ebx = 32;
		do {
			memcpy(edi, esi, 4); esi += 4; edi += 4;
			memset(edi, 0, 4); edi += 4;
		} while (--ebx);
		memset(edi, 0, 64); edi += 64;
	} while (--edx);

	memcpy((void*)RCT2_ADDRESS_STAFF_MODE_ARRAY, &s4->unk_1F42AA, 116);
	memcpy((void*)0x013CA73A, &s4->unk_1F431E, 4);
	memcpy((void*)0x013CA73E, &s4->unk_1F4322, 0x41EA);
}

/**
 *
 *  rct2: 0x006A2B62
 */
void rct1_fix_landscape()
{
	int i;
	rct_sprite *sprite;
	rct_ride *ride;

	rct1_clear_extra_sprite_entries();

	// Free sprite user strings
	for (i = 0; i < MAX_SPRITES; i++) {
		sprite = &g_sprite_list[i];
		if (sprite->unknown.sprite_identifier != 255)
			user_string_free(sprite->unknown.name_string_idx);
	}

	reset_sprite_list();

	// Free ride user strings
	FOR_ALL_RIDES(i, ride)
		user_string_free(ride->name);

	ride_init_all();
	RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_IN_PARK, uint16) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_HEADING_FOR_PARK, uint16) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_LAST_GUESTS_IN_PARK, uint16) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_GUEST_CHANGE_MODIFIER, uint8) = 0;
	rct1_clear_extra_tile_entries();
	rct1_process_scenario_flags();
	rct1_fix_colours();
	rct1_fix_z();
	rct1_fix_paths();
	rct1_remove_rides();
	object_unload_all();
	rct1_load_default_objects();
	reset_loaded_objects();
	rct1_fix_walls();
	rct1_fix_scenery();
	rct1_fix_terrain();
	rct1_fix_entrance_positions();
	rct1_reset_research();
	research_populate_list_random();
	research_remove_non_separate_vehicle_types();

	climate_reset(RCT2_GLOBAL(RCT2_ADDRESS_CLIMATE, uint8));
	RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) = SCREEN_FLAGS_SCENARIO_EDITOR;
	viewport_init_all();
	news_item_init_queue();
	window_editor_main_open();

	rct_s6_header *s6Header = (rct_s6_header*)0x009E34E4;
	rct_s6_info *s6Info = (rct_s6_info*)0x0141F570;

	s6Info->editor_step = EDITOR_STEP_LANDSCAPE_EDITOR;
	s6Info->category = 4;
	format_string(s6Info->details, STR_NO_DETAILS_YET, NULL);
	s6Info->name[0] = 0;
	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_NO_MONEY) {
		RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) |= PARK_FLAGS_NO_MONEY_SCENARIO;
	} else {
		RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) &= ~PARK_FLAGS_NO_MONEY_SCENARIO;
	}
	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_ENTRANCE_FEE, money16) == MONEY_FREE) {
		RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) |= PARK_FLAGS_PARK_FREE_ENTRY;
	} else {
		RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) &= ~PARK_FLAGS_PARK_FREE_ENTRY;
	}
	RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) &= ~PARK_FLAGS_18;
	RCT2_GLOBAL(RCT2_ADDRESS_GUEST_INITIAL_CASH, money16) = clamp(
		MONEY(10,00),
		RCT2_GLOBAL(RCT2_ADDRESS_GUEST_INITIAL_CASH, money16),
		MONEY(100,00)
	);
	RCT2_GLOBAL(RCT2_ADDRESS_INITIAL_CASH, money32) = min(
		MONEY(10000,00),
		RCT2_GLOBAL(RCT2_ADDRESS_INITIAL_CASH, money32)
	);
	finance_reset_cash_to_initial();
	finance_update_loan_hash();

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32) = clamp(
		MONEY(0,00),
		RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32),
		MONEY(5000000,00)
	);

	RCT2_GLOBAL(RCT2_ADDRESS_MAXIMUM_LOAN, money32) = clamp(
		MONEY(0,00),
		RCT2_GLOBAL(RCT2_ADDRESS_MAXIMUM_LOAN, money32),
		MONEY(5000000,00)
	);

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_INTEREST_RATE, uint8) = clamp(
		5,
		RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_INTEREST_RATE, uint8),
		80
	);

	if (
		RCT2_GLOBAL(RCT2_ADDRESS_OBJECTIVE_TYPE, uint8) == OBJECTIVE_NONE ||
		RCT2_GLOBAL(RCT2_ADDRESS_OBJECTIVE_TYPE, uint8) == OBJECTIVE_HAVE_FUN ||
		RCT2_GLOBAL(RCT2_ADDRESS_OBJECTIVE_TYPE, uint8) == OBJECTIVE_BUILD_THE_BEST
	) {
		RCT2_GLOBAL(RCT2_ADDRESS_OBJECTIVE_TYPE, uint8) = OBJECTIVE_GUESTS_BY;
		RCT2_GLOBAL(RCT2_ADDRESS_OBJECTIVE_YEAR, uint8) = 4;
		RCT2_GLOBAL(RCT2_ADDRESS_OBJECTIVE_NUM_GUESTS, uint16) = 1000;
	}

	RCT2_GLOBAL(0x01358774, uint16) = 0;
}

static void rct1_remove_rides()
{
	map_element_iterator it;

	map_element_iterator_begin(&it);
	do {
		switch (map_element_get_type(it.element)) {
		case MAP_ELEMENT_TYPE_PATH:
			if (it.element->type & 1) {
				it.element->properties.path.type &= 0xF7;
				it.element->properties.path.ride_index = 255;
			}
			break;

		case MAP_ELEMENT_TYPE_TRACK:
			sub_6A7594();
			footpath_remove_edges_at(it.x * 32, it.y * 32, it.element);
			map_element_remove(it.element);
			map_element_iterator_restart_for_tile(&it);
			break;

		case MAP_ELEMENT_TYPE_ENTRANCE:
			if (it.element->properties.entrance.type != ENTRANCE_TYPE_PARK_ENTRANCE) {
				sub_6A7594();
				footpath_remove_edges_at(it.x * 32, it.y * 32, it.element);
				map_element_remove(it.element);
				map_element_iterator_restart_for_tile(&it);
			}
			break;
		}
	} while (map_element_iterator_next(&it));
}

static bool is_object_name_blank(rct_object_entry *entry)
{
	for (int i = 0; i < 8; i++) {
		if (entry->name[i] != ' ') {
			return false;
		}
	}
	return true;
}

/**
 *
 *  rct2: 0x0069F53D
 */
static void rct1_load_default_objects()
{
	for (int i = 0; i < 9; i++) {
		rct_object_entry *entries = (rct_object_entry*)RCT1DefaultObjects[i].entries;
		for (int j = 0; j < RCT1DefaultObjects[i].count; j++) {
			if (is_object_name_blank(&entries[j])) {
				continue;
			}

			if (!object_load(j, &entries[j], NULL)) {
				error_string_quit(0x99990000 + (i * 0x100) + j, -1);
				return;
			}
		}
	}

	// Water is a special case
	rct_object_entry *waterEntries = (rct_object_entry*)RCT1DefaultObjects[9].entries;
	rct_object_entry *waterEntry = &waterEntries[RCT2_GLOBAL(0x01358841, uint8) == 0 ? 0 : 1];
	if (!object_load(0, waterEntry, NULL)) {
		error_string_quit(0x99990900, -1);
		return;
	}
}

/**
 *
 *  rct2: 0x006A29B9
 */
static void rct1_fix_terrain()
{
	rct_map_element *element;
	map_element_iterator it;

	map_element_iterator_begin(&it);
	while (map_element_iterator_next(&it)) {
		element = it.element;

		if (map_element_get_type(element) != MAP_ELEMENT_TYPE_SURFACE)
			continue;

		// Convert terrain
		map_element_set_terrain(element, RCT1TerrainConvertTable[map_element_get_terrain(element)]);
		map_element_set_terrain_edge(element, RCT1TerrainEdgeConvertTable[map_element_get_terrain_edge(element)]);
	}
}

/**
 *
 *  rct2: 0x006A2956
 */
static void rct1_fix_scenery()
{
	rct_map_element *element;
	map_element_iterator it;

	map_element_iterator_begin(&it);
	while (map_element_iterator_next(&it)) {
		element = it.element;

		if (map_element_get_type(element) != MAP_ELEMENT_TYPE_SCENERY)
			continue;

		switch (element->properties.scenery.type) {
		case 157:	// TGE1	(Geometric Sculpture)
		case 162:	// TGE2	(Geometric Sculpture)
		case 168:	// TGE3	(Geometric Sculpture)
		case 170:	// TGE4	(Geometric Sculpture)
		case 171:	// TGE5	(Geometric Sculpture)
			element->properties.scenery.colour_2 = 2;
			break;
		}
	}
}

/**
 * This isn't really RCT1 specific anymore.
 *  rct2: 0x006A2A68
 */
static void rct1_fix_entrance_positions()
{
	rct_map_element *element;
	map_element_iterator it;

	for (int i = 0; i < 4; i++)
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_X, uint16)[i] = 0x8000;

	int entranceIndex = 0;

	map_element_iterator_begin(&it);
	while (map_element_iterator_next(&it)) {
		element = it.element;

		if (map_element_get_type(element) != MAP_ELEMENT_TYPE_ENTRANCE)
			continue;
		if (element->properties.entrance.type != ENTRANCE_TYPE_PARK_ENTRANCE)
			continue;
		if ((element->properties.entrance.index & 0x0F) != 0)
			continue;

		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_X, uint16)[entranceIndex] = it.x * 32;
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_Y, uint16)[entranceIndex] = it.y * 32;
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_Z, uint16)[entranceIndex] = element->base_height * 8;
		RCT2_ADDRESS(RCT2_ADDRESS_PARK_ENTRANCE_DIRECTION, uint8)[entranceIndex] = element->type & 3;
		entranceIndex++;

		// Prevent overflow
		if (entranceIndex == 4)
			return;
	}
}

/**
 *
 *  rct2: 0x0069F509
 */
static void rct1_reset_research()
{
	rct_research_item *researchItem;

	researchItem = gResearchItems;
	researchItem->entryIndex = RESEARCHED_ITEMS_SEPARATOR;
	researchItem++;
	researchItem->entryIndex = RESEARCHED_ITEMS_END;
	researchItem++;
	researchItem->entryIndex = RESEARCHED_ITEMS_END_2;
	RCT2_GLOBAL(RCT2_ADDRESS_RESEARH_PROGRESS_STAGE, uint8) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_LAST_RESEARCHED_ITEM_SUBJECT, sint32) = -1;
	news_item_init_queue();
}

/**
 *
 *  rct2: 0x0069F06A
 */
static void rct1_process_scenario_flags()
{
	uint32 scenarioFlags = RCT2_GLOBAL(0x013CE770, uint32);

	if (!(scenarioFlags & RCT1_SCENARIO_FLAG_ENABLE_BANNERS)) {
		banner_init();
	}
	if (!(scenarioFlags & (1 << 6))) {
		sub_69E891();
	}
	if (!(scenarioFlags & RCT1_SCENARIO_FLAG_CUSTOM_PARK_ENTRANCE_PATH)) {
		rct1_reset_park_entrance_path_type();
	}
	if (!(scenarioFlags & RCT1_SCENARIO_FLAG_NO_CASH_RESET)) {
		finance_reset_cash_to_initial();
	}
	if (!(scenarioFlags & RCT1_SCENARIO_FLAG_CUSTOM_MAP_SIZE)) {
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) = 127 * 32;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_MINUS_2, uint16) = 4350;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE, uint16) = 128;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_MAX_XY, uint16) = 4095;
	}
	if (!(scenarioFlags & (1 << 15))) {
		RCT2_GLOBAL(0x01358838, uint32) = 0;
	}
}

/**
 *
 *  rct2: 0x00666DFD
 */
static void rct1_reset_park_entrance_path_type()
{
	int x, y;
	rct_map_element *mapElement;

	x = RCT2_GLOBAL(0x013573EA, uint16);
	y = RCT2_GLOBAL(0x013573EC, uint16);
	if (x == (sint16)0x8000)
		return;

	mapElement = map_get_first_element_at(x >> 5, y >> 5);
	do {
		if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_ENTRANCE) {
			if (mapElement->properties.entrance.type == ENTRANCE_TYPE_PARK_ENTRANCE) {
				mapElement->properties.entrance.path_type = 0;
				break;
			}
		}
	} while (!map_element_is_last_for_tile(mapElement++));
}

/**
 *
 *  rct2: 0x0069F007
 */
static void rct1_clear_extra_sprite_entries()
{
	rct_unk_sprite *sprite;

	for (int i = 5000; i < MAX_SPRITES; i++) {
		sprite = &(g_sprite_list[i].unknown);

		memset(&g_sprite_list[i], 0, sizeof(rct_sprite));

		sprite->sprite_identifier = 255;
		sprite->sprite_index = i;
		sprite->linked_list_type_offset = SPRITE_LINKEDLIST_OFFSET_NULL;
		sprite->previous = SPRITE_INDEX_NULL;
		sprite->next = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_NEXT_INDEX, uint16);
		RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_NEXT_INDEX, uint16) = i;

		sprite = &(g_sprite_list[sprite->next].unknown);
		sprite->previous = i;
	}
	RCT2_GLOBAL(0x013573C8, uint16) += 5000;
}

/**
 *
 *  rct2: 0x0069F44B
 */
static void rct1_clear_extra_tile_entries()
{
	// Reset the map tile pointers
	for (int i = 0; i < 0x10000; i++) {
		gMapElementTilePointers[i] = (rct_map_element*)-1;
	}

	// Get the first free map element
	rct_map_element *nextFreeMapElement = gMapElements;
	for (int i = 0; i < 128 * 128; i++) {
		do {

		} while (!map_element_is_last_for_tile(nextFreeMapElement++));
	}

	rct_map_element *mapElement = gMapElements;
	rct_map_element **tilePointer = gMapElementTilePointers;

	// 128 rows of map data from RCT1 map
	for (int x = 0; x < 128; x++) {
		// Assign the first half of this row
		for (int y = 0; y < 128; y++) {
			*tilePointer++ = mapElement;
			do {

			} while (!map_element_is_last_for_tile(mapElement++));
		}

		// Fill the rest of the row with blank tiles
		for (int y = 0; y < 128; y++) {
			nextFreeMapElement->type = MAP_ELEMENT_TYPE_SURFACE;
			nextFreeMapElement->flags = MAP_ELEMENT_FLAG_LAST_TILE;
			nextFreeMapElement->base_height = 2;
			nextFreeMapElement->clearance_height = 0;
			nextFreeMapElement->properties.surface.slope = 0;
			nextFreeMapElement->properties.surface.terrain = 0;
			nextFreeMapElement->properties.surface.grass_length = GRASS_LENGTH_CLEAR_0;
			nextFreeMapElement->properties.surface.ownership = 0;
			*tilePointer++ = nextFreeMapElement++;
		}
	}

	// 128 extra rows left to fill with blank tiles
	for (int y = 0; y < 128 * 256; y++) {
		nextFreeMapElement->type = MAP_ELEMENT_TYPE_SURFACE;
		nextFreeMapElement->flags = MAP_ELEMENT_FLAG_LAST_TILE;
		nextFreeMapElement->base_height = 2;
		nextFreeMapElement->clearance_height = 0;
		nextFreeMapElement->properties.surface.slope = 0;
		nextFreeMapElement->properties.surface.terrain = 0;
		nextFreeMapElement->properties.surface.grass_length = GRASS_LENGTH_CLEAR_0;
		nextFreeMapElement->properties.surface.ownership = 0;
		*tilePointer++ = nextFreeMapElement++;
	}

	RCT2_GLOBAL(0x0140E9A4, rct_map_element*) = nextFreeMapElement;
}

/**
 *
 *  rct2: 0x0069F143
 */
static void rct1_fix_colours()
{
	int rideIndex, colour;
	rct_ride *ride;
	rct_unk_sprite *sprite;
	rct_peep *peep;
	rct_balloon *balloon;
	rct_map_element *mapElement;

	FOR_ALL_RIDES(rideIndex, ride) {
		for (int i = 0; i < 4; i++) {
			ride->track_colour_main[i] = RCT1ColourConversionTable[ride->track_colour_main[i]];
			ride->track_colour_additional[i] = RCT1ColourConversionTable[ride->track_colour_additional[i]];
			ride->track_colour_supports[i] = RCT1ColourConversionTable[ride->track_colour_supports[i]];
		}

		for (int i = 0; i < 32; i++) {
			ride->vehicle_colours[i].body_colour = RCT1ColourConversionTable[ride->vehicle_colours[i].body_colour];
			ride->vehicle_colours[i].trim_colour = RCT1ColourConversionTable[ride->vehicle_colours[i].trim_colour];
		}
	}

	for (int i = 0; i < MAX_SPRITES; i++) {
		sprite = &(g_sprite_list[i].unknown);
		switch (sprite->sprite_identifier) {
		case SPRITE_IDENTIFIER_PEEP:
			peep = (rct_peep*)sprite;
			peep->tshirt_colour = RCT1ColourConversionTable[peep->tshirt_colour];
			peep->trousers_colour = RCT1ColourConversionTable[peep->trousers_colour];
			peep->balloon_colour = RCT1ColourConversionTable[peep->balloon_colour];
			peep->umbrella_colour = RCT1ColourConversionTable[peep->umbrella_colour];
			peep->hat_colour = RCT1ColourConversionTable[peep->hat_colour];
			break;
		case SPRITE_IDENTIFIER_MISC:
			balloon = (rct_balloon*)sprite;
			balloon->colour = RCT1ColourConversionTable[balloon->colour];
			balloon->var_2D = RCT1ColourConversionTable[balloon->var_2D];
			break;
		}
	}

	mapElement = gMapElements;
	while (mapElement < RCT2_GLOBAL(0x0140E9A4, rct_map_element*)) {
		if (mapElement->base_height != 255) {
			switch (map_element_get_type(mapElement)) {
			case MAP_ELEMENT_TYPE_SCENERY:
				colour = RCT1ColourConversionTable[mapElement->properties.scenery.colour_1 & 0x1F];
				mapElement->properties.scenery.colour_1 &= 0xE0;
				mapElement->properties.scenery.colour_1 |= colour;
				break;
			case MAP_ELEMENT_TYPE_FENCE:
				colour = RCT1ColourConversionTable[
					((mapElement->type & 0xC0) >> 3) |
					((mapElement->properties.fence.type & 0xE0) >> 5)
				];

				mapElement->type &= 0x3F;
				mapElement->properties.fence.type &= 0x1F;
				mapElement->type |= (colour & 0x18) << 3;
				mapElement->properties.fence.type |= (colour & 7) << 5;
				break;
			case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
				colour = RCT1ColourConversionTable[mapElement->properties.scenerymultiple.colour[0] & 0x1F];
				mapElement->properties.scenerymultiple.colour[0] &= 0xE0;
				mapElement->properties.scenerymultiple.colour[0] |= colour;

				colour = RCT1ColourConversionTable[mapElement->properties.scenerymultiple.colour[1] & 0x1F];
				mapElement->properties.scenerymultiple.colour[1] &= 0xE0;
				mapElement->properties.scenerymultiple.colour[1] |= colour;
				break;
			}
		}
		mapElement++;
	}
}

/**
 *
 *  rct2: 0x0069F2D0
 */
static void rct1_fix_z()
{
	int i;
	rct_ride *ride;
	rct_unk_sprite *sprite;
	rct_peep *peep;
	rct_ride_measurement *rideMeasurement;
	rct_map_element *mapElement;

	FOR_ALL_RIDES(i, ride) {
		for (int i = 0; i < 4; i++) {
			ride->station_heights[i] /= 2;
		}
		ride->var_116 /= 2;
		ride->highest_drop_height = 1;
		if (ride->var_11F != 255) {
			ride->var_11F /= 2;
		}
		ride->var_13E /= 2;
		ride->var_13F /= 2;
	}

	for (int i = 0; i < RCT2_GLOBAL(0x0138B580, uint16); i++) {
		gAnimatedObjects[i].baseZ /= 2;
	}

	for (int i = 0; i < MAX_SPRITES; i++) {
		sprite = &(g_sprite_list[i].unknown);
		if (sprite->sprite_identifier == SPRITE_IDENTIFIER_PEEP) {
			peep = (rct_peep*)sprite;
			peep->next_z /= 2;
			RCT2_GLOBAL((int)peep + 0xCE, uint8) /= 2;
		}
	}

	for (int i = 0; i < MAX_RIDE_MEASUREMENTS; i++) {
		rideMeasurement = GET_RIDE_MEASUREMENT(i);
		if (rideMeasurement->ride_index == 255)
			continue;

		for (int i = 0; i < RIDE_MEASUREMENT_MAX_ITEMS; i++) {
			rideMeasurement->altitude[i] /= 2;
		}
	}

	mapElement = gMapElements;
	while (mapElement < RCT2_GLOBAL(0x0140E9A4, rct_map_element*)) {
		if (mapElement->base_height != 255) {
			mapElement->base_height /= 2;
			mapElement->clearance_height /= 2;
		}
		mapElement++;
	}
	RCT2_GLOBAL(0x01359208, uint16) = 7;
}

/**
 *
 *  rct2: 0x0069F3AB
 */
static void rct1_fix_paths()
{
	rct_map_element *mapElement;
	int pathType, secondaryType, additions;

	mapElement = gMapElements;
	while (mapElement < RCT2_GLOBAL(0x0140E9A4, rct_map_element*)) {
		switch (map_element_get_type(mapElement)) {
		case MAP_ELEMENT_TYPE_PATH:
			// Type
			pathType = ((mapElement->properties.path.type & 0xF0) >> 2) | (mapElement->type & 3);
			secondaryType = (mapElement->flags & 0x60) >> 5;
			pathType = RCT1PathTypeConversionTable[pathType * 4 + secondaryType];

			mapElement->type &= 0xFC;
			mapElement->flags &= ~0x60;
			mapElement->properties.path.type &= 0x0F;
			mapElement->properties.path.additions &= 0x7F;
			if (pathType & 0x80) {
				mapElement->type |= 1;
			}
			mapElement->properties.path.type |= pathType << 4;

			// Additions
			additions = mapElement->properties.path.additions & 0x0F;
			additions = RCT1PathAdditionConversionTable[additions];
			if (additions & 0x80) {
				additions &= ~0x80;
				mapElement->flags |= MAP_ELEMENT_FLAG_BROKEN;
			} else {
				mapElement->flags &= ~MAP_ELEMENT_FLAG_BROKEN;
			}
			mapElement->properties.path.additions &= 0xF0;
			mapElement->properties.path.additions |= additions;
			break;
		case MAP_ELEMENT_TYPE_ENTRANCE:
			if (mapElement->properties.entrance.type == ENTRANCE_TYPE_PARK_ENTRANCE) {
				pathType = mapElement->properties.entrance.path_type;
				mapElement->properties.entrance.path_type = RCT1PathTypeConversionTable[pathType * 4] & 0x7F;
			}
			break;
		}
		mapElement++;
	}
}

/**
 *
 *  rct2: 0x006A28F5
 */
static void rct1_convert_wall(int *type, int *colourA, int *colourB, int *colourC)
{
	switch (*type) {
	case 12:	// creepy gate
		*colourA = 24;
		break;
	case 26:	// medium brown castle wall
		*type = 12;
		*colourA = 25;
		break;
	case 27:	// tall castle wall with grey window
		*type = 12;
		*colourA = 2;
	case 50:	// plate glass
		*colourA = 24;
		break;
	case 13:
		*colourB = *colourA;
		*colourA = 24;
		break;
	case 11:	// tall castle wall with grey gate
	case 22:	// brick wall with gate
		*colourB = 2;
		break;
	case 35:	// wood post fence
	case 42:	// tall grey castle wall
	case 43:	// wooden fence with snow
	case 44:
	case 45:
	case 46:
		*colourA = 1;
		break;
	}
}

/**
 *
 *  rct2: 0x006A2730
 */
static void rct1_fix_walls()
{
	rct_map_element *mapElement, originalMapElement;

	for (int x = 0; x < 128; x++) {
		for (int y = 0; y < 128; y++) {
			mapElement = map_get_first_element_at(x, y);
			do {
				if (map_element_get_type(mapElement) == MAP_ELEMENT_TYPE_FENCE) {
					originalMapElement = *mapElement;
					map_element_remove(mapElement);

					uint8 var_05 = originalMapElement.properties.fence.item[0];
					uint16 var_06 = (
						originalMapElement.properties.fence.item[1] |
						(originalMapElement.properties.fence.item[2] << 8)
					);

					for (int edge = 0; edge < 4; edge++) {
						int typeA = (var_05 >> (edge * 2)) & 3;
						int typeB = (var_06 >> (edge * 4)) & 0x0F;
						if (typeB != 0x0F) {
							int type = typeA | (typeB << 2);
							int colourA = (
								((originalMapElement.type & 0xC0) >> 3) |
								(originalMapElement.properties.fence.type >> 5)
							);
							int colourB = 0;
							int colourC = 0;
							rct1_convert_wall(&type, &colourA, &colourB, &colourC);
							map_place_fence(type, x * 32, y * 32, 0, edge, colourA, colourB, colourC, 169);
						}
					}
					break;
				}
			} while (!map_element_is_last_for_tile(mapElement++));
		}
	}
}

/**
 *
 *  rct2: 0x0069E891
 */
static void sub_69E891()
{
	RCT2_GLOBAL(0x013587D8, uint16) = 63;
}

#pragma region Tables

// rct2: 0x0097F0BC & 0x0098BC60
const uint8 RCT1ColourConversionTable[32] = {
	 0,  1,  2,  4,  5,  6,  7,  9,
	11, 12, 13, 14, 15, 16, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27,
	28, 30, 31, 29,  3, 10, 17,  8
};

static const uint8 RCT1TerrainConvertTable[16] = {
	TERRAIN_GRASS,
	TERRAIN_SAND,
	TERRAIN_DIRT,
	TERRAIN_ROCK,
	TERRAIN_MARTIAN,
	TERRAIN_CHECKERBOARD,
	TERRAIN_GRASS_CLUMPS,
	TERRAIN_DIRT,				// Originally TERRAIN_ROOF_BROWN
	TERRAIN_ICE,
	TERRAIN_DIRT,				// Originally TERRAIN_ROOF_LOG
	TERRAIN_DIRT,				// Originally TERRAIN_ROOF_IRON
	TERRAIN_DIRT,				// Originally TERRAIN_ROOF_GREY
	TERRAIN_GRID_RED,
	TERRAIN_GRID_YELLOW,
	TERRAIN_GRID_BLUE,
	TERRAIN_GRID_GREEN
};

static const uint8 RCT1TerrainEdgeConvertTable[16] = {
	TERRAIN_EDGE_ROCK,
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_BRICK
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_IRON
	TERRAIN_EDGE_WOOD_RED,
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_GREY
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_YELLOW
	TERRAIN_EDGE_WOOD_BLACK,
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_RED
	TERRAIN_EDGE_ICE,
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_PURPLE
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_GREEN
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_STONE_BROWN
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_STONE_GREY
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_SKYSCRAPER_A
	TERRAIN_EDGE_ROCK,			// Originally TERRAIN_EDGE_SKYSCRAPER_B
	TERRAIN_EDGE_ROCK			// Unused
};

// rct2: 0x0098BC9F
static const uint8 RCT1PathTypeConversionTable[96] = {
	0x80 | 0, 0x80 | 1, 0x80 | 2, 0x80 | 3,
	0x80 | 0, 0x80 | 1, 0x80 | 2, 0x80 | 3,
	0x80 | 0, 0x80 | 1, 0x80 | 2, 0x80 | 3,
	0x80 | 0, 0x80 | 1, 0x80 | 2, 0x80 | 3,
	0, 0, 0, 0,
	2, 2, 2, 2,
	1, 1, 1, 1,
	0, 0, 0, 0,
	3, 3, 3, 3,
	6, 6, 6, 6,
	0, 0, 0, 0,
	0, 0, 0, 0,
	5, 5, 5, 5,
	5, 5, 5, 5,
	5, 5, 5, 5,
	5, 5, 5, 5,
	4, 4, 4, 4,
	4, 4, 4, 4,
	4, 4, 4, 4,
	4, 4, 4, 4,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
};

// rct2: 0x0098BCFF
static const uint8 RCT1PathAdditionConversionTable[15] = {
	0,
	1, 2, 3, 4, 5, 6, 7,
	0x80 | 1, 0x80 | 2, 0x80 | 3, 0x80 | 4, 0x80 | 6, 0x80 | 7,
	8,
};

#pragma endregion

#pragma region RCT1 Default Objects

static const rct_object_entry RCT1DefaultObjectsRides[] = {
	{ 0x00008000, { "PTCT1   " }, 0 },
	{ 0x00008000, { "TOGST   " }, 0 },
	{ 0x00008000, { "ARRSW1  " }, 0 },
	{ 0x00008000, { "NEMT    " }, 0 },
	{ 0x00008000, { "ZLDB    " }, 0 },
	{ 0x00008000, { "NRL     " }, 0 },
	{ 0x00008000, { "MONO2   " }, 0 },
	{ 0x00008000, { "BATFL   " }, 0 },
	{ 0x00008000, { "RBOAT   " }, 0 },
	{ 0x00008000, { "WMOUSE  " }, 0 },
	{ 0x00008000, { "STEEP1  " }, 0 },
	{ 0x00008000, { "SPCAR   " }, 0 },
	{ 0x00008000, { "SSC1    " }, 0 },
	{ 0x00008000, { "BOB1    " }, 0 },
	{ 0x00008000, { "OBS1    " }, 0 },
	{ 0x00008000, { "SCHT1   " }, 0 },
	{ 0x00008000, { "DING1   " }, 0 },
	{ 0x00008000, { "AMT1    " }, 0 },
	{ 0x00008000, { "CLIFT1  " }, 0 },
	{ 0x00008000, { "ARRT1   " }, 0 },
	{ 0x00008000, { "HMAZE   " }, 0 },
	{ 0x00008000, { "HSKELT  " }, 0 },
	{ 0x00008000, { "KART1   " }, 0 },
	{ 0x00008000, { "LFB1    " }, 0 },
	{ 0x00008000, { "RAPBOAT " }, 0 },
	{ 0x00008000, { "DODG1   " }, 0 },
	{ 0x00008000, { "SWSH1   " }, 0 },
	{ 0x00008000, { "SWSH2   " }, 0 },
	{ 0x00008000, { "ICECR1  " }, 0 },
	{ 0x00008000, { "CHPSH   " }, 0 },
	{ 0x00008000, { "DRNKS   " }, 0 },
	{ 0x00008000, { "CNDYF   " }, 0 },
	{ 0x00008000, { "BURGB   " }, 0 },
	{ 0x00008000, { "MGR1    " }, 0 },
	{ 0x00008000, { "BALLN   " }, 0 },
	{ 0x00008000, { "INFOK   " }, 0 },
	{ 0x00008000, { "TLT1    " }, 0 },
	{ 0x00008000, { "FWH1    " }, 0 },
	{ 0x00008000, { "SIMPOD  " }, 0 },
	{ 0x00008000, { "C3D     " }, 0 },
	{ 0x00008000, { "TOPSP1  " }, 0 },
	{ 0x00008000, { "SRINGS  " }, 0 },
	{ 0x00008000, { "REVF1   " }, 0 },
	{ 0x00008000, { "SOUVS   " }, 0 },
	{ 0x00008000, { "BMVD    " }, 0 },
	{ 0x00008000, { "PIZZS   " }, 0 },
	{ 0x00008000, { "TWIST1  " }, 0 },
	{ 0x00008000, { "HHBUILD " }, 0 },
	{ 0x00008000, { "POPCS   " }, 0 },
	{ 0x00008000, { "CIRCUS1 " }, 0 },
	{ 0x00008000, { "GTC     " }, 0 },
	{ 0x00008000, { "BMSD    " }, 0 },
	{ 0x00008000, { "PTCT1   " }, 0 },
	{ 0x00008000, { "SFRIC1  " }, 0 },
	{ 0x00008000, { "SMC1    " }, 0 },
	{ 0x00008000, { "HOTDS   " }, 0 },
	{ 0x00008000, { "SQDST   " }, 0 },
	{ 0x00008000, { "HATST   " }, 0 },
	{ 0x00008000, { "TOFFS   " }, 0 },
	{ 0x00008000, { "VREEL   " }, 0 },
	{ 0x00008000, { "SPBOAT  " }, 0 },
	{ 0x00008000, { "MONBK   " }, 0 },
	{ 0x00008000, { "BMAIR   " }, 0 },
	{ 0x00008000, { "SMONO   " }, 0 },
	{ 0x00000000, { "        " }, 0 },
	{ 0x00008000, { "REVCAR  " }, 0 },
	{ 0x00008000, { "UTCAR   " }, 0 },
	{ 0x00008000, { "GOLF1   " }, 0 },
	{ 0x00000000, { "        " }, 0 },
	{ 0x00008000, { "GDROP1  " }, 0 },
	{ 0x00008000, { "FSAUC   " }, 0 },
	{ 0x00008000, { "CHBUILD " }, 0 },
	{ 0x00008000, { "HELICAR " }, 0 },
	{ 0x00008000, { "SLCT    " }, 0 },
	{ 0x00008000, { "CSTBOAT " }, 0 },
	{ 0x00008000, { "THCAR   " }, 0 },
	{ 0x00008000, { "IVMC1   " }, 0 },
	{ 0x00008000, { "JSKI    " }, 0 },
	{ 0x00008000, { "TSHRT   " }, 0 },
	{ 0x00008000, { "RFTBOAT " }, 0 },
	{ 0x00008000, { "DOUGH   " }, 0 },
	{ 0x00008000, { "ENTERP  " }, 0 },
	{ 0x00008000, { "COFFS   " }, 0 },
	{ 0x00008000, { "CHCKS   " }, 0 },
	{ 0x00008000, { "LEMST   " }, 0 }
};

// rct2: 0x0098BD0E
static const rct_object_entry RCT1DefaultObjectsSmallScenery[] = {
	{ 0x00000081, { "TL0     " }, 0 },
	{ 0x00000081, { "TL1     " }, 0 },
	{ 0x00000081, { "TL2     " }, 0 },
	{ 0x00000081, { "TL3     " }, 0 },
	{ 0x00000081, { "TM0     " }, 0 },
	{ 0x00000081, { "TM1     " }, 0 },
	{ 0x00000081, { "TM2     " }, 0 },
	{ 0x00000081, { "TM3     " }, 0 },
	{ 0x00000081, { "TS0     " }, 0 },
	{ 0x00000081, { "TS1     " }, 0 },
	{ 0x00000081, { "TS2     " }, 0 },
	{ 0x00000081, { "TS3     " }, 0 },
	{ 0x00000081, { "TS4     " }, 0 },
	{ 0x00000081, { "TS5     " }, 0 },
	{ 0x00000081, { "TS6     " }, 0 },
	{ 0x00000081, { "TIC     " }, 0 },
	{ 0x00000081, { "TLC     " }, 0 },
	{ 0x00000081, { "TMC     " }, 0 },
	{ 0x00000081, { "TMP     " }, 0 },
	{ 0x00000081, { "TITC    " }, 0 },
	{ 0x00000081, { "TGHC    " }, 0 },
	{ 0x00000081, { "TAC     " }, 0 },
	{ 0x00000081, { "TGHC2   " }, 0 },
	{ 0x00000081, { "TCJ     " }, 0 },
	{ 0x00000081, { "TMBJ    " }, 0 },
	{ 0x00000081, { "TCF     " }, 0 },
	{ 0x00000081, { "TCL     " }, 0 },
	{ 0x00000081, { "TRF     " }, 0 },
	{ 0x00000081, { "TRF2    " }, 0 },
	{ 0x00000081, { "TEL     " }, 0 },
	{ 0x00000081, { "TAP     " }, 0 },
	{ 0x00000081, { "TSP     " }, 0 },
	{ 0x00000081, { "TMZP    " }, 0 },
	{ 0x00000081, { "TCRP    " }, 0 },
	{ 0x00000081, { "TBP     " }, 0 },
	{ 0x00000081, { "TLP     " }, 0 },
	{ 0x00000081, { "TWP     " }, 0 },
	{ 0x00000081, { "TAS     " }, 0 },
	{ 0x00000081, { "TMG     " }, 0 },
	{ 0x00000081, { "TWW     " }, 0 },
	{ 0x00000081, { "TSB     " }, 0 },
	{ 0x00000081, { "TVL     " }, 0 },
	{ 0x00000081, { "TCT     " }, 0 },
	{ 0x00000081, { "TEF     " }, 0 },
	{ 0x00000081, { "TAL     " }, 0 },
	{ 0x00000081, { "TSQ     " }, 0 },
	{ 0x00000081, { "THT     " }, 0 },
	{ 0x00000081, { "TCB     " }, 0 },
	{ 0x00000081, { "TDM     " }, 0 },
	{ 0x00000081, { "TSD     " }, 0 },
	{ 0x00000081, { "TGS     " }, 0 },
	{ 0x00000081, { "TUS     " }, 0 },
	{ 0x00000081, { "TH1     " }, 0 },
	{ 0x00000081, { "TBC     " }, 0 },
	{ 0x00000081, { "TH2     " }, 0 },
	{ 0x00000081, { "TPM     " }, 0 },
	{ 0x00000081, { "TSC     " }, 0 },
	{ 0x00000081, { "TG1     " }, 0 },
	{ 0x00000081, { "TWF     " }, 0 },
	{ 0x00000081, { "TSH0    " }, 0 },
	{ 0x00000081, { "TSH1    " }, 0 },
	{ 0x00000081, { "TSH2    " }, 0 },
	{ 0x00000081, { "TSH3    " }, 0 },
	{ 0x00000081, { "TSH4    " }, 0 },
	{ 0x00000081, { "TSH5    " }, 0 },
	{ 0x00000081, { "TG2     " }, 0 },
	{ 0x00000081, { "TG3     " }, 0 },
	{ 0x00000081, { "TG4     " }, 0 },
	{ 0x00000081, { "TG5     " }, 0 },
	{ 0x00000081, { "TG6     " }, 0 },
	{ 0x00000081, { "TG7     " }, 0 },
	{ 0x00000081, { "TG8     " }, 0 },
	{ 0x00000081, { "TG9     " }, 0 },
	{ 0x00000081, { "TG10    " }, 0 },
	{ 0x00000081, { "TG11    " }, 0 },
	{ 0x00000081, { "TG12    " }, 0 },
	{ 0x00000081, { "TG13    " }, 0 },
	{ 0x00000081, { "TG14    " }, 0 },
	{ 0x00000081, { "TT1     " }, 0 },
	{ 0x00000081, { "TDF     " }, 0 },
	{ 0x00000081, { "TSH     " }, 0 },
	{ 0x00000081, { "THRS    " }, 0 },
	{ 0x00000081, { "TSTD    " }, 0 },
	{ 0x00000081, { "TRMS    " }, 0 },
	{ 0x00000081, { "TRWS    " }, 0 },
	{ 0x00000081, { "TRC     " }, 0 },
	{ 0x00000081, { "TQF     " }, 0 },
	{ 0x00000081, { "TES1    " }, 0 },
	{ 0x00000081, { "TEN     " }, 0 },
	{ 0x00000081, { "TERS    " }, 0 },
	{ 0x00000081, { "TERB    " }, 0 },
	{ 0x00000081, { "TEP     " }, 0 },
	{ 0x00000081, { "TST1    " }, 0 },
	{ 0x00000081, { "TST2    " }, 0 },
	{ 0x00000081, { "TMS1    " }, 0 },
	{ 0x00000081, { "TAS1    " }, 0 },
	{ 0x00000081, { "TAS2    " }, 0 },
	{ 0x00000081, { "TAS3    " }, 0 },
	{ 0x00000081, { "TST3    " }, 0 },
	{ 0x00000081, { "TST4    " }, 0 },
	{ 0x00000081, { "TST5    " }, 0 },
	{ 0x00000081, { "TAS4    " }, 0 },
	{ 0x00000081, { "TCY     " }, 0 },
	{ 0x00000081, { "TBW     " }, 0 },
	{ 0x00000081, { "TBR1    " }, 0 },
	{ 0x00000081, { "TBR2    " }, 0 },
	{ 0x00000081, { "TML     " }, 0 },
	{ 0x00000081, { "TMW     " }, 0 },
	{ 0x00000081, { "TBR3    " }, 0 },
	{ 0x00000081, { "TBR4    " }, 0 },
	{ 0x00000081, { "TMJ     " }, 0 },
	{ 0x00000081, { "TBR     " }, 0 },
	{ 0x00000081, { "TMO1    " }, 0 },
	{ 0x00000081, { "TMO2    " }, 0 },
	{ 0x00000081, { "TMO3    " }, 0 },
	{ 0x00000081, { "TMO4    " }, 0 },
	{ 0x00000081, { "TMO5    " }, 0 },
	{ 0x00000081, { "TWH1    " }, 0 },
	{ 0x00000081, { "TWH2    " }, 0 },
	{ 0x00000081, { "TNS     " }, 0 },
	{ 0x00000081, { "TP1     " }, 0 },
	{ 0x00000081, { "TP2     " }, 0 },
	{ 0x00000081, { "TK1     " }, 0 },
	{ 0x00000081, { "TK2     " }, 0 },
	{ 0x00000081, { "TR1     " }, 0 },
	{ 0x00000081, { "TR2     " }, 0 },
	{ 0x00000081, { "TQ1     " }, 0 },
	{ 0x00000081, { "TQ2     " }, 0 },
	{ 0x00000081, { "TWN     " }, 0 },
	{ 0x00000081, { "TCE     " }, 0 },
	{ 0x00000081, { "TCO     " }, 0 },
	{ 0x00000081, { "THL     " }, 0 },
	{ 0x00000081, { "TCC     " }, 0 },
	{ 0x00000081, { "TB1     " }, 0 },
	{ 0x00000081, { "TB2     " }, 0 },
	{ 0x00000081, { "TK3     " }, 0 },
	{ 0x00000081, { "TK4     " }, 0 },
	{ 0x00000081, { "TBN     " }, 0 },
	{ 0x00000081, { "TBN1    " }, 0 },
	{ 0x00000081, { "TDT1    " }, 0 },
	{ 0x00000081, { "TDT2    " }, 0 },
	{ 0x00000081, { "TDT3    " }, 0 },
	{ 0x00000081, { "TMM1    " }, 0 },
	{ 0x00000081, { "TMM2    " }, 0 },
	{ 0x00000081, { "TMM3    " }, 0 },
	{ 0x00000081, { "TGS1    " }, 0 },
	{ 0x00000081, { "TGS2    " }, 0 },
	{ 0x00000081, { "TGS3    " }, 0 },
	{ 0x00000081, { "TGS4    " }, 0 },
	{ 0x00000081, { "TDN4    " }, 0 },
	{ 0x00000081, { "TDN5    " }, 0 },
	{ 0x00000081, { "TJT1    " }, 0 },
	{ 0x00000081, { "TJT2    " }, 0 },
	{ 0x00000081, { "TJB1    " }, 0 },
	{ 0x00000081, { "TTF     " }, 0 },
	{ 0x00000081, { "TF1     " }, 0 },
	{ 0x00000081, { "TF2     " }, 0 },
	{ 0x00000081, { "TGE1    " }, 0 },
	{ 0x00000081, { "TJT3    " }, 0 },
	{ 0x00000081, { "TJT4    " }, 0 },
	{ 0x00000081, { "TJP1    " }, 0 },
	{ 0x00000081, { "TJB2    " }, 0 },
	{ 0x00000081, { "TGE2    " }, 0 },
	{ 0x00000081, { "TJT5    " }, 0 },
	{ 0x00000081, { "TJB3    " }, 0 },
	{ 0x00000081, { "TJB4    " }, 0 },
	{ 0x00000081, { "TJT6    " }, 0 },
	{ 0x00000081, { "TJP2    " }, 0 },
	{ 0x00000081, { "TGE3    " }, 0 },
	{ 0x00000081, { "TCK     " }, 0 },
	{ 0x00000081, { "TGE4    " }, 0 },
	{ 0x00000081, { "TGE5    " }, 0 },
	{ 0x00000081, { "TG15    " }, 0 },
	{ 0x00000081, { "TG16    " }, 0 },
	{ 0x00000081, { "TG17    " }, 0 },
	{ 0x00000081, { "TG18    " }, 0 },
	{ 0x00000081, { "TG19    " }, 0 },
	{ 0x00000081, { "TG20    " }, 0 },
	{ 0x00000081, { "TG21    " }, 0 },
	{ 0x00000081, { "TSM     " }, 0 },
	{ 0x00000081, { "TIG     " }, 0 },
	{ 0x00000081, { "TCFS    " }, 0 },
	{ 0x00000081, { "TRFS    " }, 0 },
	{ 0x00000081, { "TRF3    " }, 0 },
	{ 0x00000081, { "TNSS    " }, 0 },
	{ 0x00000081, { "TCT1    " }, 0 },
	{ 0x00000081, { "TCT2    " }, 0 },
	{ 0x00000081, { "TSF1    " }, 0 },
	{ 0x00000081, { "TSF2    " }, 0 },
	{ 0x00000081, { "TSF3    " }, 0 },
	{ 0x00000081, { "TCN     " }, 0 },
	{ 0x00000081, { "TTG     " }, 0 },
	{ 0x00000081, { "TSNC    " }, 0 },
	{ 0x00000081, { "TSNB    " }, 0 },
	{ 0x00000081, { "TSCP    " }, 0 },
	{ 0x00000081, { "TCD     " }, 0 },
	{ 0x00000081, { "TSG     " }, 0 },
	{ 0x00000081, { "TSK     " }, 0 },
	{ 0x00000081, { "TGH1    " }, 0 },
	{ 0x00000081, { "TGH2    " }, 0 },
	{ 0x00000081, { "TSMP    " }, 0 },
	{ 0x00000081, { "TJF     " }, 0 },
	{ 0x00000081, { "TLY     " }, 0 },
	{ 0x00000081, { "TGC1    " }, 0 },
	{ 0x00000081, { "TGC2    " }, 0 },
	{ 0x00000081, { "TGG     " }, 0 },
	{ 0x00000081, { "TSPH    " }, 0 },
	{ 0x00000081, { "TOH1    " }, 0 },
	{ 0x00000081, { "TOH2    " }, 0 },
	{ 0x00000081, { "TOT1    " }, 0 },
	{ 0x00000081, { "TOT2    " }, 0 },
	{ 0x00000081, { "TOS     " }, 0 },
	{ 0x00000081, { "TOT3    " }, 0 },
	{ 0x00000081, { "TOT4    " }, 0 },
	{ 0x00000081, { "TSC2    " }, 0 },
	{ 0x00000081, { "TSP1    " }, 0 },
	{ 0x00000081, { "TOH3    " }, 0 },
	{ 0x00000081, { "TSP2    " }, 0 },
	{ 0x00000081, { "ROMROOF1" }, 0 },
	{ 0x00000081, { "GEOROOF1" }, 0 },
	{ 0x00000081, { "TNTROOF1" }, 0 },
	{ 0x00000081, { "JNGROOF1" }, 0 },
	{ 0x00000081, { "MINROOF1" }, 0 },
	{ 0x00000081, { "ROMROOF2" }, 0 },
	{ 0x00000081, { "GEOROOF2" }, 0 },
	{ 0x00000081, { "PAGROOF1" }, 0 },
	{ 0x00000081, { "SPCROOF1" }, 0 },
	{ 0x00000081, { "ROOF1   " }, 0 },
	{ 0x00000081, { "ROOF2   " }, 0 },
	{ 0x00000081, { "ROOF3   " }, 0 },
	{ 0x00000081, { "ROOF4   " }, 0 },
	{ 0x00000081, { "ROOF5   " }, 0 },
	{ 0x00000081, { "ROOF6   " }, 0 },
	{ 0x00000081, { "ROOF7   " }, 0 },
	{ 0x00000081, { "ROOF8   " }, 0 },
	{ 0x00000081, { "ROOF9   " }, 0 },
	{ 0x00000081, { "ROOF10  " }, 0 },
	{ 0x00000081, { "ROOF11  " }, 0 },
	{ 0x00000081, { "ROOF12  " }, 0 },
	{ 0x00000081, { "ROOF13  " }, 0 },
	{ 0x00000081, { "ROOF14  " }, 0 },
	{ 0x00000081, { "IGROOF  " }, 0 },
	{ 0x00000081, { "CORROOF " }, 0 },
	{ 0x00000081, { "CORROOF2" }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsLargeScenery[] = {
	{ 0x00000082, { "SCOL    " }, 0 },
	{ 0x00000082, { "SHS1    " }, 0 },
	{ 0x00000082, { "SSPX    " }, 0 },
	{ 0x00000082, { "SHS2    " }, 0 },
	{ 0x00000082, { "SCLN    " }, 0 },
	{ 0x00000082, { "SMH1    " }, 0 },
	{ 0x00000082, { "SMH2    " }, 0 },
	{ 0x00000082, { "SVLC    " }, 0 },
	{ 0x00000082, { "SPYR    " }, 0 },
	{ 0x00000082, { "SMN1    " }, 0 },
	{ 0x00000082, { "SMB     " }, 0 },
	{ 0x00000082, { "SSK1    " }, 0 },
	{ 0x00000082, { "SDN1    " }, 0 },
	{ 0x00000082, { "SDN2    " }, 0 },
	{ 0x00000082, { "SDN3    " }, 0 },
	{ 0x00000082, { "SIP     " }, 0 },
	{ 0x00000082, { "STB1    " }, 0 },
	{ 0x00000082, { "STB2    " }, 0 },
	{ 0x00000082, { "STG1    " }, 0 },
	{ 0x00000082, { "STG2    " }, 0 },
	{ 0x00000082, { "SCT     " }, 0 },
	{ 0x00000082, { "SOH1    " }, 0 },
	{ 0x00000082, { "SOH2    " }, 0 },
	{ 0x00000082, { "SOH3    " }, 0 },
	{ 0x00000082, { "SGP     " }, 0 },
	{ 0x00000082, { "SSR     " }, 0 },
	{ 0x00000082, { "STH     " }, 0 },
	{ 0x00000082, { "SAH     " }, 0 },
	{ 0x00000082, { "SPS     " }, 0 },
	{ 0x00000082, { "SPG     " }, 0 },
	{ 0x00000082, { "SOB     " }, 0 },
	{ 0x00000082, { "SAH2    " }, 0 },
	{ 0x00000082, { "SST     " }, 0 },
	{ 0x00000082, { "SSH     " }, 0 },
	{ 0x00000082, { "SAH3    " }, 0 },
	{ 0x00000082, { "SSIG1   " }, 0 },
	{ 0x00000082, { "SSIG2   " }, 0 },
	{ 0x00000082, { "SSIG3   " }, 0 },
	{ 0x00000082, { "SSIG4   " }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsWall[] = {
	{ 0x00000083, { "WMF     " }, 0 },
	{ 0x00000083, { "WMFG    " }, 0 },
	{ 0x00000083, { "WRW     " }, 0 },
	{ 0x00000083, { "WEW     " }, 0 },
	{ 0x00000083, { "WHG     " }, 0 },
	{ 0x00000083, { "WHGG    " }, 0 },
	{ 0x00000083, { "WCW1    " }, 0 },
	{ 0x00000083, { "WCW2    " }, 0 },
	{ 0x00000083, { "WSW     " }, 0 },
	{ 0x00000083, { "WSWG    " }, 0 },
	{ 0x00000083, { "WMW     " }, 0 },
	{ 0x00000083, { "WFW1    " }, 0 },
	{ 0x00000083, { "WFWG    " }, 0 },
	{ 0x00000083, { "WPW1    " }, 0 },
	{ 0x00000083, { "WPW2    " }, 0 },
	{ 0x00000083, { "WPF     " }, 0 },
	{ 0x00000083, { "WPFG    " }, 0 },
	{ 0x00000083, { "WWTW    " }, 0 },
	{ 0x00000083, { "WMWW    " }, 0 },
	{ 0x00000083, { "WSW1    " }, 0 },
	{ 0x00000083, { "WSW2    " }, 0 },
	{ 0x00000083, { "WGW2    " }, 0 },
	{ 0x00000083, { "WBW     " }, 0 },
	{ 0x00000083, { "WBR1    " }, 0 },
	{ 0x00000083, { "WBRG    " }, 0 },
	{ 0x00000083, { "WBR2    " }, 0 },
	{ 0x00000083, { "WBR3    " }, 0 },
	{ 0x00000083, { "WPW3    " }, 0 },
	{ 0x00000083, { "WJF     " }, 0 },
	{ 0x00000083, { "WCH     " }, 0 },
	{ 0x00000083, { "WCHG    " }, 0 },
	{ 0x00000083, { "WC1     " }, 0 },
	{ 0x00000083, { "WC2     " }, 0 },
	{ 0x00000083, { "WC3     " }, 0 },
	{ 0x00000083, { "WC4     " }, 0 },
	{ 0x00000083, { "WC5     " }, 0 },
	{ 0x00000083, { "WC6     " }, 0 },
	{ 0x00000083, { "WC7     " }, 0 },
	{ 0x00000083, { "WC8     " }, 0 },
	{ 0x00000083, { "WC9     " }, 0 },
	{ 0x00000083, { "WC10    " }, 0 },
	{ 0x00000083, { "WC11    " }, 0 },
	{ 0x00000083, { "WC12    " }, 0 },
	{ 0x00000083, { "WC13    " }, 0 },
	{ 0x00000083, { "WC14    " }, 0 },
	{ 0x00000083, { "WC15    " }, 0 },
	{ 0x00000083, { "WC16    " }, 0 },
	{ 0x00000083, { "WC17    " }, 0 },
	{ 0x00000083, { "WC18    " }, 0 },
	{ 0x00000083, { "WALLBRDR" }, 0 },
	{ 0x00000083, { "WALLBR32" }, 0 },
	{ 0x00000083, { "WALLBR16" }, 0 },
	{ 0x00000083, { "WALLBR8 " }, 0 },
	{ 0x00000083, { "WALLCF8 " }, 0 },
	{ 0x00000083, { "WALLCF16" }, 0 },
	{ 0x00000083, { "WALLCF32" }, 0 },
	{ 0x00000083, { "WALLBB8 " }, 0 },
	{ 0x00000083, { "WALLBB16" }, 0 },
	{ 0x00000083, { "WALLBB32" }, 0 },
	{ 0x00000083, { "WALLRS8 " }, 0 },
	{ 0x00000083, { "WALLRS16" }, 0 },
	{ 0x00000083, { "WALLRS32" }, 0 },
	{ 0x00000083, { "WALLCB8 " }, 0 },
	{ 0x00000083, { "WALLCB16" }, 0 },
	{ 0x00000083, { "WALLCB32" }, 0 },
	{ 0x00000083, { "WALLGL8 " }, 0 },
	{ 0x00000083, { "WALLGL16" }, 0 },
	{ 0x00000083, { "WALLGL32" }, 0 },
	{ 0x00000083, { "WALLWD8 " }, 0 },
	{ 0x00000083, { "WALLWD16" }, 0 },
	{ 0x00000083, { "WALLWD32" }, 0 },
	{ 0x00000083, { "WALLTN32" }, 0 },
	{ 0x00000083, { "WALLJN32" }, 0 },
	{ 0x00000083, { "WALLMN32" }, 0 },
	{ 0x00000083, { "WALLSP32" }, 0 },
	{ 0x00000083, { "WALLPG32" }, 0 },
	{ 0x00000083, { "WALLU132" }, 0 },
	{ 0x00000083, { "WALLU232" }, 0 },
	{ 0x00000083, { "WALLCZ32" }, 0 },
	{ 0x00000083, { "WALLCW32" }, 0 },
	{ 0x00000083, { "WALLCY32" }, 0 },
	{ 0x00000083, { "WALLCX32" }, 0 },
	{ 0x00000083, { "WBR1A   " }, 0 },
	{ 0x00000083, { "WBR2A   " }, 0 },
	{ 0x00000083, { "WRWA    " }, 0 },
	{ 0x00000083, { "WWTWA   " }, 0 },
	{ 0x00000083, { "WALLIG16" }, 0 },
	{ 0x00000083, { "WALLIG24" }, 0 },
	{ 0x00000083, { "WALLCO16" }, 0 },
	{ 0x00000083, { "WALLCFDR" }, 0 },
	{ 0x00000083, { "WALLCBDR" }, 0 },
	{ 0x00000083, { "WALLBRWN" }, 0 },
	{ 0x00000083, { "WALLCFWN" }, 0 },
	{ 0x00000083, { "WALLCBWN" }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsBanner[] = {
	{ 0x00000084, { "BN1     " }, 0 },
	{ 0x00000084, { "BN2     " }, 0 },
	{ 0x00000084, { "BN3     " }, 0 },
	{ 0x00000084, { "BN4     " }, 0 },
	{ 0x00000084, { "BN5     " }, 0 },
	{ 0x00000084, { "BN6     " }, 0 },
	{ 0x00000084, { "BN7     " }, 0 },
	{ 0x00000084, { "BN8     " }, 0 },
	{ 0x00000084, { "BN9     " }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsPath[] = {
	{ 0x00000085, { "TARMAC  " }, 0 },
	{ 0x00000085, { "TARMACB " }, 0 },
	{ 0x00000085, { "PATHSPCE" }, 0 },
	{ 0x00000085, { "PATHDIRT" }, 0 },
	{ 0x00000085, { "ROAD    " }, 0 },
	{ 0x00000085, { "PATHCRZY" }, 0 },
	{ 0x00000085, { "PATHASH " }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsPathBits[] = {
	{ 0x00000086, { "LAMP1   " }, 0 },
	{ 0x00000086, { "LAMP2   " }, 0 },
	{ 0x00000086, { "LITTER1 " }, 0 },
	{ 0x00000086, { "BENCH1  " }, 0 },
	{ 0x00000086, { "JUMPFNT1" }, 0 },
	{ 0x00000086, { "LAMP3   " }, 0 },
	{ 0x00000086, { "LAMP4   " }, 0 },
	{ 0x00000086, { "JUMPSNW1" }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsSceneryGroup[] = {
	{ 0x00000087, { "SCGTREES" }, 0 },
	{ 0x00000087, { "SCGSHRUB" }, 0 },
	{ 0x00000087, { "SCGGARDN" }, 0 },
	{ 0x00000087, { "SCGPATHX" }, 0 },
	{ 0x00000087, { "SCGFENCE" }, 0 },
	{ 0x00000087, { "SCGMART " }, 0 },
	{ 0x00000087, { "SCGWOND " }, 0 },
	{ 0x00000087, { "SCGSNOW " }, 0 },
	{ 0x00000087, { "SCGWALLS" }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsParkEntrance[] = {
	{ 0x00000088, { "PKENT1  " }, 0 }
};

static const rct_object_entry RCT1DefaultObjectsWater[] = {
	{ 0x00000089, { "WTRCYAN " }, 0 },
	{ 0x00000089, { "WTRORNG " }, 0 }
};

static const RCT1DefaultObjectsGroup RCT1DefaultObjects[10] = {
	{ RCT1DefaultObjectsRides,			countof(RCT1DefaultObjectsRides)			},
	{ RCT1DefaultObjectsSmallScenery,	countof(RCT1DefaultObjectsSmallScenery)		},
	{ RCT1DefaultObjectsLargeScenery,	countof(RCT1DefaultObjectsLargeScenery)		},
	{ RCT1DefaultObjectsWall,			countof(RCT1DefaultObjectsWall)				},
	{ RCT1DefaultObjectsBanner,			countof(RCT1DefaultObjectsBanner)			},
	{ RCT1DefaultObjectsPath,			countof(RCT1DefaultObjectsPath)				},
	{ RCT1DefaultObjectsPathBits,		countof(RCT1DefaultObjectsPathBits)			},
	{ RCT1DefaultObjectsSceneryGroup,	countof(RCT1DefaultObjectsSceneryGroup)		},
	{ RCT1DefaultObjectsParkEntrance,	countof(RCT1DefaultObjectsParkEntrance)		},
	{ RCT1DefaultObjectsWater,			countof(RCT1DefaultObjectsWater)			}
};

// Keep these in the same order as gVehicleHierarchies
const char *SpiralRCObjectOrder[]			= { "SPDRCR  "};
const char *StandupRCObjectOrder[]			= { "TOGST   "};
const char *SuspendedSWRCObjectOrder[]		= { "ARRSW1  ", "VEKVAMP ", "ARRSW2 "};
const char *InvertedRCObjectOrder[]			= { "NEMT    "};
const char *JuniorCoasterObjectOrder[]		= { "ZLDB    ", "ZLOG    "};
const char *MiniatureRailwayObjectOrder[]	= { "NRL     ", "NRL2    ", "AML1    ", "TRAM1   "};
const char *MonorailObjectOrder[]			= { "MONO1   ", "MONO2   ", "MONO3   "};
const char *MiniSuspendedRCObjectOrder[] 	= { "BATFL   ", "SKYTR   "};
const char *BoatRideObjectOrder[]			= { "RBOAT   ", "BBOAT   ", "CBOAT   ", "SWANS   ", "TRIKE   ","JSKI    "};
const char *WoodenWMObjectOrder[]			= { "WMOUSE  ", "WMMINE  "};
const char *SteeplechaseObjectOrder[]		= { "STEEP1  ", "STEEP2  ", "SBOX    "};
const char *CarRideObjectOrder[] 			= { "SPCAR   ", "RCR     ", "TRUCK1  ", "VCR     ", "CTCAR   "};
const char *LaunchedFFObjectOrder[] 		= { "SSC1    "};
const char *BobsleighRCObjectOrder[] 		= { "BOB1    ", "INTBOB  "};
const char *ObservationTowerObjectOrder[]	= { "OBS1    ", "OBS2    "};
const char *LoopingRCObjectOrder[]			= { "SCHT1   "};
const char *DinghySlideObjectOrder[] 		= { "DING1   "};
const char *MineTrainRCObjectOrder[] 		= { "AMT1    "};
const char *ChairliftObjectOrder[] 			= { "CLIFT1  ", "CLIFT2  "};
const char *CorkscrewRCObjectOrder[] 		= { "ARRT1   ", "ARRT2   "};
const char *GoKartsObjectOrder[]	 		= { "KART1   "};
const char *LogFlumeObjectOrder[]	 		= { "LFB1    "};
const char *RiverRapidsObjectOrder[]		= { "RAPBOAT "};
const char *ReverseFreefallRCObjectOrder[]	= { "REVF1   "};
const char *LiftObjectOrder[]	 			= { "LIFT1   "};
const char *VerticalDropRCObjectOrder[]		= { "BMVD    "};
const char *GhostTrainObjectOrder[] 		= { "GTC     ", "HMCAR   "};
const char *TwisterRCObjectOrder[]			= { "BMSD    ", "BMSU    ", "BMFL    ", "BMRB    ", "GOLTR   "};
const char *WoodenRCObjectOrder[] 			= { "PTCT1   ", "MFT     ", "PTCT2   "};
const char *SideFrictionRCObjectOrder[]		= { "SFRIC1  "};
const char *SteelWildMouseObjectOrder[] 	= { "SMC1    ", "SMC2    ", "WMSPIN  "};
const char *MultiDimensionRCObjectOrder[]	= { "ARRX    "};
const char *FlyingRCObjectOrder[]			= { "BMAIR   "};
const char *VirginiaReelRCObjectOrder[]		= { "VREEL   "};
const char *SplashBoatsObjectOrder[]		= { "SPBOAT  "};
const char *MiniHelicoptersObjectOrder[]	= { "HELICAR "};
const char *LayDownRCObjectOrder[]			= { "VEKST   "};
const char *SuspendedMonorailObjectOrder[]	= { "SMONO   "};
const char *ReverserRCObjectOrder[]			= { "REVCAR  "};
const char *HeartlineTwisterObjectOrder[]	= { "UTCAR   ", "UTCARR  "};
const char *GigaRCObjectOrder[]				= { "INTST   "};
const char *RotoDropObjectOrder[]			= { "GDROP1  "};
const char *MonorailCyclesObjectOrder[]		= { "MONBK   "};
const char *CompactInvertedRCObjectOrder[]	= { "SLCT    ", "SLCFO    ", "VEKDV   "};
const char *WaterRCObjectOrder[]			= { "CSTBOAT "};
const char *AirPoweredRCObjectOrder[]		= { "THCAR   "};
const char *InvertedHairpinRCObjectOrder[]	= { "IVMC1   "};
const char *SubmarineRideObjectOrder[]		= { "SUBMAR  "};
const char *RiverRaftsObjectOrder[]			= { "RFTBOAT "};
const char *InvertedImpulseRCObjectOrder[]	= { "INTINV  "};
const char *MiniRCObjectOrder[]				= { "WCATC   ", "RCKC     ", "JSTAR1  "};
const char *MineRideRCObjectOrder[]			= { "PMT1    "};
const char *LIMLaunchedRCObjectOrder[]		= { "PREMT1  "};

typedef struct {
	const char **entries;
	int count;
} RCT1VehicleHierarchiesGroup;

RCT1VehicleHierarchiesGroup gVehicleHierarchies[0x60] = {
	{ SpiralRCObjectOrder,			countof(SpiralRCObjectOrder)			}, // 0 Spiral Roller coaster
	{ StandupRCObjectOrder,			countof(StandupRCObjectOrder)			}, // 1 Stand Up Coaster
	{ SuspendedSWRCObjectOrder,		countof(SuspendedSWRCObjectOrder)		}, // 2 Suspended Swinging
	{ InvertedRCObjectOrder,		countof(InvertedRCObjectOrder)			}, // 3 Inverted
	{ JuniorCoasterObjectOrder,		countof(JuniorCoasterObjectOrder)		}, // 4 Junior RC / Steel Mini Coaster
	{ MiniatureRailwayObjectOrder,	countof(MiniatureRailwayObjectOrder)	}, // 5 Mini Railroad
	{ MonorailObjectOrder,			countof(MonorailObjectOrder)			}, // 6 Monorail
	{ MiniSuspendedRCObjectOrder,	countof(MiniSuspendedRCObjectOrder)		}, // 7 Mini Suspended Coaster
	{ BoatRideObjectOrder,			countof(BoatRideObjectOrder)			}, // 8 Boat ride
	{ WoodenWMObjectOrder,			countof(WoodenWMObjectOrder)			}, // 9 Wooden Wild Mine/Mouse
	{ SteeplechaseObjectOrder,		countof(SteeplechaseObjectOrder)		}, // a Steeplechase/Motorbike/Soap Box Derby
	{ CarRideObjectOrder,			countof(CarRideObjectOrder)				}, // b Car Ride
	{ LaunchedFFObjectOrder,		countof(LaunchedFFObjectOrder)			}, // c Launched Freefall
	{ BobsleighRCObjectOrder,		countof(BobsleighRCObjectOrder)			}, // d Bobsleigh Coaster
	{ ObservationTowerObjectOrder,	countof(ObservationTowerObjectOrder)	}, // e Observation Tower
	{ LoopingRCObjectOrder,			countof(LoopingRCObjectOrder)			}, // f Looping Roller Coaster
	{ DinghySlideObjectOrder,		countof(DinghySlideObjectOrder)			}, // 10 Dinghy Slide
	{ MineTrainRCObjectOrder,		countof(MineTrainRCObjectOrder)			}, // 11 Mine Train Coaster
	{ ChairliftObjectOrder,			countof(ChairliftObjectOrder)			}, // 12 Chairlift
	{ CorkscrewRCObjectOrder,		countof(CorkscrewRCObjectOrder)			}, // 13 Corkscrew Roller Coaster
	{ NULL,							0										}, // 14 Maze, N/A
	{ NULL,							0										}, // 15 Spiral Slide, N/A
	{ GoKartsObjectOrder,			countof(GoKartsObjectOrder)				}, // 16 Go Karts
	{ LogFlumeObjectOrder,			countof(LogFlumeObjectOrder)			}, // 17 Log Flume
	{ RiverRapidsObjectOrder,		countof(RiverRapidsObjectOrder)			}, // 18 River Rapids
	{ NULL,							0										}, // 19 Dodgems, N/A
	{ NULL,							0										}, // 1a Pirate Ship, N/A
	{ NULL,							0										}, // 1b Swinging Inverter Ship, N/A
	{ NULL,							0										}, // 1c Food Stall, N/A
	{ NULL,							0										}, // 1d (none), N/A
	{ NULL,							0										}, // 1e Drink Stall, N/A
	{ NULL,							0										}, // 1f (none), N/A
	{ NULL,							0										}, // 20 Shop (all types), N/A
	{ NULL,							0										}, // 21 Merry Go Round, N/A
	{ NULL,							0										}, // 22 Balloon Stall (maybe), N/A
	{ NULL,							0										}, // 23 Information Kiosk, N/A
	{ NULL,							0										}, // 24 Bathroom, N/A
	{ NULL,							0										}, // 25 Ferris Wheel, N/A
	{ NULL,							0										}, // 26 Motion Simulator, N/A
	{ NULL,							0										}, // 27 3D Cinema, N/A
	{ NULL,							0										}, // 28 Top Spin, N/A
	{ NULL,							0										}, // 29 Space Rings, N/A
	{ ReverseFreefallRCObjectOrder,	countof(ReverseFreefallRCObjectOrder)	}, // 2a Reverse Freefall Coaster
	{ LiftObjectOrder,				countof(LiftObjectOrder)				}, // 2b Lift
	{ VerticalDropRCObjectOrder,	countof(VerticalDropRCObjectOrder)		}, // 2c Vertical Drop Roller Coaster
	{ NULL,							0										}, // 2d ATM, N/A
	{ NULL,							0										}, // 2e Twist, N/A
	{ NULL,							0										}, // 2f Haunted House, N/A
	{ NULL,							0										}, // 30 First Aid, N/A
	{ NULL,							0										}, // 31 Circus Show, N/A
	{ GhostTrainObjectOrder,		countof(GhostTrainObjectOrder)			}, // 32 Ghost Train
	{ TwisterRCObjectOrder,			countof(TwisterRCObjectOrder)			}, // 33 Twister Roller Coaster
	{ WoodenRCObjectOrder,			countof(WoodenRCObjectOrder)			}, // 34 Wooden Roller Coaster
	{ SideFrictionRCObjectOrder,	countof(SideFrictionRCObjectOrder)		}, // 35 Side-Friction Roller Coaster
	{ SteelWildMouseObjectOrder,	countof(SteelWildMouseObjectOrder)		}, // 36 Steel Wild Mouse
	{ MultiDimensionRCObjectOrder,	countof(MultiDimensionRCObjectOrder)	}, // 37 Multi Dimension Coaster
	{ NULL,							0										}, // 38 (none), N/A
	{ FlyingRCObjectOrder,			countof(FlyingRCObjectOrder)			}, // 39 Flying Roller Coaster
	{ NULL,							0										}, // 3a (none), N/A
	{ VirginiaReelRCObjectOrder,	countof(VirginiaReelRCObjectOrder)		}, // 3b Virginia Reel
	{ SplashBoatsObjectOrder,		countof(SplashBoatsObjectOrder)			}, // 3c Splash Boats
	{ MiniHelicoptersObjectOrder,	countof(MiniHelicoptersObjectOrder)		}, // 3d Mini Helicopters
	{ LayDownRCObjectOrder,			countof(LayDownRCObjectOrder)			}, // 3e Lay-down Roller Coaster
	{ SuspendedMonorailObjectOrder,	countof(SuspendedMonorailObjectOrder)	}, // 3f Suspended Monorail
	{ NULL,							0										}, // 40 (none), N/A
	{ ReverserRCObjectOrder,		countof(ReverserRCObjectOrder)			}, // 41 Reverser Roller Coaster
	{ HeartlineTwisterObjectOrder,	countof(HeartlineTwisterObjectOrder)	}, // 42 Heartline Twister Roller Coaster
	{ NULL,							0										}, // 43 Mini Golf, N/A
	{ GigaRCObjectOrder,			countof(GigaRCObjectOrder)				}, // 44 Giga Coaster
	{ RotoDropObjectOrder,			countof(RotoDropObjectOrder)			}, // 45 Roto-Drop
	{ NULL,							0										}, // 46 Flying Saucers, N/A
	{ NULL,							0										}, // 47 Crooked House, N/A
	{ MonorailCyclesObjectOrder,	countof(MonorailCyclesObjectOrder)		}, // 48 Monorail Cycles
	{ CompactInvertedRCObjectOrder,	countof(CompactInvertedRCObjectOrder)	}, // 49 Compact Inverted Coaster
	{ WaterRCObjectOrder,			countof(WaterRCObjectOrder)				}, // 4a Water Coaster
	{ AirPoweredRCObjectOrder,		countof(AirPoweredRCObjectOrder)		}, // 4b Air Powered Vertical Coaster
	{ InvertedHairpinRCObjectOrder,	countof(InvertedHairpinRCObjectOrder)	}, // 4c Inverted Hairpin Coaster
	{ NULL,							0										}, // 4d Magic Carpet, N/A
	{ SubmarineRideObjectOrder,		countof(SubmarineRideObjectOrder)		}, // 4e Submarine Ride
	{ RiverRaftsObjectOrder,		countof(RiverRaftsObjectOrder)			}, // 4f River Rafts
	{ NULL,							0										}, // 50 (none), N/A
	{ NULL,							0										}, // 51 Enterprise, N/A
	{ NULL,							0										}, // 52 (none), N/A
	{ NULL,							0										}, // 53 (none), N/A
	{ NULL,							0										}, // 54 (none), N/A
	{ NULL,							0										}, // 55 (none), N/A
	{ InvertedImpulseRCObjectOrder,	countof(InvertedImpulseRCObjectOrder)	}, // 56 Inverted Impulse Coaster
	{ MiniRCObjectOrder,			countof(MiniRCObjectOrder)				}, // 57 Mini Roller Coaster
	{ MineRideRCObjectOrder,		countof(MineRideRCObjectOrder)			}, // 58 Mine Ride
	{ NULL,							0										}, // 59 Unknown Ride
	{ LIMLaunchedRCObjectOrder,		countof(LIMLaunchedRCObjectOrder)		}, // 60 LIM Launched Roller Coaster
};

const uint8 gRideCategories[0x60] = {
		2,		 // Spiral Roller coaster
		2,		 // Stand Up Coaster
		2,		 // Suspended Swinging
		2,		 // Inverted
		2,		 // Steel Mini Coaster
		0,		 // Mini Railroad
		0,		 // Monorail
		2,		 // Mini Suspended Coaster
		4,		 // Boat ride
		2,		 // Wooden Wild Mine/Mouse
		2,		 // Steeplechase/Motorbike/Soap Box Derby
		1,		 // Car Ride
		3,		 // Launched Freefall
		2,		 // Bobsleigh Coaster
		1,		 // Observation Tower
		2,		 // Looping Roller Coaster
		4,		 // Dinghy Slide
		2,		 // Mine Train Coaster
		0,		 // Chairlift
		2,		 // Corkscrew Roller Coaster
		1,		 // Maze
		1,		 // Spiral Slide
		3,		 // Go Karts
		4,		 // Log Flume
		4,		 // River Rapids
		1,		 // Dodgems
		3,		 // Pirate Ship
		3,		 // Swinging Inverter Ship
		5,		 // Food Stall
		255,	 // (none)
		5,		 // Drink Stall
		255,	 // (none)
		5,		 // Shop (all types)
		1,		 // Merry Go Round
		5,		 // Balloon Stall (maybe)
		5,		 // Information Kiosk
		5,		 // Bathroom
		1,		 // Ferris Wheel
		3,		 // Motion Simulator
		3,		 // 3D Cinema
		3,		 // Top Spin
		1,		 // Space Rings
		2,		 // Reverse Freefall Coaster
		0,		 // Elevator
		2,		 // Vertical Drop Roller Coaster
		5,		 // ATM
		3,		 // Twist
		1,		 // Haunted House
		5,		 // First Aid
		1,		 // Circus Show
		1,		 // Ghost Train
		2,		 // Twister Roller Coaster
		2,		 // Wooden Roller Coaster
		2,		 // Side-Friction Roller Coaster
		2,		 // Wild Mouse
		2,		 // Multi Dimension Coaster
		255,	 // (none)
		2,		 // Flying Roller Coaster
		255,	 // (none)
		2,		 // Virginia Reel
		4,		 // Splash Boats
		1,		 // Mini Helicopters
		2,		 // Lay-down Roller Coaster
		0,		 // Suspended Monorail
		255,	 // (none)
		2,		 // Reverser Roller Coaster
		2,		 // Heartline Twister Roller Coaster
		1,		 // Mini Golf
		2,		 // Giga Coaster
		3,		 // Roto-Drop
		1,		 // Flying Saucers
		1,		 // Crooked House
		1,		 // Monorail Cycles
		2,		 // Compact Inverted Coaster
		2,		 // Water Coaster
		2,		 // Air Powered Vertical Coaster
		2,		 // Inverted Hairpin Coaster
		3,		 // Magic Carpet
		4,		 // Submarine Ride
		4,		 // River Rafts
		255,	 // (none)
		3,		 // Enterprise
		255,	 // (none)
		255,	 // (none)
		255,	 // (none)
		255,	 // (none)
		2,		 // Inverted Impulse Coaster
		2,		 // Mini Roller Coaster
		2,		 // Mine Ride
		255,	 //59 Unknown Ride
		2		 // LIM Launched Roller Coaster
};

/*  This function keeps a list of the preferred vehicle for every generic track type, out of the available vehicle types in the current game.
	It determines which picture is shown on the new ride tab and which train type is selected by default.*/
bool vehicleIsHigherInHierarchy(int track_type, char *currentVehicleName, char *comparedVehicleName)
{
	if(currentVehicleName==NULL || comparedVehicleName==NULL || gVehicleHierarchies[track_type].entries==NULL) {
		return false;
	}

	int currentVehicleHierarchy;
	int comparedVehicleHierarchy;

	currentVehicleHierarchy=255;
	comparedVehicleHierarchy=255;

	for(int i = 0; i < gVehicleHierarchies[track_type].count; i++) {
		if(gVehicleHierarchies[track_type].entries[i]==NULL)
			continue;

		if(strcmp(comparedVehicleName,gVehicleHierarchies[track_type].entries[i])==0)
			comparedVehicleHierarchy=i;

		if(strcmp(currentVehicleName,gVehicleHierarchies[track_type].entries[i])==0)
			currentVehicleHierarchy=i;
	}

	if(comparedVehicleHierarchy<currentVehicleHierarchy) {
		return true;
	}
	return false;
}

bool rideTypeShouldLoseSeparateFlag(rct_ride_type *ride)
{
	if(!gConfigInterface.select_by_track_type)
		return false;

	bool remove_flag=true;
	for(int j=0;j<3;j++)
	{
		if(ride_type_has_flag(ride->ride_type[j], RIDE_TYPE_FLAG_FLAT_RIDE))
			remove_flag=false;
		if(ride->ride_type[j]==RIDE_TYPE_MAZE || ride->ride_type[j]==RIDE_TYPE_MINI_GOLF)
			remove_flag=false;
	}
	return remove_flag;
}

#pragma endregion
