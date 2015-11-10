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

#include "../interface/themes.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "../network/network.h"
#include "../sprites.h"
#include "../windows/dropdown.h"
#include "../util/util.h"

enum WINDOW_PLAYER_LIST_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_CONTENT_PANEL,
	WIDX_TAB1,
	WIDX_LIST,
};

static rct_widget window_player_list_widgets[] = {
	{ WWT_FRAME,			0,	0,		339,	0,		239,	0x0FFFFFFFF,				STR_NONE },									// panel / background
	{ WWT_CAPTION,			0,	1,		338,	1,		14,		STR_PLAYER_LIST,			STR_WINDOW_TITLE_TIP },						// title bar
	{ WWT_CLOSEBOX,			0,	327,	337,	2,		13,		STR_CLOSE_X,				STR_CLOSE_WINDOW_TIP },						// close x button
	{ WWT_RESIZE,			1,	0,		339,	43,		239,	0x0FFFFFFFF,				STR_NONE },									// content panel
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x02000144E,				STR_NONE },									// tab
	{ WWT_SCROLL,			1,	3,		336,	60,		236,	2,							STR_NONE },									// list
	{ WIDGETS_END },
};

static void window_player_list_mouseup(rct_window *w, int widgetIndex);
static void window_player_list_resize(rct_window *w);
static void window_player_list_mousedown(int widgetIndex, rct_window* w, rct_widget* widget);
static void window_player_list_dropdown(rct_window *w, int widgetIndex, int dropdownIndex);
static void window_player_list_update(rct_window *w);
static void window_player_list_scrollgetsize(rct_window *w, int scrollIndex, int *width, int *height);
static void window_player_list_scrollmousedown(rct_window *w, int scrollIndex, int x, int y);
static void window_player_list_scrollmouseover(rct_window *w, int scrollIndex, int x, int y);
static void window_player_list_invalidate(rct_window *w);
static void window_player_list_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_player_list_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex);

static rct_window_event_list window_player_list_events = {
	NULL,
	window_player_list_mouseup,
	window_player_list_resize,
	window_player_list_mousedown,
	window_player_list_dropdown,
	NULL,
	window_player_list_update,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_player_list_scrollgetsize,
	window_player_list_scrollmousedown,
	NULL,
	window_player_list_scrollmouseover,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_player_list_invalidate,
	window_player_list_paint,
	window_player_list_scrollpaint
};

enum {
	DDIDX_KICK
};

static void window_player_list_refresh_list(rct_window *w);

static int _dropdownPlayerId;

void window_player_list_open()
{
	rct_window* window;

	// Check if window is already open
	window = window_bring_to_front_by_class(WC_PLAYER_LIST);
	if (window != NULL)
		return;

	window = window_create_auto_pos(320, 144, &window_player_list_events, WC_PLAYER_LIST, WF_10 | WF_RESIZABLE);

	window->widgets = window_player_list_widgets;
	window->enabled_widgets = 1 << WIDX_CLOSE;
	window_init_scroll_widgets(window);
	window->no_list_items = 0;
	window->selected_list_item = -1;
	window->frame_no = 0;
	window->min_width = 320;
	window->min_height = 124;
	window->max_width = 500;
	window->max_height = 450;

	window->page = 0;
	window->list_information_type = 0;
	window->colours[0] = 7;
	window->colours[1] = 7;
	window->colours[2] = 7;
}

static void window_player_list_mouseup(rct_window *w, int widgetIndex)
{
	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	}
}

static void window_player_list_resize(rct_window *w)
{
	if (w->width < w->min_width) {
		window_invalidate(w);
		w->width = w->min_width;
	}
	if (w->height < w->min_height) {
		window_invalidate(w);
		w->height = w->min_height;
	}

	window_player_list_refresh_list(w);
}

static void window_player_list_mousedown(int widgetIndex, rct_window* w, rct_widget* widget)
{
	switch (widgetIndex) {
	case WIDX_TAB1:
		if (w->page != widgetIndex - WIDX_TAB1) {
			w->page = widgetIndex - WIDX_TAB1;
			w->no_list_items = 0;
			w->frame_no = 0;
			w->selected_list_item = -1;
			window_invalidate(w);
		}
		break;
	}
}

static void window_player_list_dropdown(rct_window *w, int widgetIndex, int dropdownIndex)
{
	switch (dropdownIndex) {
	case DDIDX_KICK:
		network_kick_player(_dropdownPlayerId);
		break;
	}
}

static void window_player_list_update(rct_window *w)
{
	widget_invalidate(w, WIDX_TAB1 + w->page);
}

static void window_player_list_scrollgetsize(rct_window *w, int scrollIndex, int *width, int *height)
{
	int i;

	if (w->selected_list_item != -1) {
		w->selected_list_item = -1;
		window_invalidate(w);
	}

	*height = network_get_num_players() * 10;
	i = *height - window_player_list_widgets[WIDX_LIST].bottom + window_player_list_widgets[WIDX_LIST].top + 21;
	if (i < 0)
		i = 0;
	if (i < w->scrolls[0].v_top) {
		w->scrolls[0].v_top = i;
		window_invalidate(w);
	}
}

static void window_player_list_scrollmousedown(rct_window *w, int scrollIndex, int x, int y)
{
	if (network_get_mode() != NETWORK_MODE_SERVER) {
		return;
	}

	int index;

	index = y / 10;
	if (index >= w->no_list_items)
		return;

	w->selected_list_item = index;
	window_invalidate(w);

	rct_widget *listWidget = &w->widgets[WIDX_LIST];
	int ddx = w->x + listWidget->left + x;
	int ddy = w->y + listWidget->top + y;

	if (index == 0) {
		return;
	}
	_dropdownPlayerId = network_get_player_id(index);
	if (_dropdownPlayerId == network_get_current_player_id()) {
		return;
	}

	gDropdownItemsFormat[0] = STR_KICK_PLAYER;
	window_dropdown_show_text(ddx, ddy, 0, 7, 0, 1);
}

static void window_player_list_scrollmouseover(rct_window *w, int scrollIndex, int x, int y)
{
	int index;

	index = y / 10;
	if (index >= w->no_list_items)
		return;

	w->selected_list_item = index;
	window_invalidate(w);
}

static void window_player_list_invalidate(rct_window *w)
{
	for (int i = 0; i < 1; i++)
		w->pressed_widgets &= ~(1 << (WIDX_TAB1 + i));
	w->pressed_widgets |= 1LL << (WIDX_TAB1 + w->page);
	window_player_list_widgets[WIDX_BACKGROUND].right = w->width - 1;
	window_player_list_widgets[WIDX_BACKGROUND].bottom = w->height - 1;
	window_player_list_widgets[WIDX_CONTENT_PANEL].right = w->width - 1;
	window_player_list_widgets[WIDX_CONTENT_PANEL].bottom = w->height - 1;
	window_player_list_widgets[WIDX_TITLE].right = w->width - 2;
	window_player_list_widgets[WIDX_CLOSE].left = w->width - 2 - 0x0B;
	window_player_list_widgets[WIDX_CLOSE].right = w->width - 2 - 0x0B + 0x0A;
	window_player_list_widgets[WIDX_LIST].right = w->width - 4;
	window_player_list_widgets[WIDX_LIST].bottom = w->height - 0x0F;
}

static void window_player_list_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	window_draw_widgets(w, dpi);
	gfx_draw_string_left(dpi, STR_PLAYER, w, w->colours[2], w->x + 6, 58 - 12 + w->y + 1);
	gfx_draw_string_left(dpi, STR_PING, w, w->colours[2], w->x + 246, 58 - 12 + w->y + 1);

	gfx_draw_sprite(dpi, SPR_TAB_GUESTS_0, w->x + w->widgets[WIDX_TAB1].left, w->y + w->widgets[WIDX_TAB1].top, 0);
}

static void window_player_list_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex)
{
	int y;

	gfx_fill_rect(dpi, dpi->x, dpi->y, dpi->x + dpi->width - 1, dpi->y + dpi->height - 1, ColourMapA[w->colours[1]].mid_light);

	y = 0;
	for (int i = 0; i < network_get_num_players(); i++) {
		if (y > dpi->y + dpi->height) {
			break;
		}

		if (y + 11 >= dpi->y) {
			char buffer[300];
			char* lineCh = buffer;
			int colour = 0;
			if (i == w->selected_list_item) {
				gfx_fill_rect(dpi, 0, y, 800, y + 9, 0x02000031);
				safe_strncpy(&buffer[0], network_get_player_name(i), sizeof(buffer));
				colour = w->colours[2];
			} else {
				if (network_get_player_flags(i) & NETWORK_PLAYER_FLAG_ISSERVER) {
					lineCh = utf8_write_codepoint(lineCh, FORMAT_BABYBLUE);
				} else {
					lineCh = utf8_write_codepoint(lineCh, FORMAT_BLACK);
				}
				safe_strncpy(lineCh, network_get_player_name(i), sizeof(buffer) - (lineCh - buffer));
			}
			gfx_clip_string(buffer, 230);
			gfx_draw_string(dpi, buffer, colour, 0, y - 1);
			lineCh = buffer;
			int ping = network_get_player_ping(i);
			if (ping <= 100) {
				lineCh = utf8_write_codepoint(lineCh, FORMAT_GREEN);
			} else
			if (ping <= 250) {
				lineCh = utf8_write_codepoint(lineCh, FORMAT_YELLOW);
			} else {
				lineCh = utf8_write_codepoint(lineCh, FORMAT_RED);
			}
			sprintf(lineCh, "%d ms", ping);
			gfx_draw_string(dpi, buffer, colour, 240, y - 1);
		}
		y += 10;
	}
}

static void window_player_list_refresh_list(rct_window *w)
{
	w->no_list_items = network_get_num_players();
	w->list_item_positions[0] = 0;

	w->selected_list_item = -1;
	window_invalidate(w);
}
