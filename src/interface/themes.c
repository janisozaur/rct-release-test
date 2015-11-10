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

#include "../localisation/string_ids.h"
#include "../util/util.h"
#include "colour.h"
#include "window.h"
#include "themes.h"

#define COLOURS_1(c0) 1, { { (c0), 0, 0, 0, 0, 0 } }
#define COLOURS_2(c0, c1) 2, { { (c0), (c1), 0, 0, 0, 0 } }
#define COLOURS_3(c0, c1, c2) 3, { { (c0), (c1), (c2), 0, 0, 0 } }
#define COLOURS_4(c0, c1, c2, c3) 4, { { (c0), (c1), (c2), (c3), 0, 0 } }
#define COLOURS_5(c0, c1, c2, c3, c4) 5, { { (c0), (c1), (c2), (c3), (c4), 0 } }
#define COLOURS_6(c0, c1, c2, c3, c4, c5) 6, { { (c0), (c1), (c2), (c3), (c4), (c5) } }

#define THEME_DEF_END { 0xFF, { 0, 0, 0, 0, 0, 0 } }

#define TWINDOW(window_class, window_name, window_string_id, theme) { window_class, window_name, window_string_id, theme }

theme_window_definition gThemeWindowDefinitions[] = {
	/* Window Class                        ini section name                    stringid  window defaults               */
	{ WC_TOP_TOOLBAR,						"top_toolbar",						5245,	COLOURS_4(COLOUR_LIGHT_BLUE,					COLOUR_DARK_GREEN,					COLOUR_DARK_BROWN,					COLOUR_GREY				) },
	{ WC_BOTTOM_TOOLBAR,					"bottom_toolbar",					5246,	COLOURS_4(TRANSLUCENT(COLOUR_DARK_GREEN),		TRANSLUCENT(COLOUR_DARK_GREEN),		COLOUR_BLACK,						COLOUR_BRIGHT_GREEN		) },
	{ WC_RIDE,								"ride",								5203,	COLOURS_3(COLOUR_GREY,							COLOUR_BORDEAUX_RED,				COLOUR_GREY													) },
	{ WC_RIDE_CONSTRUCTION,					"ride_construction",				5199,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_RIDE_LIST,							"ride_list",						5204,	COLOURS_3(COLOUR_GREY,							COLOUR_BORDEAUX_RED,				COLOUR_BORDEAUX_RED											) },
	{ WC_SAVE_PROMPT,						"save_prompt",						5223,	COLOURS_1(TRANSLUCENT(COLOUR_BORDEAUX_RED)																										) },
	{ WC_CONSTRUCT_RIDE,					"new_ride",							5201,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_BORDEAUX_RED,				COLOUR_BORDEAUX_RED											) },
	{ WC_DEMOLISH_RIDE_PROMPT,				"demolish_ride_prompt",				5224,	COLOURS_1(TRANSLUCENT(COLOUR_BORDEAUX_RED)																										) },
	{ WC_SCENERY,							"scenery",							5197,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_GREEN,					COLOUR_DARK_GREEN											) },
	{ WC_OPTIONS,							"options",							5219,	COLOURS_3(COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE											) },
	{ WC_FOOTPATH,							"footpath",							5198,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_LAND,								"land",								5193, 	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_WATER,								"water",							5194,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_PEEP,								"guest",							5205,	COLOURS_3(COLOUR_GREY,							COLOUR_OLIVE_GREEN,					COLOUR_OLIVE_GREEN											) },
	{ WC_GUEST_LIST,						"guest_list",						5206,	COLOURS_3(COLOUR_GREY,							COLOUR_OLIVE_GREEN,					COLOUR_OLIVE_GREEN											) },
	{ WC_STAFF_LIST,						"staff_list",						5208,	COLOURS_3(COLOUR_GREY,							COLOUR_LIGHT_PURPLE,				COLOUR_LIGHT_PURPLE											) },
	{ WC_FIRE_PROMPT,						"staff_fire_prompt",				5225,	COLOURS_1(TRANSLUCENT(COLOUR_BORDEAUX_RED)																										) },
	{ WC_PARK_INFORMATION,					"park_information",					5253,	COLOURS_3(COLOUR_GREY,							COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW											) },
	{ WC_FINANCES,							"finances",							5187,	COLOURS_3(COLOUR_GREY,							COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW											) },
	{ WC_TITLE_MENU,						"title_menu",						5249,	COLOURS_3(TRANSLUCENT(COLOUR_DARK_GREEN),		TRANSLUCENT(COLOUR_DARK_GREEN),		TRANSLUCENT(COLOUR_DARK_GREEN)								) },
	{ WC_TITLE_EXIT,						"title_exit",						5250,	COLOURS_3(TRANSLUCENT(COLOUR_DARK_GREEN),		TRANSLUCENT(COLOUR_DARK_GREEN),		TRANSLUCENT(COLOUR_DARK_GREEN)								) },
	{ WC_RECENT_NEWS,						"recent_news",						5192,	COLOURS_3(COLOUR_GREY,							COLOUR_GREY,						COLOUR_BLACK												) },
	{ WC_SCENARIO_SELECT,					"scenario_select",					5252,	COLOURS_3(COLOUR_GREY,							COLOUR_BORDEAUX_RED,				COLOUR_BORDEAUX_RED											) },
	{ WC_TRACK_DESIGN_LIST,					"track_design_list",				5202,	COLOURS_3(COLOUR_BORDEAUX_RED,					COLOUR_BORDEAUX_RED,				COLOUR_BORDEAUX_RED											) },
	{ WC_TRACK_DESIGN_PLACE,				"track_design_place",				5200,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_NEW_CAMPAIGN,						"new_campaign",						5188,	COLOURS_3(COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW											) },
	{ WC_KEYBOARD_SHORTCUT_LIST,			"keyboard_shortcuts",				5220,	COLOURS_3(COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE											) },
	{ WC_CHANGE_KEYBOARD_SHORTCUT,			"change_keyboard_shortcut",			5221,	COLOURS_3(COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE											) },
	{ WC_MAP,								"map",								5190,	COLOURS_2(COLOUR_DARK_GREEN,					COLOUR_DARK_BROWN																				) },
	{ WC_BANNER,							"banner",							5209,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_EDITOR_OBJECT_SELECTION,			"editor_object_selection",			5210,	COLOURS_3(COLOUR_LIGHT_PURPLE,					COLOUR_GREY,						COLOUR_GREY													) },
	{ WC_EDITOR_INVENTION_LIST,				"editor_invention_list",			5211,	COLOURS_3(COLOUR_LIGHT_PURPLE,					COLOUR_GREY,						COLOUR_GREY													) },
	{ WC_EDITOR_SCENARIO_OPTIONS,			"editor_scenario_options",			5212,	COLOURS_3(COLOUR_LIGHT_PURPLE,					COLOUR_GREY,						COLOUR_GREY													) },
	{ WC_EDTIOR_OBJECTIVE_OPTIONS,			"editor_objection_options",			5213,	COLOURS_3(COLOUR_LIGHT_PURPLE,					COLOUR_GREY,						COLOUR_GREY													) },
	{ WC_MANAGE_TRACK_DESIGN,				"manage_track_design",				5215,	COLOURS_3(COLOUR_GREY,							COLOUR_GREY,						COLOUR_GREY													) },
	{ WC_TRACK_DELETE_PROMPT,				"track_delete_prompt",				5226,	COLOURS_3(COLOUR_BORDEAUX_RED,					COLOUR_BORDEAUX_RED,				COLOUR_BORDEAUX_RED											) },
	{ WC_INSTALL_TRACK,						"install_track",					5216,	COLOURS_3(COLOUR_BORDEAUX_RED,					COLOUR_BORDEAUX_RED,				COLOUR_BORDEAUX_RED											) },
	{ WC_CLEAR_SCENERY,						"clear_scenery",					5195,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_CHEATS,							"cheats",							5217,	COLOURS_3(COLOUR_GREY,							COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW											) },
	{ WC_RESEARCH,							"research",							5189,	COLOURS_3(COLOUR_GREY,							COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW											) },
	{ WC_VIEWPORT,							"viewport",							5191,	COLOURS_3(COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_MAPGEN,							"map_generation",					5214,	COLOURS_3(COLOUR_DARK_GREEN,					COLOUR_DARK_BROWN,					COLOUR_DARK_BROWN											) },
	{ WC_LOADSAVE,							"loadsave",							5222,	COLOURS_3(COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE,					COLOUR_LIGHT_BLUE											) },
	{ WC_LOADSAVE_OVERWRITE_PROMPT,			"loadsave_overwrite_prompt",		5227,	COLOURS_1(TRANSLUCENT(COLOUR_BORDEAUX_RED)																										) },
	{ WC_TITLE_OPTIONS,						"title_options",					5251,	COLOURS_3(TRANSLUCENT(COLOUR_DARK_GREEN),		TRANSLUCENT(COLOUR_DARK_GREEN),		TRANSLUCENT(COLOUR_DARK_GREEN)								) },
	{ WC_LAND_RIGHTS,						"land_rights",						5196,	COLOURS_3(COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW,					COLOUR_DARK_YELLOW											) },
	{ WC_THEMES,							"themes",							5218, 	COLOURS_3(COLOUR_GREY,							COLOUR_DARK_GREEN,					COLOUR_DARK_GREEN											) },
	{ WC_STAFF,								"staff",							5207,	COLOURS_3(COLOUR_GREY,							COLOUR_LIGHT_PURPLE,				COLOUR_LIGHT_PURPLE											) },
	{ WC_EDITOR_TRACK_BOTTOM_TOOLBAR,		"editor_track_bottom_toolbar",		5247,	COLOURS_3(TRANSLUCENT(COLOUR_LIGHT_BLUE),		TRANSLUCENT(COLOUR_LIGHT_BLUE),		TRANSLUCENT(COLOUR_LIGHT_BLUE)								) },
	{ WC_EDITOR_SCENARIO_BOTTOM_TOOLBAR,	"editor_scenario_bottom_toolbar",	5248,	COLOURS_3(TRANSLUCENT(COLOUR_LIGHT_BROWN),		TRANSLUCENT(COLOUR_LIGHT_BROWN),	TRANSLUCENT(COLOUR_MOSS_GREEN)								) },
	{ WC_TITLE_EDITOR,						"title_sequences",					5433,	COLOURS_3(COLOUR_GREY,							COLOUR_OLIVE_GREEN,					COLOUR_OLIVE_GREEN											) },
};

#define COLOURS_RCT1(c0, c1, c2, c3, c4, c5) { { (c0), (c1), (c2), (c3), (c4), (c5) } }

theme_window_preset gThemeWindowsRCT1[] = {
	{ WC_TOP_TOOLBAR,		COLOURS_RCT1(COLOUR_GREY,				COLOUR_GREY,				COLOUR_GREY,				COLOUR_GREY,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_BOTTOM_TOOLBAR,	COLOURS_RCT1(TRANSLUCENT(COLOUR_GREY),	TRANSLUCENT(COLOUR_GREY),	COLOUR_BLACK,				COLOUR_YELLOW,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_RIDE,				COLOURS_RCT1(COLOUR_BORDEAUX_RED,		COLOUR_GREY,				COLOUR_SATURATED_GREEN,		COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_RIDE_LIST,			COLOURS_RCT1(COLOUR_BORDEAUX_RED,		COLOUR_GREY,				COLOUR_GREY,				COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_CONSTRUCT_RIDE,	COLOURS_RCT1(COLOUR_BORDEAUX_RED,		COLOUR_GREY,				COLOUR_GREY,				COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_PEEP,				COLOURS_RCT1(COLOUR_LIGHT_BROWN,		COLOUR_BORDEAUX_RED,		COLOUR_BORDEAUX_RED,		COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_GUEST_LIST,		COLOURS_RCT1(COLOUR_LIGHT_BROWN,		COLOUR_BORDEAUX_RED,		COLOUR_BORDEAUX_RED,		COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_STAFF_LIST,		COLOURS_RCT1(COLOUR_DARK_GREEN,			COLOUR_LIGHT_PURPLE,		COLOUR_LIGHT_PURPLE,		COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_FINANCES,			COLOURS_RCT1(COLOUR_LIGHT_PURPLE,		COLOUR_GREY,				COLOUR_GREY,				COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_TITLE_MENU,		COLOURS_RCT1(TRANSLUCENT(COLOUR_GREY),	TRANSLUCENT(COLOUR_GREY),	TRANSLUCENT(COLOUR_GREY),	COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_TITLE_EXIT,		COLOURS_RCT1(TRANSLUCENT(COLOUR_GREY),	TRANSLUCENT(COLOUR_GREY),	TRANSLUCENT(COLOUR_GREY),	COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_NEW_CAMPAIGN,		COLOURS_RCT1(COLOUR_LIGHT_PURPLE,		COLOUR_LIGHT_PURPLE,		COLOUR_GREY,				COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_TITLE_OPTIONS,		COLOURS_RCT1(TRANSLUCENT(COLOUR_GREY),	TRANSLUCENT(COLOUR_GREY),	TRANSLUCENT(COLOUR_GREY),	COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	{ WC_STAFF,				COLOURS_RCT1(COLOUR_DARK_GREEN,			COLOUR_LIGHT_PURPLE,		COLOUR_LIGHT_PURPLE,		COLOUR_BLACK,	COLOUR_BLACK,	COLOUR_BLACK)	},
	THEME_DEF_END
};

uint16 gCurrentTheme = 0;
uint32 gNumThemeWindows = sizeof(gThemeWindowDefinitions) / sizeof(theme_window_definition);


theme_preset* theme_get_preset()
{
	return &gConfigThemes.presets[gCurrentTheme];
}

theme_window_definition* theme_window_definition_get_by_class(rct_windowclass classification)
{
	for (int i = 0; i < (int)gNumThemeWindows; i++) {
		if (gThemeWindowDefinitions[i].classification == classification) {
			return &gThemeWindowDefinitions[i];
		}
	}
	return NULL;
}

theme_window* theme_window_get_by_class(rct_windowclass classification)
{
	for (int i = 0; i < (int)gNumThemeWindows; i++) {
		if (gThemeWindowDefinitions[i].classification == classification) {
			return &gConfigThemes.presets[gCurrentTheme].windows[i];
		}
	}
	return NULL;
}

void colour_scheme_update(rct_window *window)
{
	theme_window* theme = theme_window_get_by_class(window->classification);

	bool transparent = false;
	for (int i = 0; i < 6; i++) {
		window->colours[i] = theme->colours[i];
		if (theme->colours[i] & 0x80) {
			transparent = true;
		}
	}
	// Some windows need to be transparent even if the colours aren't.
	// There doesn't seem to be any side-effects for all windows being transparent
	window->flags |= WF_TRANSPARENT;
}

void colour_scheme_update_by_class(rct_window *window, rct_windowclass classification)
{
	theme_window* theme = theme_window_get_by_class(classification);

	bool transparent = false;
	for (int i = 0; i < 6; i++) {
		window->colours[i] = theme->colours[i];
		if (theme->colours[i] & 0x80) {
			transparent = true;
		}
	}
	// Some windows need to be transparent even if the colours aren't.
	// There doesn't seem to be any side-effects for all windows being transparent
	window->flags |= WF_TRANSPARENT;
}

void theme_change_preset(int preset)
{
	if (preset >= 0 && preset < gConfigThemes.num_presets) {
		switch (preset) {
		case 0:
			gConfigInterface.current_theme_preset = "*RCT2";
			break;
		case 1:
			gConfigInterface.current_theme_preset = "*RCT1";
			break;
		default:
			gConfigInterface.current_theme_preset = gConfigThemes.presets[preset].name;
			break;
		}
		gCurrentTheme = preset;
	}
	window_invalidate_all();
}

void theme_create_preset(int duplicate, const char *name)
{
	int preset = gConfigThemes.num_presets;
	gConfigThemes.num_presets++;
	gConfigThemes.presets = realloc(gConfigThemes.presets, sizeof(theme_preset) * gConfigThemes.num_presets);
	safe_strncpy(gConfigThemes.presets[preset].name, name, THEME_PRESET_NAME_SIZE);
	gConfigThemes.presets[preset].windows = malloc(sizeof(theme_window) * gNumThemeWindows);
	for (int i = 0; i < (int)gNumThemeWindows; i++) {
		gConfigThemes.presets[preset].windows[i] = gConfigThemes.presets[duplicate].windows[i];
	}
	gConfigThemes.presets[preset].features = gConfigThemes.presets[duplicate].features;
	themes_save_preset(preset);
	theme_change_preset(preset);
}

void theme_delete_preset(int preset)
{
	if (preset >= 2) {
		utf8 path[MAX_PATH];
		platform_get_user_directory(path, "themes");
		strcat(path, gConfigThemes.presets[preset].name);
		strcat(path, ".ini");
		platform_file_delete(path);

		free(gConfigThemes.presets[preset].windows);

		for (int i = preset; i < gConfigThemes.num_presets - 1; i++) {
			gConfigThemes.presets[i] = gConfigThemes.presets[i + 1];
		}
		gConfigThemes.num_presets--;
		theme_change_preset(0);
	}
}

void theme_rename_preset(int preset, const char *newName)
{
	if (preset >= 2) {
		utf8 src[MAX_PATH], dest[MAX_PATH];
		platform_get_user_directory(src, "themes");
		platform_get_user_directory(dest, "themes");
		strcat(src, gConfigThemes.presets[preset].name);
		strcat(dest, newName);
		strcat(src, ".ini");
		strcat(dest, ".ini");
		platform_file_move(src, dest);

		safe_strncpy(gConfigThemes.presets[preset].name, newName, THEME_PRESET_NAME_SIZE);

		if (preset == gCurrentTheme) {
			gConfigInterface.current_theme_preset = gConfigThemes.presets[preset].name;
		}
	}
}
