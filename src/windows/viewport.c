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
#include "../game.h"
#include "../world/map.h"
#include "../localisation/localisation.h"
#include "../sprites.h"
#include "../interface/viewport.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "dropdown.h"
#include "../interface/themes.h"

#define INITIAL_WIDTH 500
#define INITIAL_HEIGHT 350

enum {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_PAGE_BACKGROUND,
	WIDX_VIEWPORT,
	WIDX_ZOOM_IN,
	WIDX_ZOOM_OUT,
	WIDX_LOCATE
};

static rct_widget window_viewport_widgets[] = {
	{ WWT_FRAME,			0,	0,	0,	0,	0,	0xFFFFFFFF,			STR_NONE				},	// panel / background
	{ WWT_CAPTION,			0,	1,	0,	1,	14,	2779,				STR_WINDOW_TITLE_TIP	},	// title bar
	{ WWT_CLOSEBOX,			0,	0,	0,	2,	13,	0x338,				STR_CLOSE_WINDOW_TIP	},	// close x button
	{ WWT_RESIZE,			1,	0,	0,	14,	0,	0xFFFFFFFF,			STR_NONE				},	// resize
	{ WWT_VIEWPORT,			0,	3,	0,	17,	0,	0xFFFFFFFF,			STR_NONE				},	// viewport

	{ WWT_FLATBTN,			0,	0,	0,	17,	40,	SPR_G2_ZOOM_IN,		STR_ZOOM_IN_TIP			},	// zoom in
	{ WWT_FLATBTN,			0,	0,	0,	41,	64,	SPR_G2_ZOOM_OUT,	STR_ZOOM_OUT_TIP		},	// zoom out
	{ WWT_FLATBTN,			0,	0,	0,	65,	88,	SPR_LOCATE,			STR_LOCATE_SUBJECT_TIP	},	// locate
	{ WIDGETS_END },
};

static void window_viewport_mouseup(rct_window *w, int widgetIndex);
static void window_viewport_resize(rct_window *w);
static void window_viewport_update(rct_window *w);
static void window_viewport_invalidate(rct_window *w);
static void window_viewport_paint(rct_window *w, rct_drawpixelinfo *dpi);

static rct_window_event_list window_viewport_events = {
	NULL,
	window_viewport_mouseup,
	window_viewport_resize,
	NULL,
	NULL,
	NULL,
	window_viewport_update,
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
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_viewport_invalidate,
	window_viewport_paint,
	NULL
};

static int _viewportNumber = 1;

/**
 * Creates a custom viewport window.
 */
void window_viewport_open()
{
	rct_window *w, *mainWindow;
	rct_viewport *mainViewport;
	int x, y, rotation;

	w = window_create_auto_pos(
		INITIAL_WIDTH, INITIAL_HEIGHT,
		&window_viewport_events,
		WC_VIEWPORT,
		WF_RESIZABLE
	);
	w->widgets = window_viewport_widgets;
	w->enabled_widgets =
		(1 << WIDX_CLOSE) |
		(1 << WIDX_ZOOM_IN) |
		(1 << WIDX_ZOOM_OUT) |
		(1 << WIDX_LOCATE);
	w->number = _viewportNumber++;

	rotation = get_current_rotation();

	// Create viewport
	viewport_create(w, w->x, w->y, w->width, w->height, 0, 128 * 32, 128 * 32, 0, 1, -1);
	mainWindow = window_get_main();
	if (mainWindow != NULL) {
		mainViewport = mainWindow->viewport;
		x = mainViewport->view_x + (mainViewport->view_width / 2);
		y = mainViewport->view_y + (mainViewport->view_height / 2);
		w->saved_view_x = x - (w->viewport->view_width / 2);
		w->saved_view_y = y - (w->viewport->view_height / 2);
	}

	w->viewport->flags |= VIEWPORT_FLAG_SOUND_ON;
}

static void window_viewport_anchor_border_widgets(rct_window *w)
{
	w->widgets[WIDX_BACKGROUND].right = w->width - 1;
	w->widgets[WIDX_BACKGROUND].bottom = w->height - 1;
	w->widgets[WIDX_PAGE_BACKGROUND].right = w->width - 1;
	w->widgets[WIDX_PAGE_BACKGROUND].bottom = w->height - 1;
	w->widgets[WIDX_TITLE].right = w->width - 2;
	w->widgets[WIDX_CLOSE].left = w->width - 13;
	w->widgets[WIDX_CLOSE].right = w->width - 3;
}

static void window_viewport_mouseup(rct_window *w, int widgetIndex)
{
	rct_window *mainWindow;
	sint16 x, y;

	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_ZOOM_IN:
		if (w->viewport != NULL && w->viewport->zoom > 0) {
			w->viewport->zoom--;
			window_invalidate(w);
		}
		break;
	case WIDX_ZOOM_OUT:
		if (w->viewport != NULL && w->viewport->zoom < 3) {
			w->viewport->zoom++;
			window_invalidate(w);
		}
		break;
	case WIDX_LOCATE:
		mainWindow = window_get_main();
		if (mainWindow != NULL) {
			get_map_coordinates_from_pos(w->x + (w->width / 2), w->y + (w->height / 2), VIEWPORT_INTERACTION_MASK_NONE, &x, &y, NULL, NULL, NULL);
			window_scroll_to_location(mainWindow, x, y, map_element_height(x, y));
		}
		break;
	}
}

static void window_viewport_resize(rct_window *w)
{
	w->flags |= WF_RESIZABLE;
	window_set_resize(w, 200, 200, 2000, 2000);
}

static void window_viewport_update(rct_window *w)
{
	rct_window *mainWindow;

	mainWindow = window_get_main();
	if (mainWindow == NULL)
		return;

	if (w->viewport->flags != mainWindow->viewport->flags) {
		w->viewport->flags = mainWindow->viewport->flags;
		window_invalidate(w);
	}

	// Not sure how to invalidate part of the viewport that has changed, this will have to do for now
	//widget_invalidate(w, WIDX_VIEWPORT);
}

static void window_viewport_invalidate(rct_window *w)
{
	rct_widget *viewportWidget;
	rct_viewport *viewport;
	int i;

	colour_scheme_update(w);

	viewportWidget = &window_viewport_widgets[WIDX_VIEWPORT];
	viewport = w->viewport;

	// Anchor widgets
	window_viewport_anchor_border_widgets(w);
	viewportWidget->right = w->width - 26;
	viewportWidget->bottom = w->height - 3;
	for (i = WIDX_ZOOM_IN; i <= WIDX_LOCATE; i++) {
		window_viewport_widgets[i].left = w->width - 25;
		window_viewport_widgets[i].right = w->width - 2;
	}

	// Set title
	RCT2_GLOBAL(0x013CE952 + 0, uint32) = w->number;

	// Set disabled widgets
	w->disabled_widgets = 0;
	if (viewport->zoom == 0)
		w->disabled_widgets |= 1 << WIDX_ZOOM_IN;
	if (viewport->zoom >= 3)
		w->disabled_widgets |= 1 << WIDX_ZOOM_OUT;

	viewport->x = w->x + viewportWidget->left;
	viewport->y = w->y + viewportWidget->top;
	viewport->width = viewportWidget->right - viewportWidget->left;
	viewport->height = viewportWidget->bottom - viewportWidget->top;
	viewport->view_width = viewport->width << viewport->zoom;
	viewport->view_height = viewport->height << viewport->zoom;
}

static void window_viewport_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	window_draw_widgets(w, dpi);

	// Draw viewport
	if (w->viewport != NULL)
		window_draw_viewport(dpi, w);
}
