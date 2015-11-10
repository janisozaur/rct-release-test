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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "common.h"
#include "platform/platform.h"

// First 0xF of rct_object_entry->flags
typedef enum{
	OBJECT_TYPE_RIDE,
	OBJECT_TYPE_SMALL_SCENERY,
	OBJECT_TYPE_LARGE_SCENERY,
	OBJECT_TYPE_WALLS,
	OBJECT_TYPE_BANNERS,
	OBJECT_TYPE_PATHS,
	OBJECT_TYPE_PATH_BITS,
	OBJECT_TYPE_SCENERY_SETS,
	OBJECT_TYPE_PARK_ENTRANCE,
	OBJECT_TYPE_WATER,
	OBJECT_TYPE_SCENARIO_TEXT
}OBJECT_TYPE;

typedef enum{
	OBJECT_SELECTION_FLAG_SELECTED = (1 << 0),
	OBJECT_SELECTION_FLAG_2 = (1 << 1),
	OBJECT_SELECTION_FLAG_IN_USE = (1 << 2),
	OBJECT_SELECTION_FLAG_REQUIRED = (1 << 3),
	OBJECT_SELECTION_FLAG_ALWAYS_REQUIRED = (1 << 4),
	OBJECT_SELECTION_FLAG_6 = (1 << 5),
	OBJECT_SELECTION_FLAG_7 = (1 << 6),
	OBJECT_SELECTION_FLAG_8 = (1 << 7),
}OBJECT_SELECTION_FLAGS;

/**
 * Object entry structure.
 * size: 0x10
 */
typedef struct {
	uint32 flags;
	char name[8];
	uint32 checksum;
} rct_object_entry;

/**
 * Object entry structure extended.
 * size: 0x14
 */
typedef struct {
	uint32 flags;
	char name[8];
	uint32 checksum;
	uint32 chunk_size;
} rct_object_entry_extended;

extern int object_entry_group_counts[];
extern int object_entry_group_encoding[];

typedef struct {
	uint8 **chunks;
	rct_object_entry_extended *entries;
} rct_object_entry_group;

typedef struct {
	uint8 category[2];
	uint8 ride_type;
} rct_ride_filters;

typedef struct {
	union {
		rct_ride_filters ride;
	};
} rct_object_filters;

extern rct_object_entry_group object_entry_groups[];
extern char gTempObjectLoadName[9];

int object_load_entry(const utf8 *path, rct_object_entry *outEntry);
void object_list_load();
void set_load_objects_fail_reason();
int object_read_and_load_entries(SDL_RWops* rw);
int object_load_packed(SDL_RWops* rw);
void object_unload_all();

int check_object_entry(rct_object_entry *entry);
int object_load(int groupIndex, rct_object_entry *entry, int* chunk_size);
int object_load_file(int groupIndex, const rct_object_entry *entry, int* chunkSize, const rct_object_entry *installedObject);
void object_unload(rct_object_entry *entry);
int object_get_scenario_text(rct_object_entry *entry);
void object_free_scenario_text();
int object_get_length(rct_object_entry *entry);
int object_entry_compare(const rct_object_entry *a, const rct_object_entry *b);
int object_calculate_checksum(const rct_object_entry *entry, const uint8 *data, int dataLength);
int object_paint(int type, int eax, int ebx, int ecx, int edx, int esi, int edi, int ebp);
rct_object_entry *object_get_next(rct_object_entry *entry);
int write_object_file(SDL_RWops* rw, rct_object_entry* entry);
void reset_loaded_objects();
int find_object_in_entry_group(rct_object_entry* entry, uint8* entry_type, uint8* entry_index);
void object_create_identifier_name(char* string_buffer, const rct_object_entry* object);

rct_object_entry *object_list_find(rct_object_entry *entry);

char *object_get_name(rct_object_entry *entry);

rct_object_filters *get_object_filter(int index);

#endif
