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
#include "../audio/audio.h"
#include "../editor.h"
#include "../localisation/localisation.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../ride/ride.h"
#include "../ride/track.h"
#include "../sprites.h"
#include "error.h"
#include "../interface/themes.h"
#include "../rct1.h"

enum {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_TRACK_LIST,
	WIDX_TRACK_PREVIEW,
	WIDX_ROTATE,
	WIDX_TOGGLE_SCENERY,
	WIDX_BACK,
};

static rct_widget window_track_list_widgets[] = {
	{ WWT_FRAME,			0,	0,		599,	0,		399,	0xFFFFFFFF,				STR_NONE								},
	{ WWT_CAPTION,			0,	1,		598,	1,		14,		STR_SELECT_DESIGN,		STR_WINDOW_TITLE_TIP					},
	{ WWT_CLOSEBOX,			0,	587,	597,	2,		13,		STR_CLOSE_X,			STR_CLOSE_WINDOW_TIP					},
	{ WWT_SCROLL,			0,	4,		221,	33,		395,	2,						STR_CLICK_ON_DESIGN_TO_BUILD_IT_TIP		},
	{ WWT_FLATBTN,			0,	224,	595,	18,		236,	0xFFFFFFFF,				STR_NONE								},
	{ WWT_FLATBTN,			0,	574,	597,	374,	397,	5169,					STR_ROTATE_90_TIP						},
	{ WWT_FLATBTN,			0,	574,	597,	350,	373,	5171,					STR_TOGGLE_SCENERY_TIP					},
	{ WWT_13,				0,	4,		221,	18,		29,		STR_SELECT_OTHER_RIDE,	STR_NONE								},
	{ WIDGETS_END },
};

static void window_track_list_close(rct_window *w);
static void window_track_list_mouseup(rct_window *w, int widgetIndex);
static void window_track_list_scrollgetsize(rct_window *w, int scrollIndex, int *width, int *height);
static void window_track_list_scrollmousedown(rct_window *w, int scrollIndex, int x, int y);
static void window_track_list_scrollmouseover(rct_window *w, int scrollIndex, int x, int y);
static void window_track_list_tooltip(rct_window* w, int widgetIndex, rct_string_id *stringId);
static void window_track_list_invalidate(rct_window *w);
static void window_track_list_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_track_list_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex);

static rct_window_event_list window_track_list_events = {
	window_track_list_close,
	window_track_list_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_track_list_scrollgetsize,
	window_track_list_scrollmousedown,
	NULL,
	window_track_list_scrollmouseover,
	NULL,
	NULL,
	NULL,
	window_track_list_tooltip,
	NULL,
	NULL,
	window_track_list_invalidate,
	window_track_list_paint,
	window_track_list_scrollpaint
};

ride_list_item _window_track_list_item;

/**
 *
 *  rct2: 0x006CF1A2
 */
void window_track_list_open(ride_list_item item)
{
	rct_window *w;
	int x, y;
	void *mem;

	window_close_construction_windows();
	_window_track_list_item = item;

	if (RCT2_GLOBAL(0x00F635ED, uint8) & 1)
		window_error_open(STR_WARNING, STR_TOO_MANY_TRACK_DESIGNS_OF_THIS_TYPE);

	mem = malloc(1285292);
	if (mem == NULL)
		return;

	RCT2_GLOBAL(RCT2_ADDRESS_TRACK_DESIGN_CACHE, void*) = mem;
	reset_track_list_cache();

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		x = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16) / 2 - 300;
		y = max(28, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16) / 2 - 200);
	} else {
		x = 0;
		y = 29;
	}
	w = window_create(
		x,
		y,
		600,
		400,
		&window_track_list_events,
		WC_TRACK_DESIGN_LIST,
		0
	);
	w->widgets = window_track_list_widgets;
	w->enabled_widgets = (1 << WIDX_CLOSE) | (1 << WIDX_ROTATE) | (1 << WIDX_TOGGLE_SCENERY) | (1 << WIDX_BACK);
	window_init_scroll_widgets(w);
	w->track_list.var_480 = 0xFFFF;
	w->track_list.var_482 = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER ? 0 : 1;
	w->track_list.var_484 = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_TRACK_DESIGN_SCENERY_TOGGLE, uint8) = 0;
	window_push_others_right(w);
	RCT2_GLOBAL(RCT2_ADDRESS_TRACK_PREVIEW_ROTATION, uint8) = 2;
}

/**
 *
 *  rct2: 0x006CFB82
 */
static void window_track_list_select(rct_window *w, int index)
{
	utf8 *trackDesignItem, *trackDesignList = RCT2_ADDRESS(RCT2_ADDRESS_TRACK_LIST, utf8);
	rct_track_design *trackDesign;

	w->track_list.var_480 = index;

	sound_play_panned(SOUND_CLICK_1, w->x + (w->width / 2), 0, 0, 0);
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) && index == 0) {
		window_close(w);
		ride_construct_new(_window_track_list_item);
		return;
	}

	if (RCT2_GLOBAL(0x00F44153, uint8) != 0)
		RCT2_GLOBAL(RCT2_ADDRESS_TRACK_DESIGN_SCENERY_TOGGLE, uint8) = 1;

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER))
		index--;

	trackDesignItem = trackDesignList + (index * 128);
	RCT2_GLOBAL(0x00F4403C, utf8*) = trackDesignItem;

	window_track_list_format_name(
		(char*)0x009BC313,
		trackDesignItem,
		RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER ?
		0 :
		FORMAT_WHITE,
		1);

	char track_path[MAX_PATH] = { 0 };
	subsitute_path(track_path, (char*)RCT2_ADDRESS_TRACKS_PATH, trackDesignItem);

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		window_track_manage_open();
		return;
	}

	if (!load_track_design(track_path)) {
		w->track_list.var_480 = 0xFFFF;
		window_invalidate(w);
		return;
	}

	trackDesign = track_get_info(index, NULL);
	if (trackDesign == NULL) return;
	if (trackDesign->track_td6.track_flags & 4)
		window_error_open(STR_THIS_DESIGN_WILL_BE_BUILT_WITH_AN_ALTERNATIVE_VEHICLE_TYPE, -1);

	window_close(w);
	window_track_place_open();
}

static int window_track_list_get_list_item_index_from_position(int x, int y)
{
	int index;
	uint8 *trackDesignItem, *trackDesignList = RCT2_ADDRESS(RCT2_ADDRESS_TRACK_LIST, uint8);

	index = 0;
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)) {
		y -= 10;
		if (y < 0)
			return index;
		index++;
	}

	for (trackDesignItem = trackDesignList; *trackDesignItem != 0; trackDesignItem += 128) {
		y -= 10;
		if (y < 0)
			return index;
		index++;
	}

	return -1;
}

/**
 *
 *  rct2: 0x006CFD76
 */
static void window_track_list_close(rct_window *w)
{
	free(RCT2_GLOBAL(RCT2_ADDRESS_TRACK_DESIGN_CACHE, void*));
}

/**
 *
 *  rct2: 0x006CFA31
 */
static void window_track_list_mouseup(rct_window *w, int widgetIndex)
{
	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
			window_close_by_number(WC_MANAGE_TRACK_DESIGN, w->number);
			window_close_by_number(WC_TRACK_DELETE_PROMPT, w->number);
			trackmanager_load();
		}
		break;
	case WIDX_ROTATE:
		RCT2_GLOBAL(RCT2_ADDRESS_TRACK_PREVIEW_ROTATION, uint8)++;
		RCT2_GLOBAL(RCT2_ADDRESS_TRACK_PREVIEW_ROTATION, uint8) %= 4;
		window_invalidate(w);
		break;
	case WIDX_TOGGLE_SCENERY:
		RCT2_GLOBAL(RCT2_ADDRESS_TRACK_DESIGN_SCENERY_TOGGLE, uint8) ^= 1;
		reset_track_list_cache();
		window_invalidate(w);
		break;
	case WIDX_BACK:
		window_close(w);
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
			window_close_by_number(WC_MANAGE_TRACK_DESIGN, w->number);
			window_close_by_number(WC_TRACK_DELETE_PROMPT, w->number);
			trackmanager_load();
		} else {
			window_new_ride_open();
		}
		break;
	}
}

/**
 *
 *  rct2: 0x006CFAB0
 */
static void window_track_list_scrollgetsize(rct_window *w, int scrollIndex, int *width, int *height)
{
	uint8 *trackDesignItem, *trackDesignList = RCT2_ADDRESS(RCT2_ADDRESS_TRACK_LIST, uint8);

	*height = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER ? 0 : 10;
	for (trackDesignItem = trackDesignList; *trackDesignItem != 0; trackDesignItem += 128)
		*height += 10;
}

/**
 *
 *  rct2: 0x006CFB39
 */
static void window_track_list_scrollmousedown(rct_window *w, int scrollIndex, int x, int y)
{
	int i;

	if (w->track_list.var_484 & 1)
		return;

	i = window_track_list_get_list_item_index_from_position(x, y);
	if (i != -1)
		window_track_list_select(w, i);
}

/**
 *
 *  rct2: 0x006CFAD7
 */
static void window_track_list_scrollmouseover(rct_window *w, int scrollIndex, int x, int y)
{
	int i;

	if (w->track_list.var_484 & 1)
		return;

	i = window_track_list_get_list_item_index_from_position(x, y);
	if (i != -1 && w->track_list.var_482 != i) {
		w->track_list.var_482 = i;
		window_invalidate(w);
	}
}

/**
 *
 *  rct2: 0x006CFD6C
 */
static void window_track_list_tooltip(rct_window* w, int widgetIndex, rct_string_id *stringId)
{
	RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint16) = STR_LIST;
}

/**
 *
 *  rct2: 0x006CF2D6
 */
static void window_track_list_invalidate(rct_window *w)
{
	rct_ride_type *entry;
	rct_string_id stringId;

	colour_scheme_update(w);

	entry = GET_RIDE_ENTRY(_window_track_list_item.entry_index);

	stringId = entry->name;
	if (!(entry->flags & RIDE_ENTRY_FLAG_SEPARATE_RIDE_NAME) || rideTypeShouldLoseSeparateFlag(entry))
		stringId = _window_track_list_item.type + 2;

	RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint16) = stringId;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		window_track_list_widgets[WIDX_TITLE].image = STR_TRACK_DESIGNS;
		window_track_list_widgets[WIDX_TRACK_LIST].tooltip = STR_CLICK_ON_DESIGN_TO_RENAME_OR_DELETE_IT;
	} else {
		window_track_list_widgets[WIDX_TITLE].image = STR_SELECT_DESIGN;
		window_track_list_widgets[WIDX_TRACK_LIST].tooltip = STR_CLICK_ON_DESIGN_TO_BUILD_IT_TIP;
	}

	if ((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) || w->track_list.var_482 != 0) {
		w->pressed_widgets |= 1 << WIDX_TRACK_PREVIEW;
		w->disabled_widgets &= ~(1 << WIDX_TRACK_PREVIEW);
		window_track_list_widgets[WIDX_ROTATE].type = WWT_FLATBTN;
		window_track_list_widgets[WIDX_TOGGLE_SCENERY].type = WWT_FLATBTN;
		if (RCT2_GLOBAL(RCT2_ADDRESS_TRACK_DESIGN_SCENERY_TOGGLE, uint8) == 0)
			w->pressed_widgets |= (1 << WIDX_TOGGLE_SCENERY);
		else
			w->pressed_widgets &= ~(1 << WIDX_TOGGLE_SCENERY);
	} else {
		w->pressed_widgets &= ~(1 << WIDX_TRACK_PREVIEW);
		w->disabled_widgets |= (1 << WIDX_TRACK_PREVIEW);
		window_track_list_widgets[WIDX_ROTATE].type = WWT_EMPTY;
		window_track_list_widgets[WIDX_TOGGLE_SCENERY].type = WWT_EMPTY;
	}
}

/**
 *
 *  rct2: 0x006CF387
 */
static void window_track_list_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	rct_widget *widget;
	rct_track_design *trackDesign = NULL;
	uint8 *image;
	utf8 *trackDesignList = RCT2_ADDRESS(RCT2_ADDRESS_TRACK_LIST, utf8);
	uint16 holes, speed, drops, dropHeight, inversions;
	fixed32_2dp rating;
	int trackIndex, x, y, colour, gForces, airTime;
	rct_g1_element tmpElement, *subsituteElement;

	window_draw_widgets(w, dpi);

	trackIndex = w->track_list.var_482;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		if (*trackDesignList == 0 || trackIndex == -1)
			return;
	} else if (trackIndex-- == 0) {
		return;
	}

	// Track preview
	widget = &window_track_list_widgets[WIDX_TRACK_PREVIEW];
	x = w->x + widget->left + 1;
	y = w->y + widget->top + 1;
	colour = ColourMapA[w->colours[0]].darkest;
	gfx_fill_rect(dpi, x, y, x + 369, y + 216, colour);

	trackDesign = track_get_info(trackIndex, &image);
	if (trackDesign == NULL)
		return;

	rct_track_td6* track_td6 = &trackDesign->track_td6;

	subsituteElement = &g1Elements[0];
	tmpElement = *subsituteElement;
	subsituteElement->offset = image;
	subsituteElement->width = 370;
	subsituteElement->height = 217;
	subsituteElement->x_offset = 0;
	subsituteElement->y_offset = 0;
	subsituteElement->flags = G1_FLAG_BMP;
	gfx_draw_sprite(dpi, 0, x, y, 0);
	*subsituteElement = tmpElement;

	x = w->x + (widget->left + widget->right) / 2;
	y = w->y + widget->bottom - 12;

	RCT2_GLOBAL(0x00F44153, uint8) = 0;
	// Warnings
	if ((track_td6->track_flags & 4) && !(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER)) {
		// Vehicle design not available
		gfx_draw_string_centred_clipped(dpi, STR_VEHICLE_DESIGN_UNAVAILABLE, NULL, 0, x, y, 368);
		y -= 10;
	}

	if (track_td6->track_flags & 1) {
		RCT2_GLOBAL(0x00F44153, uint8) = 1;
		if (RCT2_GLOBAL(RCT2_ADDRESS_TRACK_DESIGN_SCENERY_TOGGLE, uint8) == 0) {
			// Scenery not available
			gfx_draw_string_centred_clipped(dpi, STR_DESIGN_INCLUDES_SCENERY_WHICH_IS_UNAVAILABLE, NULL, 0, x, y, 368);
			y -= 10;
		}
	}

	// Track design name
	window_track_list_format_name((char*)0x009BC677, trackDesignList + (trackIndex * 128), FORMAT_WINDOW_COLOUR_1, 1);
	gfx_draw_string_centred_clipped(dpi, 3165, NULL, 0, x, y, 368);

	// Information
	x = w->x + widget->left + 1;
	y = w->y + widget->bottom + 2;

	// Stats
	rating = track_td6->excitement * 10;
	gfx_draw_string_left(dpi, STR_TRACK_LIST_EXCITEMENT_RATING, &rating, 0, x, y);
	y += 10;

	rating = track_td6->intensity * 10;
	gfx_draw_string_left(dpi, STR_TRACK_LIST_INTENSITY_RATING, &rating, 0, x, y);
	y += 10;

	rating = track_td6->nausea * 10;
	gfx_draw_string_left(dpi, STR_TRACK_LIST_NAUSEA_RATING, &rating, 0, x, y);
	y += 14;

	if (track_td6->type != RIDE_TYPE_MAZE) {
		if (track_td6->type == RIDE_TYPE_MINI_GOLF) {
			// Holes
			holes = track_td6->holes & 0x1F;
			gfx_draw_string_left(dpi, STR_HOLES, &holes, 0, x, y);
			y += 10;
		} else {
			// Maximum speed
			speed = ((track_td6->max_speed << 16) * 9) >> 18;
			gfx_draw_string_left(dpi, STR_MAX_SPEED, &speed, 0, x, y);
			y += 10;

			// Average speed
			speed = ((track_td6->average_speed << 16) * 9) >> 18;
			gfx_draw_string_left(dpi, STR_AVERAGE_SPEED, &speed, 0, x, y);
			y += 10;
		}

		// Ride length
		RCT2_GLOBAL(0x013CE952 + 0, uint16) = 1345;
		RCT2_GLOBAL(0x013CE952 + 2, uint16) = track_td6->ride_length;
		gfx_draw_string_left_clipped(dpi, STR_TRACK_LIST_RIDE_LENGTH, (void*)0x013CE952, 0, x, y, 214);
		y += 10;
	}

	if (ride_type_has_flag(track_td6->type, RIDE_TYPE_FLAG_HAS_G_FORCES)) {
		// Maximum positive vertical Gs
		gForces = track_td6->max_positive_vertical_g * 32;
		gfx_draw_string_left(dpi, STR_MAX_POSITIVE_VERTICAL_G, &gForces, 0, x, y);
		y += 10;

		// Maximum negative verical Gs
		gForces = track_td6->max_negative_vertical_g * 32;
		gfx_draw_string_left(dpi, STR_MAX_NEGATIVE_VERTICAL_G, &gForces, 0, x, y);
		y += 10;

		// Maximum lateral Gs
		gForces = track_td6->max_lateral_g * 32;
		gfx_draw_string_left(dpi, STR_MAX_LATERAL_G, &gForces, 0, x, y);
		y += 10;

		// If .TD6
		if (track_td6->version_and_colour_scheme / 4 >= 2) {
			if (track_td6->total_air_time != 0) {
				// Total air time
				airTime = track_td6->total_air_time * 25;
				gfx_draw_string_left(dpi, STR_TOTAL_AIR_TIME, &airTime, 0, x, y);
				y += 10;
			}
		}
	}

	if (ride_type_has_flag(track_td6->type, RIDE_TYPE_FLAG_HAS_DROPS)) {
		// Drops
		drops = track_td6->drops & 0x3F;
		gfx_draw_string_left(dpi, STR_DROPS, &drops, 0, x, y);
		y += 10;

		// Drop height is multiplied by 0.75
		dropHeight = (track_td6->highest_drop_height + (track_td6->highest_drop_height / 2)) / 2;
		gfx_draw_string_left(dpi, STR_HIGHEST_DROP_HEIGHT, &drops, 0, x, y);
		y += 10;
	}

	if (track_td6->type != RIDE_TYPE_MINI_GOLF) {
		inversions = track_td6->inversions & 0x1F;
		if (inversions != 0) {
			// Inversions
			gfx_draw_string_left(dpi, STR_INVERSIONS, &inversions, 0, x, y);
			y += 10;
		}
	}
	y += 4;

	if (track_td6->space_required_x != 0xFF) {
		// Space required
		RCT2_GLOBAL(0x013CE952 + 0, uint16) = track_td6->space_required_x;
		RCT2_GLOBAL(0x013CE952 + 2, uint16) = track_td6->space_required_y;
		gfx_draw_string_left(dpi, STR_TRACK_LIST_SPACE_REQUIRED, (void*)0x013CE952, 0, x, y);
		y += 10;
	}

	if (track_td6->cost != 0) {
		gfx_draw_string_left(dpi, STR_TRACK_LIST_COST_AROUND, &track_td6->cost, 0, x, y);
		y += 14;
	}
}

/**
 *
 *  rct2: 0x006CF8CD
 */
static void window_track_list_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex)
{
	rct_string_id stringId, stringId2;
	int i, x, y, colour;
	utf8 *trackDesignItem, *trackDesignList = RCT2_ADDRESS(RCT2_ADDRESS_TRACK_LIST, utf8);

	colour = ColourMapA[w->colours[0]].mid_light;
	colour = (colour << 24) | (colour << 16) | (colour << 8) | colour;
	gfx_clear(dpi, colour);

	i = 0;
	x = 0;
	y = 0;

	trackDesignItem = trackDesignList;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_MANAGER) {
		if (*trackDesignItem == 0) {
			// No track designs
			gfx_draw_string_left(dpi, STR_NO_TRACK_DESIGNS_OF_THIS_TYPE, NULL, 0, x, y - 1);
			return;
		}
	} else {
		// Build custom track item
		if (i == w->track_list.var_482) {
			// Highlight
			gfx_fill_rect(dpi, x, y, w->width, y + 9, 0x2000000 | 49);
			stringId = 1193;
		} else {
			stringId = 1191;
		}

		stringId2 = STR_BUILD_CUSTOM_DESIGN;
		gfx_draw_string_left(dpi, stringId, &stringId2, 0, x, y - 1);
		y += 10;
		i++;
	}

	while (*trackDesignItem != 0) {
		if (y + 10 >= dpi->y && y < dpi->y + dpi->height) {
			if (i == w->track_list.var_482) {
				// Highlight
				gfx_fill_rect(dpi, x, y, w->width, y + 9, 0x2000000 | 49);
				stringId = 1193;
			} else {
				stringId = 1191;
			}

			// Draw track name
			window_track_list_format_name((char *)language_get_string(3165), trackDesignItem, 0, 1);
			stringId2 = 3165;
			gfx_draw_string_left(dpi, stringId, &stringId2, 0, x, y - 1);
		}
		y += 10;
		i++;
		trackDesignItem += 128;
	}
}
