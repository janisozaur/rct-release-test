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

#ifndef _UTIL_H_
#define _UTIL_H_

#include "../common.h"

int squaredmetres_to_squaredfeet(int squaredMetres);
int metres_to_feet(int metres);
int mph_to_kmph(int mph);

bool filename_valid_characters(const utf8 *filename);

const char *path_get_filename(const utf8 *path);
const char *path_get_extension(const utf8 *path);
void path_set_extension(utf8 *path, const utf8 *newExtension);
void path_remove_extension(utf8 *path);
bool readentirefile(const utf8 *path, void **outBuffer, int *outLength);

int bitscanforward(int source);
int bitcount(int source);
bool strequals(const char *a, const char *b, int length, bool caseInsensitive);
int strcicmp(char const *a, char const *b);
char *safe_strncpy(char * destination, const char * source, size_t num);

bool utf8_is_bom(const char *str);
bool str_is_null_or_empty(const char *str);

#endif
