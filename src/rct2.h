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

#ifndef _RCT2_H_
#define _RCT2_H_

#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES
#endif
#undef M_PI

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
	#include <time.h>
#endif

typedef signed char sint8;
typedef signed short sint16;
typedef signed long sint32;
typedef signed long long sint64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned long long uint64;

typedef char utf8;
typedef utf8* utf8string;
typedef const utf8* const_utf8string;
typedef wchar_t utf16;
typedef utf16* utf16string;

#define rol8(x, shift)		(((uint8)(x) << (shift)) | ((uint8)(x) >> (8 - (shift))))
#define ror8(x, shift)		(((uint8)(x) >> (shift)) | ((uint8)(x) << (8 - (shift))))
#define rol16(x, shift)		(((uint16)(x) << (shift)) | ((uint16)(x) >> (16 - (shift))))
#define ror16(x, shift)		(((uint16)(x) >> (shift)) | ((uint16)(x) << (16 - (shift))))
#define rol32(x, shift)		(((uint32)(x) << (shift)) | ((uint32)(x) >> (32 - (shift))))
#define ror32(x, shift)		(((uint32)(x) >> (shift)) | ((uint32)(x) << (32 - (shift))))
#define rol64(x, shift)		(((uint64)(x) << (shift)) | ((uint32)(x) >> (64 - (shift))))
#define ror64(x, shift)		(((uint64)(x) >> (shift)) | ((uint32)(x) << (64 - (shift))))
#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
	#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#define sgn(x)				((x > 0) ? 1 : ((x < 0) ? -1 : 0))
#define clamp(l, x, h)		(min(h, max(l, x)))

// Rounds an integer down to the given power of 2. y must be a power of 2.
#define floor2(x, y)		((x) & (~((y) - 1)))

#define countof(x)			((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#ifndef _MSC_VER
// use similar struct packing as MSVC for our structs
#pragma pack(1)
#endif

#define OPENRCT2_NAME				"OpenRCT2"
#define OPENRCT2_VERSION			"0.0.3"
#define OPENRCT2_ARCHITECTURE		"x86"
#ifdef _WIN32
	#define OPENRCT2_PLATFORM		"Windows"
#endif // _WIN32
#ifdef __linux__
	#define OPENRCT2_PLATFORM		"Linux"
#endif
#ifdef __APPLE__
	#define OPENRCT2_PLATFORM		"OS X"
#endif
#ifndef OPENRCT2_PLATFORM
	#error Unknown platform!
#endif
#define OPENRCT2_TIMESTAMP			__DATE__ " " __TIME__

// The following constants are for automated build servers
#define OPENRCT2_BUILD_NUMBER		""
#define OPENRCT2_BUILD_SERVER		""
#define OPENRCT2_BRANCH				"develop"
#define OPENRCT2_COMMIT_SHA1		""
#define OPENRCT2_COMMIT_SHA1_SHORT	""
#define OPENRCT2_MASTER_SERVER_URL	"https://servers.openrct2.website"

// Represent fixed point numbers. dp = decimal point
typedef uint8 fixed8_1dp;
typedef uint8 fixed8_2dp;
typedef sint16 fixed16_1dp;
typedef sint16 fixed16_2dp;
typedef sint32 fixed32_1dp;
typedef sint32 fixed32_2dp;

// Money is stored as a multiple of 0.10.
typedef fixed8_1dp money8;
typedef fixed16_1dp money16;
typedef fixed32_1dp money32;

// Construct a fixed point number. For example, to create the value 3.65 you
// would write FIXED_2DP(3,65)
#define FIXED_XDP(x, whole, fraction)	((whole) * (10 * x) + (fraction))
#define FIXED_1DP(whole, fraction)		FIXED_XDP(1, whole, fraction)
#define FIXED_2DP(whole, fraction)		FIXED_XDP(10, whole, fraction)

// Construct a money value in the format MONEY(10,70) to represent 10.70. Fractional part must be two digits.
#define MONEY(whole, fraction)			((whole) * 10 + ((fraction) / 10))

#define MONEY_FREE						MONEY(0,00)
#define MONEY32_UNDEFINED				((money32)0x80000000)
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

typedef void (EMPTY_ARGS_VOID_POINTER)();
typedef unsigned short rct_string_id;

typedef struct {
	uint32 installLevel;
	char title[260];
	char path[260];
	uint32 var_20C;
	uint8 pad_210[256];
	char expansionPackNames[16][128];
	uint32 activeExpansionPacks;		//0xB10
} rct2_install_info;

enum {
	// Although this is labeled a flag it actually means when
	// zero the screen is in playing mode.
	SCREEN_FLAGS_PLAYING = 0,
	SCREEN_FLAGS_TITLE_DEMO = 1,
	SCREEN_FLAGS_SCENARIO_EDITOR = 2,
	SCREEN_FLAGS_TRACK_DESIGNER = 4,
	SCREEN_FLAGS_TRACK_MANAGER = 8,
};

#define SCREEN_FLAGS_EDITOR (SCREEN_FLAGS_SCENARIO_EDITOR | SCREEN_FLAGS_TRACK_DESIGNER | SCREEN_FLAGS_TRACK_MANAGER)

enum {
	PATH_ID_G1,
	PATH_ID_PLUGIN,
	PATH_ID_CSS1,
	PATH_ID_CSS2,
	PATH_ID_CSS4,
	PATH_ID_CSS5,
	PATH_ID_CSS6,
	PATH_ID_CSS7,
	PATH_ID_CSS8,
	PATH_ID_CSS9,
	PATH_ID_CSS11,
	PATH_ID_CSS12,
	PATH_ID_CSS13,
	PATH_ID_CSS14,
	PATH_ID_CSS15,
	PATH_ID_CSS3,
	PATH_ID_CSS17,
	PATH_ID_CSS18,
	PATH_ID_CSS19,
	PATH_ID_CSS20,
	PATH_ID_CSS21,
	PATH_ID_CSS22,
	PATH_ID_SCORES,
	PATH_ID_CSS23,
	PATH_ID_CSS24,
	PATH_ID_CSS25,
	PATH_ID_CSS26,
	PATH_ID_CSS27,
	PATH_ID_CSS28,
	PATH_ID_CSS29,
	PATH_ID_CSS30,
	PATH_ID_CSS31,
	PATH_ID_CSS32,
	PATH_ID_CSS33,
	PATH_ID_CSS34,
	PATH_ID_CSS35,
	PATH_ID_CSS36,
	PATH_ID_CSS37,
	PATH_ID_CSS38,
	PATH_ID_CUSTOM1,
	PATH_ID_CUSTOM2,
	PATH_ID_CSS39,
	PATH_ID_CSS40,
	PATH_ID_CSS41,
	PATH_ID_SIXFLAGS_MAGICMOUNTAIN,
	PATH_ID_CSS42,
	PATH_ID_CSS43,
	PATH_ID_CSS44,
	PATH_ID_CSS45,
	PATH_ID_CSS46,
	PATH_ID_CSS50,
	PATH_ID_END
};

// rct2 @ 0x0097F67C
static const char * const file_paths[] =
{
	"Data\\g1.dat",
	"Data\\plugin.dat",
	"Data\\css1.dat",
	"Data\\css2.dat",
	"Data\\css4.dat",
	"Data\\css5.dat",
	"Data\\css6.dat",
	"Data\\css7.dat",
	"Data\\css8.dat",
	"Data\\css9.dat",
	"Data\\css11.dat",
	"Data\\css12.dat",
	"Data\\css13.dat",
	"Data\\css14.dat",
	"Data\\css15.dat",
	"Data\\css3.dat",
	"Data\\css17.dat",
	"Data\\css18.dat",
	"Data\\css19.dat",
	"Data\\css20.dat",
	"Data\\css21.dat",
	"Data\\css22.dat",
	"Saved Games\\scores.dat",
	"Data\\css23.dat",
	"Data\\css24.dat",
	"Data\\css25.dat",
	"Data\\css26.dat",
	"Data\\css27.dat",
	"Data\\css28.dat",
	"Data\\css29.dat",
	"Data\\css30.dat",
	"Data\\css31.dat",
	"Data\\css32.dat",
	"Data\\css33.dat",
	"Data\\css34.dat",
	"Data\\css35.dat",
	"Data\\css36.dat",
	"Data\\css37.dat",
	"Data\\css38.dat",
	"Data\\CUSTOM1.WAV",
	"Data\\CUSTOM2.WAV",
	"Data\\css39.dat",
	"Data\\css40.dat",
	"Data\\css41.dat",
	"Scenarios\\Six Flags Magic Mountain.SC6",
	"Data\\css42.dat",
	"Data\\css43.dat",
	"Data\\css44.dat",
	"Data\\css45.dat",
	"Data\\css46.dat",
	"Data\\css50.dat"
};

// Files to check (rct2 @ 0x0097FB5A)
static const struct file_to_check
{
	int pathId; // ID of file
	unsigned int fileSize; // Expected size in bytes
} files_to_check[] = {
	{ PATH_ID_END,          0 }
};

extern uint32 gCurrentDrawCount;

int rct2_init();
void rct2_update();
void rct2_draw();
void rct2_endupdate();
void subsitute_path(char *dest, const char *path, const char *filename);
int check_mutex();
int check_file_paths();
int check_file_path(int pathId);
int check_files_integrity();
const char *get_file_path(int pathId);
void get_system_info();
void get_system_time();
void get_local_time();
void *rct2_malloc(size_t numBytes);
void *rct2_realloc(void *block, size_t numBytes);
void rct2_free(void *block);
void rct2_quit();

int rct2_open_file(const char *path);

#endif
