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
#include "../input.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "../sprites.h"
#include "../world/map.h"
#include "../interface/themes.h"

#define MINIMUM_TOOL_SIZE 1
#define MAXIMUM_TOOL_SIZE 64

enum WINDOW_WATER_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_PREVIEW,
	WIDX_DECREMENT,
	WIDX_INCREMENT
};

static rct_widget window_water_widgets[] = {
	{ WWT_FRAME,	0,	0,	75,	0,	76,	-1,										STR_NONE },							// panel / background
	{ WWT_CAPTION,	0,	1,	74,	1,	14,	STR_WATER,								STR_WINDOW_TITLE_TIP },				// title bar
	{ WWT_CLOSEBOX,	0,	63,	73,	2,	13,	824,									STR_CLOSE_WINDOW_TIP },				// close x button
	{ WWT_IMGBTN,	0,	16,	59,	17,	48,	SPR_LAND_TOOL_SIZE_0,					STR_NONE },							// preview box
	{ WWT_TRNBTN,	2,	17,	32,	18,	33,	0x20000000 | SPR_LAND_TOOL_DECREASE,	STR_ADJUST_SMALLER_WATER_TIP },		// decrement size
	{ WWT_TRNBTN,	2,	43,	58,	32,	47,	0x20000000 | SPR_LAND_TOOL_INCREASE,	STR_ADJUST_LARGER_WATER_TIP },		// increment size
	{ WIDGETS_END },
};

static int window_water_should_close();

static void window_water_close(rct_window *w);
static void window_water_mouseup(rct_window *w, int widgetIndex);
static void window_water_update(rct_window *w);
static void window_water_invalidate(rct_window *w);
static void window_water_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_water_textinput(rct_window *w, int widgetIndex, char *text);
static void window_water_inputsize(rct_window *w);

static rct_window_event_list window_water_events = {
	window_water_close,
	window_water_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	window_water_update,
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
	window_water_textinput,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_water_invalidate,
	window_water_paint,
	NULL
};

/**
 *
 *  rct2: 0x006E6A40
 */
void window_water_open()
{
	rct_window* window;

	// Check if window is already open
	if (window_find_by_class(WC_WATER) != NULL)
		return;

	window = window_create(
		RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16) - 76,
		29,
		76,
		77,
		&window_water_events,
		WC_WATER,
		0
	);
	window->widgets = window_water_widgets;
	window->enabled_widgets = (1 << WIDX_CLOSE) | (1 << WIDX_DECREMENT) | (1 << WIDX_INCREMENT) | (1 << WIDX_PREVIEW);
	window_init_scroll_widgets(window);
	window_push_others_below(window);

	RCT2_GLOBAL(RCT2_ADDRESS_WATER_RAISE_COST, uint32) = MONEY32_UNDEFINED;
	RCT2_GLOBAL(RCT2_ADDRESS_WATER_LOWER_COST, uint32) = MONEY32_UNDEFINED;
}

/**
 *
 *  rct2: 0x006E6B65
 */
static void window_water_close(rct_window *w)
{
	// If the tool wasn't changed, turn tool off
	if (!window_water_should_close())
		tool_cancel();
}

/**
 *
 *  rct2: 0x006E6B4E
 */
static void window_water_mouseup(rct_window *w, int widgetIndex)
{
	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_DECREMENT:
		// Decrement land tool size
		RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) = max(MINIMUM_TOOL_SIZE, RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16)-1);

		// Invalidate the window
		window_invalidate(w);
		break;
	case WIDX_INCREMENT:
		// Increment land tool size
		RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) = min(MAXIMUM_TOOL_SIZE, RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16)+1);

		// Invalidate the window
		window_invalidate(w);
		break;
	case WIDX_PREVIEW:
		window_water_inputsize(w);
		break;
	}
}

static void window_water_textinput(rct_window *w, int widgetIndex, char *text)
{
	int size;
	char* end;

	if (widgetIndex != WIDX_PREVIEW || text == NULL)
		return;

	size = strtol(text, &end, 10);
	if (*end == '\0') {
		size = max(MINIMUM_TOOL_SIZE,size);
		size = min(MAXIMUM_TOOL_SIZE,size);
		RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) = size;

		window_invalidate(w);
	}
}

static void window_water_inputsize(rct_window *w)
{
	((uint16*)TextInputDescriptionArgs)[0] = MINIMUM_TOOL_SIZE;
	((uint16*)TextInputDescriptionArgs)[1] = MAXIMUM_TOOL_SIZE;
	window_text_input_open(w, WIDX_PREVIEW, 5128, 5129, STR_NONE, STR_NONE, 3);
}

/**
 *
 *  rct2: 0x006E6BCE
 */
static void window_water_update(rct_window *w)
{
	// Close window if another tool is open
	if (window_water_should_close())
		window_close(w);
}

/**
 *
 *  rct2: 0x006E6AB8
 */
static void window_water_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	// Set the preview image button to be pressed down
	w->pressed_widgets |= (1 << WIDX_PREVIEW);

	// Update the preview image
	//window_water_widgets[WIDX_PREVIEW].image = SPR_LAND_TOOL_SIZE_0 + RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16);

	window_water_widgets[WIDX_PREVIEW].image = RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) <= 7 ?
		SPR_LAND_TOOL_SIZE_0 + RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) :
		0xFFFFFFFF;

}

/**
 *
 *  rct2: 0x006E6ACF
 */
static void window_water_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	int x, y;

	x = w->x + (window_water_widgets[WIDX_PREVIEW].left + window_water_widgets[WIDX_PREVIEW].right) / 2;
	y = w->y + (window_water_widgets[WIDX_PREVIEW].top + window_water_widgets[WIDX_PREVIEW].bottom) / 2;

	window_draw_widgets(w, dpi);
	// Draw number for tool sizes bigger than 7
	if (RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16) > 7) {
		gfx_draw_string_centred(dpi, STR_LAND_TOOL_SIZE_VALUE, x, y - 2, 0, &RCT2_GLOBAL(RCT2_ADDRESS_LAND_TOOL_SIZE, sint16));
	}
	y = w->y + window_water_widgets[WIDX_PREVIEW].bottom + 5;


	// Draw raise cost amount
	x = (window_water_widgets[WIDX_PREVIEW].left + window_water_widgets[WIDX_PREVIEW].right) / 2 + w->x;
	y = window_water_widgets[WIDX_PREVIEW].bottom + w->y + 5;
	if (RCT2_GLOBAL(RCT2_ADDRESS_WATER_RAISE_COST, uint32) != MONEY32_UNDEFINED && RCT2_GLOBAL(RCT2_ADDRESS_WATER_RAISE_COST, uint32) != 0)
		gfx_draw_string_centred(dpi, 984, x, y, 0, (void*)RCT2_ADDRESS_WATER_RAISE_COST);
	y += 10;

	// Draw lower cost amount
	if (RCT2_GLOBAL(RCT2_ADDRESS_WATER_LOWER_COST, uint32) != MONEY32_UNDEFINED && RCT2_GLOBAL(RCT2_ADDRESS_WATER_LOWER_COST, uint32) != 0)
		gfx_draw_string_centred(dpi, 985, x, y, 0, (void*)RCT2_ADDRESS_WATER_LOWER_COST);


}

/**
 *
 *  rct2: 0x0066D125
 */
static int window_water_should_close()
{
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & INPUT_FLAG_TOOL_ACTIVE))
		return 1;
	if (RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WINDOWCLASS, rct_windowclass) != WC_TOP_TOOLBAR)
		return 1;
	if (RCT2_GLOBAL(RCT2_ADDRESS_TOOL_WIDGETINDEX, uint16) != 8)
		return 1;
	return 0;
}
