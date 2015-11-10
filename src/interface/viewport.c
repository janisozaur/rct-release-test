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
#include "../config.h"
#include "../drawing/drawing.h"
#include "../localisation/localisation.h"
#include "../ride/ride_data.h"
#include "../ride/track_data.h"
#include "../ride/track_paint.h"
#include "../sprites.h"
#include "../world/map.h"
#include "../world/sprite.h"
#include "../world/banner.h"
#include "../world/entrance.h"
#include "../world/footpath.h"
#include "../world/scenery.h"
#include "colour.h"
#include "viewport.h"
#include "window.h"

#define RCT2_FIRST_VIEWPORT		(RCT2_ADDRESS(RCT2_ADDRESS_VIEWPORT_LIST, rct_viewport))
#define RCT2_LAST_VIEWPORT		(RCT2_ADDRESS(RCT2_ADDRESS_ACTIVE_VIEWPORT_PTR_ARRAY, rct_viewport) - 1)
#define RCT2_NEW_VIEWPORT		(RCT2_GLOBAL(RCT2_ADDRESS_ACTIVE_VIEWPORT_PTR_ARRAY, rct_viewport*))

//#define DEBUG_SHOW_DIRTY_BOX

rct_viewport* g_viewport_list = RCT2_ADDRESS(RCT2_ADDRESS_VIEWPORT_LIST, rct_viewport);

typedef struct paint_struct paint_struct;

struct paint_struct{
	uint32 image_id;		// 0x00
	uint32 var_04;
	uint16 attached_x;		// 0x08
	uint16 attached_y;		// 0x0A
	union {
		struct {
			uint8 var_0C;
			uint8 pad_0D;
			paint_struct* next_attached_ps;	//0x0E
			uint16 pad_12;
		};
		struct {
			uint16 some_x; // 0x0C
			uint16 some_y; // 0x0E
			uint16 other_x; // 0x10
			uint16 other_y; // 0x12
		};
	};
	uint16 x;				// 0x14
	uint16 y;				// 0x16
	uint16 var_18;
	uint8 var_1A;
	uint8 var_1B;
	paint_struct* attached_ps;	//0x1C
	paint_struct* var_20;
	paint_struct* next_quadrant_ps; // 0x24
	uint8 sprite_type;		//0x28
	uint8 var_29;
	uint16 pad_2A;
	uint16 map_x;			// 0x2C
	uint16 map_y;			// 0x2E
	rct_map_element *mapElement; // 0x30 (or sprite pointer)
};

/**
 *  This is not a viewport function. It is used to setup many variables for
 *  multiple things.
 *  rct2: 0x006E6EAC
 */
void viewport_init_all()
{
	colours_init_maps();

	// Setting up windows
	RCT2_GLOBAL(RCT2_ADDRESS_NEW_WINDOW_PTR, rct_window*) = g_window_list;
	RCT2_GLOBAL(0x01423604, sint32) = 0;

	// Setting up viewports
	for (int i = 0; i < 9; i++) {
		g_viewport_list[i].width = 0;
	}
	RCT2_NEW_VIEWPORT = NULL;

	// ?
	RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, sint32) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_INPUT_STATE, sint8) = INPUT_STATE_RESET;
	RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_DOWN_WINDOWCLASS, rct_windowclass) = -1;
	RCT2_GLOBAL(RCT2_ADDRESS_PICKEDUP_PEEP_SPRITE, sint32) = -1;
	RCT2_GLOBAL(RCT2_ADDRESS_TOOLTIP_NOT_SHOWN_TICKS, sint16) = -1;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_FLAGS, sint16) = 0;
	RCT2_GLOBAL(0x009DEA50, sint16) = -1;
	textinput_cancel();
	format_string((char*)0x0141FA44, STR_CANCEL, NULL);
	format_string((char*)0x0141F944, STR_OK, NULL);
}

/**
 *  rct:0x006EB0C1
 *  x : ax
 *  y : bx
 *  z : cx
 *  out_x : ax
 *  out_y : bx
 *  Converts between 3d point of a sprite to 2d coordinates for centering on that sprite
 */
void center_2d_coordinates(int x, int y, int z, int* out_x, int* out_y, rct_viewport* viewport){
	int start_x = x;

	rct_xyz16 coord_3d = {
		.x = x,
		.y = y,
		.z = z
	};

	rct_xy16 coord_2d = coordinate_3d_to_2d(&coord_3d, get_current_rotation());

	// If the start location was invalid
	// propagate the invalid location to the output.
	// This fixes a bug that caused the game to enter an infinite loop.
	if (start_x == (sint16)0x8000){
		*out_x = (sint16)0x8000;
		*out_y = 0;
		return;
	}

	*out_x = coord_2d.x - viewport->view_width / 2;
	*out_y = coord_2d.y - viewport->view_height / 2;
}

/**
*
*  rct2: 0x006EB009
*  x:      ax
*  y:      eax (top 16)
*  width:  bx
*  height: ebx (top 16)
*  zoom:    cl (8 bits)
*  center_x: edx lower 16 bits
*  center_y: edx upper 16 bits
*  center_z: ecx upper 16 bits
*  sprite: edx lower 16 bits
*  flags:  edx top most 2 bits 0b_X1 for zoom clear see below for 2nd bit.
*  w:      esi
*
*  Viewport will look at sprite or at coordinates as specified in flags 0b_1X for sprite 0b_0X for coordinates
*/
void viewport_create(rct_window *w, int x, int y, int width, int height, int zoom, int center_x, int center_y, int center_z, char flags, sint16 sprite)
{
	rct_viewport* viewport;
	for (viewport = g_viewport_list; viewport->width != 0; viewport++){
		if (viewport > RCT2_LAST_VIEWPORT){
			error_string_quit(0xFF000001, -1);
		}
	}

	viewport->x = x;
	viewport->y = y;
	viewport->width = width;
	viewport->height = height;

	if (!(flags & VIEWPORT_FOCUS_TYPE_COORDINATE)){
		zoom = 0;
	}

	viewport->view_width = width << zoom;
	viewport->view_height = height << zoom;
	viewport->zoom = zoom;
	viewport->flags = 0;

	if (gConfigGeneral.always_show_gridlines)
		viewport->flags |= VIEWPORT_FLAG_GRIDLINES;
	w->viewport = viewport;

	if (flags & VIEWPORT_FOCUS_TYPE_SPRITE){
		w->viewport_target_sprite = sprite;
		rct_sprite* center_sprite = &g_sprite_list[sprite];
		center_x = center_sprite->unknown.x;
		center_y = center_sprite->unknown.y;
		center_z = center_sprite->unknown.z;
	}
	else{
		w->viewport_target_sprite = SPR_NONE;
	}

	int view_x, view_y;
	center_2d_coordinates(center_x, center_y, center_z, &view_x, &view_y, viewport);

	w->saved_view_x = view_x;
	w->saved_view_y = view_y;
	viewport->view_x = view_x;
	viewport->view_y = view_y;

	viewport_update_pointers();
}

/**
 *
 *  rct2: 0x006EE510
 */
void viewport_update_pointers()
{
	rct_viewport *viewport;
	rct_viewport **vp = RCT2_ADDRESS(RCT2_ADDRESS_ACTIVE_VIEWPORT_PTR_ARRAY, rct_viewport*);

	// The last possible viewport entry is 1 before what is the active viewport_ptr_array
	for (viewport = g_viewport_list; viewport <= RCT2_LAST_VIEWPORT; viewport++)
		if (viewport->width != 0)
			*vp++ = viewport;

	*vp = NULL;
}

/**
 * edx is assumed to be (and always is) the current rotation, so it is not needed as parameter.
 * rct2: 0x00689174
 */
void sub_689174(sint16* x, sint16* y, sint16 *z)
{
	sint16 start_x = *x;
	sint16 start_y = *y;
	sint16 height = 0;

	rct_xy16 pos;
	for (int i = 0; i < 6; i++) {
		pos = viewport_coord_to_map_coord(start_x, start_y, height);
		height = map_element_height((0xFFFF) & pos.x, (0xFFFF) & pos.y);

		// HACK: This is to prevent the x and y values being set to values outside
		// of the map. This can happen when the height is larger than the map size.
		sint16 max = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_MINUS_2, sint16);
		if (pos.x > max && pos.y > max) {
			int x_corr[] = { -1, 1, 1, -1 };
			int y_corr[] = { -1, -1, 1, 1 };
			uint32 rotation = get_current_rotation();
			pos.x += x_corr[rotation] * height;
			pos.y += y_corr[rotation] * height;
		}
	}

	*x = pos.x;
	*y = pos.y;
	*z = height;
}

void sub_683326(int left, int top, int right, int bottom)
{
	RCT2_CALLPROC_X(0x00683359, left, top, right, bottom, 0, 0, 0);
}

/**
 * shifts pixels from the region in a direction. Used when a viewport moves;
 * consider putting in src/drawing/drawing.c or src/drawing/rect.c
 * 
 * rct2: 0x00683359
 * ax = x
 * bx = y;
 * cx = width;
 * dx = height;
 * di = dx;
 * si = dy;
 */
void gfx_move_screen_rect(int x, int y, int width, int height, int dx, int dy)
{
	// nothing to do
	if (dx == 0 && dy == 0)
		return;

	// get screen info
	rct_drawpixelinfo *screenDPI = RCT2_ADDRESS(RCT2_ADDRESS_SCREEN_DPI, rct_drawpixelinfo);

	// adjust for move off screen
	// NOTE: when zooming, there can be x, y, dx, dy combinations that go off the 
	// screen; hence the checks. This code should ultimately not be called when
	// zooming because this function is specific to updating the screen on move
	int lmargin = min(x - dx, 0);
	int rmargin = min(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16) - (x - dx + width), 0);
	int tmargin = min(y - dy, 0);
	int bmargin = min(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16) - (y - dy + height), 0);
	x -= lmargin;
	y -= tmargin;
	width  += lmargin + rmargin;
	height += tmargin + bmargin;

	sint32 stride = screenDPI->width + screenDPI->pitch;
	uint8* to   = screenDPI->bits + y * stride + x;
	uint8* from = screenDPI->bits + (y - dy) * stride + x - dx;

	if (dy > 0)
	{
		// if positive dy, reverse directions
		to   += (height - 1) * stride;
		from += (height - 1) * stride;
		stride = -stride;
	}

	// move bits
	for (int i = 0; i < height; i++, to += stride, from += stride)
		memmove(to, from, width);
}

void sub_6E7FF3(rct_window *window, rct_viewport *viewport, int x, int y)
{
	// sub-divide by intersecting windows
	if (window < RCT2_GLOBAL(RCT2_ADDRESS_NEW_WINDOW_PTR, rct_window*))
	{
		// skip current window and non-intersecting windows
		if (viewport == window->viewport                                ||
			viewport->x + viewport->width  <= window->x                 ||
			viewport->x                    >= window->x + window->width ||
			viewport->y + viewport->height <= window->y                 ||
			viewport->y                    >= window->y + window->height){
			sub_6E7FF3(window + 1, viewport, x, y);
			return;
		}

		// save viewport
		rct_viewport view_copy;
		memcpy(&view_copy, viewport, sizeof(rct_viewport));

		if (viewport->x < window->x)
		{
			viewport->width = window->x - viewport->x;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);

			viewport->x += viewport->width;
			viewport->view_x += viewport->width << viewport->zoom;
			viewport->width = view_copy.width - viewport->width;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);
		}
		else if (viewport->x + viewport->width > window->x + window->width)
		{
			viewport->width = window->x + window->width - viewport->x;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);

			viewport->x += viewport->width;
			viewport->view_x += viewport->width << viewport->zoom;
			viewport->width = view_copy.width - viewport->width;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);
		}
		else if (viewport->y < window->y)
		{
			viewport->height = window->y - viewport->y;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);

			viewport->y += viewport->height;
			viewport->view_y += viewport->height << viewport->zoom;
			viewport->height = view_copy.height - viewport->height;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);
		}
		else if (viewport->y + viewport->height > window->y + window->height)
		{
			viewport->height = window->y + window->height - viewport->y;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);

			viewport->y += viewport->height;
			viewport->view_y += viewport->height << viewport->zoom;
			viewport->height = view_copy.height - viewport->height;
			viewport->view_width = viewport->width << viewport->zoom;
			sub_6E7FF3(window, viewport, x, y);
		}

		// restore viewport
		memcpy(viewport, &view_copy, sizeof(rct_viewport));
	}
	else
	{
		sint16 left = viewport->x;
		sint16 right = viewport->x + viewport->width;
		sint16 top = viewport->y;
		sint16 bottom = viewport->y + viewport->height;

		// if moved more than the viewport size
		if (abs(x) < viewport->width && abs(y) < viewport->height)
		{
			// update whole block ?
			gfx_move_screen_rect(viewport->x, viewport->y, viewport->width, viewport->height, x, y);

			if (x > 0)
			{
				// draw left
				sint16 _right = viewport->x + x;
				gfx_redraw_screen_rect(left, top, _right, bottom);
				left += x;
			}
			else if (x < 0)
			{
				// draw right
				sint16 _left = viewport->x + viewport->width + x;
				gfx_redraw_screen_rect(_left, top, right, bottom);
				right += x;
			}

			if (y > 0)
			{
				// draw top
				bottom = viewport->y + y;
				gfx_redraw_screen_rect(left, top, right, bottom);
			}
			else if (y < 0)
			{
				// draw bottom
				top = viewport->y + viewport->height + y;
				gfx_redraw_screen_rect(left, top, right, bottom);
			}
		}
		else
		{
			// redraw whole viewport
			gfx_redraw_screen_rect(left, top, right, bottom);
		}
	}
}

void sub_6E7F34(rct_window* w, rct_viewport* viewport, sint16 x_diff, sint16 y_diff){
	rct_window* orignal_w = w;
	int left = 0, right = 0, top = 0, bottom = 0;

	for (; w < RCT2_GLOBAL(RCT2_ADDRESS_NEW_WINDOW_PTR, rct_window*); w++){
		if (!(w->flags & WF_TRANSPARENT)) continue;
		if (w->viewport == viewport) continue;

		if (viewport->x + viewport->width <= w->x)continue;
		if (w->x + w->width <= viewport->x) continue;

		if (viewport->y + viewport->height <= w->y)continue;
		if (w->y + w->height <= viewport->y) continue;

		left = w->x;
		right = w->x + w->width;
		top = w->y;
		bottom = w->y + w->height;

		if (left < viewport->x)left = viewport->x;
		if (right > viewport->x + viewport->width) right = viewport->x + viewport->width;

		if (top < viewport->y)top = viewport->y;
		if (bottom > viewport->y + viewport->height) bottom = viewport->y + viewport->height;

		if (left >= right) continue;
		if (top >= bottom) continue;

		gfx_redraw_screen_rect(left, top, right, bottom);
	}

	w = orignal_w;
	sub_6E7FF3(w, viewport, x_diff, y_diff);
}

void sub_6E7DE1(sint16 x, sint16 y, rct_window* w, rct_viewport* viewport){
	uint8 zoom = (1 << viewport->zoom);

	// Note: do not do the subtraction and then divide!
	// Note: Due to arithmatic shift != /zoom a shift will have to be used
	// hopefully when 0x006E7FF3 is finished this can be converted to /zoom.
	sint16 x_diff = (viewport->view_x >> viewport->zoom) - (x >> viewport->zoom);
	sint16 y_diff = (viewport->view_y >> viewport->zoom) - (y >> viewport->zoom);

	viewport->view_x = x;
	viewport->view_y = y;

	// If no change in viewing area
	if ((!x_diff) && (!y_diff))return;

	if (w->flags & WF_7){
		int left = max(viewport->x, 0);
		int top = max(viewport->y, 0);
		int right = min(viewport->x + viewport->width, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16));
		int bottom = min(viewport->y + viewport->height, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16));

		if (left >= right) return;
		if (top >= bottom) return;

		gfx_redraw_screen_rect(left, top, right, bottom);
		return;
	}

	rct_viewport view_copy;
	memcpy(&view_copy, viewport, sizeof(rct_viewport));

	if (viewport->x < 0){
		viewport->width += viewport->x;
		viewport->view_width += viewport->x * zoom;
		viewport->view_x -= viewport->x * zoom;
		viewport->x = 0;
	}

	int eax = viewport->x + viewport->width - RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16);
	if (eax > 0){
		viewport->width -= eax;
		viewport->view_width -= eax * zoom;
	}

	if (viewport->width <= 0){
		memcpy(viewport, &view_copy, sizeof(rct_viewport));
		return;
	}

	if (viewport->y < 0){
		viewport->height += viewport->y;
		viewport->view_height += viewport->y * zoom;
		viewport->view_y -= viewport->y * zoom;
		viewport->y = 0;
	}

	eax = viewport->y + viewport->height - RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16);
	if (eax > 0){
		viewport->height -= eax;
		viewport->view_height -= eax * zoom;
	}

	if (viewport->height <= 0){
		memcpy(viewport, &view_copy, sizeof(rct_viewport));
		return;
	}

	sub_6E7F34(w, viewport, x_diff, y_diff);

	memcpy(viewport, &view_copy, sizeof(rct_viewport));
}

//rct2: 0x006E7A15
void viewport_set_underground_flag(int underground, rct_window* window, rct_viewport* viewport)
{
	if (window->classification != WC_MAIN_WINDOW)
	{
		if (!underground)
		{
			int bit = viewport->flags & VIEWPORT_FLAG_UNDERGROUND_INSIDE;
			viewport->flags &= ~VIEWPORT_FLAG_UNDERGROUND_INSIDE;
			if (!bit) return;
		}
		else
		{
			int bit = viewport->flags & VIEWPORT_FLAG_UNDERGROUND_INSIDE;
			viewport->flags |= VIEWPORT_FLAG_UNDERGROUND_INSIDE;
			if (bit) return;
		}
		window_invalidate(window);
	}
}

/**
 *
 *  rct2: 0x006E7A3A
 */
void viewport_update_position(rct_window *window)
{
	window_event_resize_call(window);

	rct_viewport* viewport = window->viewport;
	if (!viewport)return;

	if (window->viewport_target_sprite != -1) {
		viewport_update_sprite_follow(window);
		return;
	}

	sint16 x = viewport->view_width / 2 + window->saved_view_x;
	sint16 y = viewport->view_height / 2 + window->saved_view_y;
	sint16 z;

	sub_689174(&x, &y, &z);

	viewport_set_underground_flag(0, window, viewport);

	//Clamp to the map minimum value
	int at_map_edge = 0;
	if (x < MAP_MINIMUM_X_Y){
		x = MAP_MINIMUM_X_Y;
		at_map_edge = 1;
	}
	if (y < MAP_MINIMUM_X_Y){
		y = MAP_MINIMUM_X_Y;
		at_map_edge = 1;
	}

	//Clamp to the map maximum value (scenario specific)
	if (x > RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_MINUS_2, sint16)){
		x = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_MINUS_2, sint16);
		at_map_edge = 1;
	}
	if (y > RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_MINUS_2, sint16)){
		y = RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_MINUS_2, sint16);
		at_map_edge = 1;
	}

	if (at_map_edge) {
		// The &0xFFFF is to prevent the sign extension messing the
		// function up.
		int z = map_element_height(x & 0xFFFF, y & 0xFFFF);
		int _2d_x, _2d_y;
		center_2d_coordinates(x, y, z, &_2d_x, &_2d_y, viewport);

		if (window->saved_view_x > 0){
			_2d_x = min(_2d_x, window->saved_view_x);
		}
		else{
			_2d_x = max(_2d_x, window->saved_view_x);
		}

		if (window->saved_view_y > 0){
			_2d_y = min(_2d_y, window->saved_view_y);
		}
		else{
			_2d_y = max(_2d_y, window->saved_view_y);
		}

		window->saved_view_x = _2d_x;
		window->saved_view_y = _2d_y;
	}

	x = window->saved_view_x;
	y = window->saved_view_y;
	if (window->flags & WF_SCROLLING_TO_LOCATION){
		// Moves the viewport if focusing in on an item
		uint8 flags = 0;
		x -= viewport->view_x;
		if (x < 0){
			x = -x;
			flags |= 1;
		}
		y -= viewport->view_y;
		if (y < 0){
			y = -y;
			flags |= 2;
		}
		x = (x + 7) / 8;
		y = (y + 7) / 8;

		//If we are at the final zoom position
		if (!x && !y){
			window->flags &= ~WF_SCROLLING_TO_LOCATION;
		}
		if (flags & 1){
			x = -x;
		}
		if (flags & 2){
			y = -y;
		}
		x += viewport->view_x;
		y += viewport->view_y;
	}

	sub_6E7DE1(x, y, window, viewport);
}

void viewport_update_sprite_follow(rct_window *window)
{
	if (window->viewport_target_sprite != -1 && window->viewport){
		rct_sprite* sprite = &g_sprite_list[window->viewport_target_sprite];

		int height = (map_element_height(0xFFFF & sprite->unknown.x, 0xFFFF & sprite->unknown.y) & 0xFFFF) - 16;
		int underground = sprite->unknown.z < height;

		viewport_set_underground_flag(underground, window, window->viewport);

		int center_x, center_y;
		center_2d_coordinates(sprite->unknown.x, sprite->unknown.y, sprite->unknown.z, &center_x, &center_y, window->viewport);

		sub_6E7DE1(center_x, center_y, window, window->viewport);
	}
}

/**
 *
 *  rct2: 0x00685C02
 *  ax: left
 *  bx: top
 *  dx: right
 *  esi: viewport
 *  edi: dpi
 *  ebp: bottom
 */
void viewport_render(rct_drawpixelinfo *dpi, rct_viewport *viewport, int left, int top, int right, int bottom)
{
	if (right <= viewport->x) return;
	if (bottom <= viewport->y) return;
	if (left >= viewport->x + viewport->width)return;
	if (top >= viewport->y + viewport->height)return;

	int l = left, t = top, r = right, b = bottom;

	left = max(left - viewport->x, 0);
	right = min(right - viewport->x, viewport->width);
	top = max(top - viewport->y, 0);
	bottom = min(bottom - viewport->y, viewport->height);

	left <<= viewport->zoom;
	right <<= viewport->zoom;
	top <<= viewport->zoom;
	bottom <<= viewport->zoom;

	left += viewport->view_x;
	right += viewport->view_x;
	top += viewport->view_y;
	bottom += viewport->view_y;

	int height = bottom - top;
	if (height > 384){
		//Paint
		viewport_paint(viewport, dpi, left, top, right, top + 384);
		top += 384;
	}
	//Paint
	viewport_paint(viewport, dpi, left, top, right, bottom);

#ifdef DEBUG_SHOW_DIRTY_BOX
	if (viewport != g_viewport_list){
		gfx_fill_rect_inset(dpi, l, t, r-1, b-1, 0x2, 0x30);
		return;
	}
#endif
}

/**
*
*  rct2: 0x0068615B
*/
void painter_setup(){
	RCT2_GLOBAL(0xEE7888, uint32) = 0x00EE788C;
	RCT2_GLOBAL(0xF1AD28, uint32) = 0;
	RCT2_GLOBAL(0xF1AD2C, uint32) = 0;
	uint8* edi = RCT2_ADDRESS(0xF1A50C, uint8);
	memset(edi, 0, 2048);
	RCT2_GLOBAL(0xF1AD0C, sint32) = -1;
	RCT2_GLOBAL(0xF1AD10, uint32) = 0;
	RCT2_GLOBAL(0xF1AD20, uint32) = 0;
	RCT2_GLOBAL(0xF1AD24, uint32) = 0;
}

/***
 *
 * rct2: 0x00688596
 * Part of 0x688485
 */
void paint_attached_ps(paint_struct* ps, paint_struct* attached_ps, rct_drawpixelinfo* dpi){
	for (; attached_ps; attached_ps = attached_ps->next_attached_ps){
		sint16 x = attached_ps->attached_x + ps->x;
		sint16 y = attached_ps->attached_y + ps->y;

		int image_id = attached_ps->image_id;
		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_SEETHROUGH_RIDES){
			if (ps->sprite_type == 3){
				if (image_id & 0x40000000){
					image_id &= 0x7FFFF;
					image_id |= 0x41880000;
				}
			}
		}

		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_SEETHROUGH_SCENERY){
			if (ps->sprite_type == 5){
				if (image_id & 0x40000000){
					image_id &= 0x7FFFF;
					image_id |= 0x41880000;
				}
			}
		}

		if (attached_ps->var_0C & 1) {
			gfx_draw_sprite_raw_masked(dpi, x, y, image_id, attached_ps->var_04);
		} else {
			gfx_draw_sprite(dpi, image_id, x, y, ps->var_04);
		}
	}
}

void sub_688485(){
	rct_drawpixelinfo* dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);
	paint_struct* ps = RCT2_GLOBAL(0xEE7884, paint_struct*);
	paint_struct* previous_ps = ps->next_quadrant_ps;

	for (ps = ps->next_quadrant_ps; ps;){
		sint16 x = ps->x;
		sint16 y = ps->y;
		if (ps->sprite_type == 2){
			if (dpi->zoom_level >= 1){
				x &= 0xFFFE;
				y &= 0xFFFE;
				if (dpi->zoom_level >= 2){
					x &= 0xFFFC;
					y &= 0xFFFC;
				}
			}
		}
		int image_id = ps->image_id;
		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_SEETHROUGH_RIDES){
			if (ps->sprite_type == 3){
				if (!(image_id & 0x40000000)){
					image_id &= 0x7FFFF;
					image_id |= 0x41880000;
				}
			}
		}
		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_UNDERGROUND_INSIDE){
			if (ps->sprite_type == 9){
				if (!(image_id & 0x40000000)){
					image_id &= 0x7FFFF;
					image_id |= 0x41880000;
				}
			}
		}
		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_SEETHROUGH_SCENERY){
			if (ps->sprite_type == 10 || ps->sprite_type == 12 || ps->sprite_type == 9 || ps->sprite_type == 5){
				if (!(image_id & 0x40000000)){
					image_id &= 0x7FFFF;
					image_id |= 0x41880000;
				}
			}
		}

		if (ps->var_1A & 1)
			gfx_draw_sprite_raw_masked(dpi, x, y, image_id, ps->var_04);
		else
			gfx_draw_sprite(dpi, image_id, x, y, ps->var_04);

		if (ps->var_20 != 0){
			ps = ps->var_20;
			continue;
		}

		paint_attached_ps(ps, ps->attached_ps, dpi);
		ps = previous_ps->next_quadrant_ps;
		previous_ps = ps;
	}

}

/* rct2: 0x006874B0, 0x00687618, 0x0068778C, 0x00687902, 0x0098199C */
int sub_98199C(sint8 al, sint8 ah, int image_id, sint8 cl, int height, sint16 length_y, sint16 length_x, uint32 rotation){
	RCT2_CALLPROC_X(RCT2_ADDRESS(0x98199C, uint32_t)[get_current_rotation()],
		al | (ah << 8), 
		image_id, 
		cl, 
		height, 
		length_y, 
		length_x, 
		rotation);
	return 1;
}

/* rct2: 0x00686806, 0x006869B2, 0x00686B6F, 0x00686D31, 0x0098197C */
int sub_98197C(sint8 al, sint8 ah, int image_id, sint8 cl, int height, sint16 length_y, sint16 length_x, uint32 rotation){
	int ebp = ah + RCT2_GLOBAL(0x9DEA56, sint16);

	RCT2_GLOBAL(0xF1AD28, paint_struct*) = 0;
	RCT2_GLOBAL(0xF1AD2C, uint32) = 0;

	//Not a paint struct but something similar
	paint_struct* ps = RCT2_GLOBAL(0xEE7888, paint_struct*);

	if ((uint32)ps >= RCT2_GLOBAL(0xEE7880, uint32))return 1;

	ps->image_id = image_id;

	rct_g1_element *g1Element;
	uint32 image_element = image_id & 0x7FFFF;

	if (image_element < SPR_G2_BEGIN) {
		g1Element = &g1Elements[image_element];
	}
	else {
		g1Element = &g2.elements[image_element - SPR_G2_BEGIN];
	}

	rct_xyz16 coord_3d = {
		.x = al,
		.y = cl,
		.z = height
	};

	switch (rotation) {
	case 0:
		rotate_map_coordinates(&coord_3d.x, &coord_3d.y, 0);
		break;
	case 1:
		rotate_map_coordinates(&coord_3d.x, &coord_3d.y, 3);
		break;
	case 2:
		rotate_map_coordinates(&coord_3d.x, &coord_3d.y, 2);
		break;
	case 3:
		rotate_map_coordinates(&coord_3d.x, &coord_3d.y, 1);
		break;
	}
	coord_3d.x += RCT2_GLOBAL(0x9DE568, sint16);
	coord_3d.y += RCT2_GLOBAL(0x9DE56C, sint16);

	rct_xy16 map = coordinate_3d_to_2d(&coord_3d, rotation);

	ps->x = map.x;
	ps->y = map.y;

	int left = map.x + g1Element->x_offset;
	int bottom = map.y + g1Element->y_offset;

	int right = left + g1Element->width;
	int top = bottom + g1Element->height;

	RCT2_GLOBAL(0xF1AD1C, uint16) = left;
	RCT2_GLOBAL(0xF1AD1E, uint16) = bottom;

	rct_drawpixelinfo* dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);

	if (right <= dpi->x)return 1;
	if (top <= dpi->y)return 1;
	if (left > dpi->x + dpi->width)return 1;
	if (bottom > dpi->y + dpi->height)return 1;

	rct_xy16 boundBox = {
		.x = length_x,
		.y = length_y
	};

	rct_xy16 s_unk = {
		.x = RCT2_GLOBAL(0x9DEA52, sint16),
		.y = RCT2_GLOBAL(0x9DEA54, sint16)
	};

	// Unsure why rots 1 and 3 need to swap
	switch (rotation){
	case 0:		
		boundBox.x--;
		boundBox.y--;
		rotate_map_coordinates(&s_unk.x, &s_unk.y, 0);
		rotate_map_coordinates(&boundBox.x, &boundBox.y, 0);
		break;
	case 1:
		boundBox.x--;
		rotate_map_coordinates(&s_unk.x, &s_unk.y, 3);
		rotate_map_coordinates(&boundBox.x, &boundBox.y, 3);
		break;
	case 2:
		rotate_map_coordinates(&boundBox.x, &boundBox.y, 2);
		rotate_map_coordinates(&s_unk.x, &s_unk.y, 2);
		break;
	case 3:
		boundBox.y--;
		rotate_map_coordinates(&boundBox.x, &boundBox.y, 1);
		rotate_map_coordinates(&s_unk.x, &s_unk.y, 1);
		break;
	}

	ps->other_x = boundBox.x + s_unk.x + RCT2_GLOBAL(0x9DE568, sint16);
	ps->some_x = RCT2_GLOBAL(0x009DEA56, sint16);
	ps->some_y = ebp;
	ps->other_y = boundBox.y + s_unk.y + RCT2_GLOBAL(0x009DE56C, sint16);
	ps->var_1A = 0;
	ps->attached_x = s_unk.x + RCT2_GLOBAL(0x9DE568, sint16);
	ps->attached_y = s_unk.y + RCT2_GLOBAL(0x009DE56C, sint16);
	ps->attached_ps = NULL;
	ps->var_20 = NULL;
	ps->sprite_type = RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8);
	ps->var_29 = RCT2_GLOBAL(0x9DE571, uint8);
	ps->map_x = RCT2_GLOBAL(0x9DE574, uint16);
	ps->map_y = RCT2_GLOBAL(0x9DE576, uint16);
	ps->mapElement = RCT2_GLOBAL(0x9DE578, rct_map_element*);

	RCT2_GLOBAL(0xF1AD28, paint_struct*) = ps;

	rct_xy16 attach = {
		.x = ps->attached_x,
		.y = ps->attached_y
	};

	rotate_map_coordinates(&attach.x, &attach.y, rotation);
	switch (rotation){
	case 0:
		break;
	case 1:
	case 3:
		attach.x += 0x2000;
		break;
	case 2:
		attach.x += 0x4000;
		break;
	}

	sint16 di = attach.x + attach.y;

	if (di < 0)
		di = 0;

	di /= 32;
	if (di > 511)
		di = 511;

	ps->var_18 = di;
	paint_struct* old_ps = RCT2_ADDRESS(0x00F1A50C, paint_struct*)[di];
	RCT2_ADDRESS(0x00F1A50C, paint_struct*)[di] = ps;
	ps->next_quadrant_ps = old_ps;

	if ((uint16)di < RCT2_GLOBAL(0x00F1AD0C, uint32)){
		RCT2_GLOBAL(0x00F1AD0C, uint32) = di;
	}

	if ((uint16)di > RCT2_GLOBAL(0x00F1AD10, uint32)){
		RCT2_GLOBAL(0x00F1AD10, uint32) = di;
	}

	RCT2_GLOBAL(0xEE7888, paint_struct*) += 1;
	return 0;
}

/**
 *
 *  rct2: 0x006D4244
 */
void viewport_vehicle_paint_setup(rct_vehicle *vehicle, int imageDirection)
{
	rct_ride_type *rideEntry;
	const rct_ride_type_vehicle *vehicleEntry;

	int x = vehicle->x;
	int y = vehicle->y;
	int z = vehicle->z;

	if (vehicle->var_0C & 0x80) {
		uint32 ebx = 22965 + vehicle->var_C5;
		RCT2_GLOBAL(0x9DEA52, uint16) = 0;
		RCT2_GLOBAL(0x9DEA54, uint16) = 0;
		RCT2_GLOBAL(0x9DEA56, uint16) = z + 2;
		sub_98197C(0, 0, ebx, 0, z, 1, 1, get_current_rotation());
		return;
	}

	if (vehicle->ride_subtype == 0xFF) {
		vehicleEntry = &CableLiftVehicle;
	} else {
		rideEntry = GET_RIDE_ENTRY(vehicle->ride_subtype);
		vehicleEntry = &rideEntry->vehicles[vehicle->vehicle_type];

		if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_11) {
			vehicleEntry++;
			z += 16;
		}
	}

	uint32 rct2VehiclePtrFormat = ((uint32)vehicleEntry) - offsetof(rct_ride_type, vehicles);
	RCT2_GLOBAL(0x00F64DFC, uint32) = rct2VehiclePtrFormat;
	switch (vehicleEntry->car_visual) {
	case VEHICLE_VISUAL_DEFAULT:						RCT2_CALLPROC_X(0x006D45F8, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_LAUNCHED_FREEFALL:				RCT2_CALLPROC_X(0x006D5FAB, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_OBSERVATION_TOWER:				RCT2_CALLPROC_X(0x006D6258, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_RIVER_RAPIDS:					RCT2_CALLPROC_X(0x006D5889, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_MINI_GOLF_PLAYER:				RCT2_CALLPROC_X(0x006D42F0, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_MINI_GOLF_BALL:					RCT2_CALLPROC_X(0x006D43C6, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_REVERSER:						RCT2_CALLPROC_X(0x006D4453, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_SPLASH_BOATS_OR_WATER_COASTER:	RCT2_CALLPROC_X(0x006D4295, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_ROTO_DROP:						RCT2_CALLPROC_X(0x006D5DA9, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case 10:											RCT2_CALLPROC_X(0x006D5600, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case 11:											RCT2_CALLPROC_X(0x006D5696, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case 12:											RCT2_CALLPROC_X(0x006D57EE, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case 13:											RCT2_CALLPROC_X(0x006D5783, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case 14:											RCT2_CALLPROC_X(0x006D5701, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_VIRGINIA_REEL:					RCT2_CALLPROC_X(0x006D5B48, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	case VEHICLE_VISUAL_SUBMARINE:						RCT2_CALLPROC_X(0x006D44D5, x, imageDirection, y, z, (int)vehicle, rct2VehiclePtrFormat, 0); break;
	}
}

/**
 *
 *  rct2: 0x0068F0FB
 */
void viewport_peep_paint_setup(rct_peep *peep, int imageDirection)
{
	RCT2_CALLPROC_X(0x0068F0FB, peep->x, imageDirection, peep->y, peep->z, (int)peep, 0, 0);
}

/**
 *
 *  rct2: 0x00672AC9
 */
void viewport_misc_paint_setup(rct_sprite *misc, int imageDirection)
{
	RCT2_CALLPROC_X(0x00672AC9, misc->unknown.x, imageDirection, misc->unknown.y, misc->unknown.z, (int)misc, 0, 0);
}

/**
*  Litter Paint Setup
*  rct2: 0x006736FC
*/
void viewport_litter_paint_setup(rct_litter *litter, int imageDirection)
{
	rct_drawpixelinfo *dpi;

	dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);
	if (dpi->zoom_level != 0) return; // If zoomed at all no litter drawn

	// litter has no sprite direction so remove that
	imageDirection >>= 3;
	// Some litter types have only 1 direction so remove
	// anything that isn't required.
	imageDirection &= RCT2_ADDRESS(0x97EF6C, uint32)[litter->type * 2 + 1];

	uint32 image_id = imageDirection + RCT2_ADDRESS(0x97EF6C, uint32)[litter->type * 2];

	RCT2_GLOBAL(0x9DEA52, uint16) = 0xFFFC;
	RCT2_GLOBAL(0x9DEA54, uint16) = 0xFFFC;
	RCT2_GLOBAL(0x9DEA56, uint16) = litter->z + 2;

	sub_98197C(0, 0xFF, image_id, 0, litter->z, 4, 4, get_current_rotation());
}


/**
*  Paint Quadrant
*  rct2: 0x0069E8B0
*/
void sprite_paint_setup(uint16 eax, uint16 ecx){
	uint32 _eax = eax, _ecx = ecx;
	rct_drawpixelinfo* dpi;


	if (RCT2_GLOBAL(0x9DEA6F, uint8) & 1) return;

	dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);

	if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_INVISIBLE_SPRITES) return;

	if (dpi->zoom_level > 2) return;

	if (eax > 0x2000)return;
	if (ecx > 0x2000)return;

	//push eax, ecx
	eax = (eax & 0x1FE0) << 3 | (ecx >> 5);
	int sprite_idx = RCT2_ADDRESS(0xF1EF60, uint16)[eax];
	if (sprite_idx == SPRITE_INDEX_NULL) return;

	for (rct_sprite* spr = &g_sprite_list[sprite_idx]; sprite_idx != SPRITE_INDEX_NULL; sprite_idx = spr->unknown.next_in_quadrant){
		spr = &g_sprite_list[sprite_idx];
		dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);

		if (dpi->y + dpi->height <= spr->unknown.sprite_top) continue;
		if (spr->unknown.sprite_bottom <= dpi->y)continue;
		if (dpi->x + dpi->width <= spr->unknown.sprite_left)continue;
		if (spr->unknown.sprite_right <= dpi->x)continue;

		int image_direction = get_current_rotation();
		image_direction <<= 3;
		image_direction += spr->unknown.sprite_direction;
		image_direction &= 0x1F;

		RCT2_GLOBAL(0x9DE578, uint32) = (uint32)spr;
		int ebp = spr->unknown.sprite_identifier;

		RCT2_GLOBAL(0x9DE568, sint16) = spr->unknown.x;
		RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = VIEWPORT_INTERACTION_ITEM_SPRITE;
		RCT2_GLOBAL(0x9DE56C, sint16) = spr->unknown.y;

		switch (spr->unknown.sprite_identifier){
		case SPRITE_IDENTIFIER_VEHICLE:
			viewport_vehicle_paint_setup((rct_vehicle*)spr, image_direction);
			break;
		case SPRITE_IDENTIFIER_PEEP:
			viewport_peep_paint_setup((rct_peep*)spr, image_direction);
			break;
		case SPRITE_IDENTIFIER_MISC:
			viewport_misc_paint_setup(spr, image_direction);
			break;
		case SPRITE_IDENTIFIER_LITTER:
			viewport_litter_paint_setup((rct_litter*)spr, image_direction);
			break;
		default:
			assert(false);
			break;
		}
	}
}

/* rct2: 0x006629BC
 * returns al
 * ebp : image_id
 * ax : unknown
 * dx : height
 * edi : unknown
 */
int sub_6629BC(int height, uint16 ax, uint32 image_id, int edi){
	int eax = ax, ebx = 0, ecx = 0, edx = height, esi = 0, _edi = edi, ebp = image_id;

	RCT2_CALLFUNC_X(0x006629BC, &eax, &ebx, &ecx, &edx, &esi, &_edi, &ebp);

	return eax & 0xFF;
}

/* rct2: 0x0066508C & 0x00665540 */
void viewport_ride_entrance_exit_paint_setup(uint8 direction, int height, rct_map_element* map_element)
{
	rct_drawpixelinfo* dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);
	uint8 is_exit = map_element->properties.entrance.type == ENTRANCE_TYPE_RIDE_EXIT;

	if (RCT2_GLOBAL(0x9DEA6F, uint8_t) & 1){
		if (map_element->properties.entrance.ride_index != RCT2_GLOBAL(0x00F64DE8, uint8))
			return;
	}

	rct_ride* ride = GET_RIDE(map_element->properties.entrance.ride_index);
	if (ride->entrance_style == RIDE_ENTRANCE_STYLE_NONE) return;

	const rct_ride_entrance_definition *style = &RideEntranceDefinitions[ride->entrance_style];

	uint8 colour_1, colour_2;
	uint32 transparant_image_id = 0, image_id = 0;
	if (style->flags & (1 << 30)) {
		colour_1 = ride->track_colour_main[0] + 0x70;
		transparant_image_id = (colour_1 << 19) | 0x40000000;
	}

	colour_1 = ride->track_colour_main[0];
	colour_2 = ride->track_colour_additional[0];
	image_id = (colour_1 << 19) | (colour_2 << 24) | 0xA0000000;

	RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = VIEWPORT_INTERACTION_ITEM_RIDE;
	RCT2_GLOBAL(0x009E32BC, uint32) = 0;

	if (map_element->flags & MAP_ELEMENT_FLAG_GHOST){
		RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = VIEWPORT_INTERACTION_ITEM_NONE;
		image_id = RCT2_ADDRESS(0x993CC4, uint32_t)[RCT2_GLOBAL(0x9AACBF, uint8)];
		RCT2_GLOBAL(0x009E32BC, uint32) = image_id;
		if (transparant_image_id)
			transparant_image_id = image_id;
	}

	if (is_exit){
		image_id |= style->sprite_index + direction + 8;
	}
	else{
		image_id |= style->sprite_index + direction;
	}
	// Format modifed to stop repeated code

	// Each entrance is split into 2 images for drawing
	// Certain entrance styles have another 2 images to draw for coloured windows

	RCT2_GLOBAL(0x009DEA52, uint16) = 2;
	RCT2_GLOBAL(0x009DEA54, uint16) = 2;
	RCT2_GLOBAL(0x009DEA56, uint16) = height;

	sint8 ah = is_exit ? 0x23 : 0x33;

	sint16 lengthY = (direction & 1) ? 28 : 2;
	sint16 lengthX = (direction & 1) ? 2 : 28;

	sub_98197C(0, ah, image_id, 0, height, lengthY, lengthX, get_current_rotation());

	if (transparant_image_id){
		if (is_exit){
			transparant_image_id |= style->sprite_index + direction + 24;
		}
		else{
			transparant_image_id |= style->sprite_index + direction + 16;
		}
		RCT2_GLOBAL(0x009DEA52, uint16) = 2;
		RCT2_GLOBAL(0x009DEA54, uint16) = 2;
		RCT2_GLOBAL(0x009DEA56, uint16) = height;

		sub_98199C(0, ah, transparant_image_id, 0, height, lengthY, lengthX, 0);
	}

	image_id += 4;

	RCT2_GLOBAL(0x009DEA52, uint16) = (direction & 1) ? 28 : 2;
	RCT2_GLOBAL(0x009DEA54, uint16) = (direction & 1) ? 2 : 28;
	RCT2_GLOBAL(0x009DEA56, uint16) = height;

	sub_98197C(0, ah, image_id, 0, height, lengthY, lengthX, get_current_rotation());

	if (transparant_image_id){
		transparant_image_id += 4;
		RCT2_GLOBAL(0x009DEA52, uint16) = (direction & 1) ? 28 : 2;
		RCT2_GLOBAL(0x009DEA54, uint16) = (direction & 1) ? 2 : 28;
		RCT2_GLOBAL(0x009DEA56, uint16) = height;

		sub_98199C(0, ah, transparant_image_id, 0, height, lengthY, lengthX, 0);
	}

	uint32 eax = 0xFFFF0600 | ((height / 16) & 0xFF);
	if (direction & 1){
		RCT2_ADDRESS(0x009E30B6, uint32)[RCT2_GLOBAL(0x141F56B, uint8) / 2] = eax;
		RCT2_GLOBAL(0x141F56B, uint8)++;
	}
	else{
		RCT2_ADDRESS(0x009E3138, uint32)[RCT2_GLOBAL(0x141F56A, uint8) / 2] = eax;
		RCT2_GLOBAL(0x141F56A, uint8)++;
	}

	if (!is_exit &&
		!(map_element->flags & MAP_ELEMENT_FLAG_GHOST) &&
		map_element->properties.entrance.ride_index != 0xFF){

		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint32) = 0;
		RCT2_GLOBAL(0x13CE956, uint32) = 0;

		rct_string_id string_id = STR_RIDE_ENTRANCE_CLOSED;

		if (ride->status == RIDE_STATUS_OPEN &&
			!(ride->lifecycle_flags & RIDE_LIFECYCLE_BROKEN_DOWN)){

			RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS + 0, rct_string_id) = ride->name;
			RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS + 2, uint32) = ride->name_arguments;

			string_id = STR_RIDE_ENTRANCE_NAME;
		}

		utf8 entrance_string[MAX_PATH];
		if (gConfigGeneral.upper_case_banners) {
			format_string_to_upper(entrance_string, string_id, RCT2_ADDRESS(RCT2_ADDRESS_COMMON_FORMAT_ARGS, void));
		} else {
			format_string(entrance_string, string_id, RCT2_ADDRESS(RCT2_ADDRESS_COMMON_FORMAT_ARGS, void));
		}

		RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0x1C0;

		uint16 string_width = gfx_get_string_width(entrance_string);
		uint16 scroll = (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) / 2) % string_width;

		RCT2_GLOBAL(0x009DEA52, uint16) = 2;
		RCT2_GLOBAL(0x009DEA54, uint16) = 2;
		RCT2_GLOBAL(0x009DEA56, uint16) = height + style->height;
		sub_98199C(0, 0x33, scrolling_text_setup(string_id, scroll, style->scrolling_mode), 0, height + style->height, 0x1C, 0x1C, 0);
	}

	image_id = RCT2_GLOBAL(0x009E32BC, uint32);
	if (!image_id){
		image_id = 0x20B80000;
	}

	if (direction & 1){
		sub_6629BC(height, 0, image_id, 1);
	}
	else{
		sub_6629BC(height, 0, image_id, 0);
	}

	RCT2_GLOBAL(0x141E9B4, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9B8, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9BC, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9C0, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9C4, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9C8, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9CC, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9D0, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9D4, uint16) = 0xFFFF;

	height += is_exit ? 40 : 56;
	if (RCT2_GLOBAL(0x141E9D8, sint16) < height){
		RCT2_GLOBAL(0x141E9D8, sint16) = height;
		RCT2_GLOBAL(0x141E9DA, uint8) = 32;
	}
}

/* rct2: 0x006658ED */
void viewport_park_entrance_paint_setup(uint8 direction, int height, rct_map_element* map_element){
	if (RCT2_GLOBAL(0x9DEA6F, uint8_t) & 1)
		return;

	RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = VIEWPORT_INTERACTION_ITEM_PARK;
	RCT2_GLOBAL(0x009E32BC, uint32) = 0;
	uint32 image_id, ghost_id = 0;
	if (map_element->flags & MAP_ELEMENT_FLAG_GHOST){
		RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = VIEWPORT_INTERACTION_ITEM_NONE;
		ghost_id = RCT2_ADDRESS(0x993CC4, uint32)[RCT2_GLOBAL(0x9AACBF, uint8)];
		RCT2_GLOBAL(0x009E32BC, uint32) = ghost_id;
	}

	rct_path_type* path_entry = g_pathTypeEntries[map_element->properties.entrance.path_type];

	// Index to which part of the entrance
	// Middle, left, right
	uint8 part_index = map_element->properties.entrance.index & 0xF;
	rct_entrance_type* entrance;
	uint8 di = (direction / 2 + part_index / 2) & 1 ? 0x1A : 0x20;

	switch (part_index){
	case 0:
		image_id = (path_entry->image + 5 * (1 + (direction & 1))) | ghost_id;
		RCT2_GLOBAL(0x009DEA52, uint16) = 0;
		RCT2_GLOBAL(0x009DEA54, uint16) = 2;
		RCT2_GLOBAL(0x009DEA56, sint16) = height;

		sub_98197C(0, 0, image_id, 0, height, 0x1C, 32, get_current_rotation());

		entrance = (rct_entrance_type*)object_entry_groups[OBJECT_TYPE_PARK_ENTRANCE].chunks[0];
		image_id = (entrance->image_id + direction * 3) | ghost_id;

		RCT2_GLOBAL(0x009DEA52, uint16) = 2;
		RCT2_GLOBAL(0x009DEA54, uint16) = 2;
		RCT2_GLOBAL(0x009DEA56, sint16) = height + 32;

		sub_98197C(0, 0x2F, image_id, 0, height, 0x1C, 0x1C, get_current_rotation());

		if ((direction + 1) & (1 << 1))
			break;
		if (ghost_id != 0)
			break;

		rct_string_id park_text_id = 1730;
		RCT2_GLOBAL(0x0013CE952, uint32) = 0;
		RCT2_GLOBAL(0x0013CE956, uint32) = 0;

		if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & PARK_FLAGS_PARK_OPEN){
			RCT2_GLOBAL(0x0013CE952, rct_string_id) = RCT2_GLOBAL(RCT2_ADDRESS_PARK_NAME, rct_string_id);
			RCT2_GLOBAL(0x0013CE954, rct_string_id) = RCT2_GLOBAL(RCT2_ADDRESS_PARK_NAME_ARGS, rct_string_id);

			park_text_id = 1731;
		}

		utf8 park_name[MAX_PATH];
		if (gConfigGeneral.upper_case_banners) {
			format_string_to_upper(park_name, park_text_id, RCT2_ADDRESS(RCT2_ADDRESS_COMMON_FORMAT_ARGS, void));
		} else {
			format_string(park_name, park_text_id, RCT2_ADDRESS(RCT2_ADDRESS_COMMON_FORMAT_ARGS, void));
		}

		RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0x1C0;
		uint16 string_width = gfx_get_string_width(park_name);
		uint16 scroll = (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) / 2) % string_width;

		if (entrance->scrolling_mode == 0xFF)
			break;

		RCT2_GLOBAL(0x009DEA52, uint16) = 2;
		RCT2_GLOBAL(0x009DEA54, uint16) = 2;
		RCT2_GLOBAL(0x009DEA56, sint16) = height + entrance->text_height;

		sub_98199C(0, 0x2F, scrolling_text_setup(park_text_id, scroll, entrance->scrolling_mode + direction / 2), 0, height + entrance->text_height, 0x1C, 0x1C, 0);
		break;
	case 1:
	case 2:
		entrance = (rct_entrance_type*)object_entry_groups[OBJECT_TYPE_PARK_ENTRANCE].chunks[0];
		image_id = (entrance->image_id + part_index + direction * 3) | ghost_id;

		RCT2_GLOBAL(0x009DEA52, uint16) = 3;
		RCT2_GLOBAL(0x009DEA54, uint16) = 3;
		RCT2_GLOBAL(0x009DEA56, sint16) = height;

		sub_98197C(0, 0x4F, image_id, 0, height, di, 0x1A, get_current_rotation());
		break;
	}

	image_id = ghost_id;
	if (!image_id){
		image_id = 0x20B80000;
	}

	if (direction & 1){
		sub_6629BC(height, 0, image_id, 1);
	}
	else{
		sub_6629BC(height, 0, image_id, 0);
	}

	RCT2_GLOBAL(0x141E9B4, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9B8, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9BC, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9C0, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9C4, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9C8, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9CC, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9D0, uint16) = 0xFFFF;
	RCT2_GLOBAL(0x141E9D4, uint16) = 0xFFFF;

	height += 80;
	if (RCT2_GLOBAL(0x141E9D8, sint16) < height){
		RCT2_GLOBAL(0x141E9D8, sint16) = height;
		RCT2_GLOBAL(0x141E9DA, uint8) = 32;
	}
}

/**
 *
 *  rct2: 0x006C4794
 */
void viewport_track_paint_setup(uint8 direction, int height, rct_map_element *mapElement)
{
	rct_drawpixelinfo *dpi = RCT2_GLOBAL(0x0140E9A8, rct_drawpixelinfo*);
	rct_ride *ride;
	int rideIndex, trackType, trackColourScheme, trackSequence;

	rideIndex = mapElement->properties.track.ride_index;
	ride = GET_RIDE(rideIndex);

	// HACK Set entrance style to plain if none to stop glitch until entrance track piece is implemented
	bool isEntranceStyleNone = false;
	if (ride->entrance_style == RIDE_ENTRANCE_STYLE_NONE) {
		isEntranceStyleNone = true;
		ride->entrance_style = RIDE_ENTRANCE_STYLE_PLAIN;
	}

	if (!(RCT2_GLOBAL(0x009DEA6F, uint8) & 1) || rideIndex == RCT2_GLOBAL(0x00F64DE8, uint8)) {
		trackType = mapElement->properties.track.type;
		trackSequence = mapElement->properties.track.sequence & 0x0F;
		trackColourScheme = mapElement->properties.track.colour & 3;

		if ((RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_TRACK_HEIGHTS) && dpi->zoom_level == 0) {
			RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = 0;
			if (RCT2_ADDRESS(0x00999694, uint32)[trackType] & (1 << trackSequence)) {
				uint16 ax = RCT2_GLOBAL(0x0097D21A + (ride->type * 8), uint8);
				uint32 ebx = 0x20381689 + (height + 8) / 16;
				ebx += RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_HEIGHT_MARKERS, uint16);
				ebx -= RCT2_GLOBAL(0x01359208, uint16);
				RCT2_GLOBAL(0x009DEA52, uint16) = 1000;
				RCT2_GLOBAL(0x009DEA54, uint16) = 1000;
				RCT2_GLOBAL(0x009DEA56, uint16) = 2047;
				sub_98197C(16, 0, ebx, 16, height + ax + 3, 1, 1, get_current_rotation());
			}
		}

		RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = 3;
		RCT2_GLOBAL(0x00F44198, uint32) = (ride->track_colour_main[trackColourScheme] << 19) | (ride->track_colour_additional[trackColourScheme] << 24) | 0xA0000000;
		RCT2_GLOBAL(0x00F441A0, uint32) = 0x20000000;
		RCT2_GLOBAL(0x00F441A4, uint32) = 0x20C00000;
		RCT2_GLOBAL(0x00F4419C, uint32) = (ride->track_colour_supports[trackColourScheme] << 19) | 0x20000000;
		if (mapElement->type & 0x40) {
			RCT2_GLOBAL(0x00F44198, uint32) = 0x21600000;
			RCT2_GLOBAL(0x00F4419C, uint32) = 0x21600000;
			RCT2_GLOBAL(0x00F441A0, uint32) = 0x21600000;
			RCT2_GLOBAL(0x00F441A4, uint32) = 0x21600000;
		}
		if (mapElement->flags & MAP_ELEMENT_FLAG_GHOST) {
			uint32 ghost_id = RCT2_ADDRESS(0x00993CC4, uint32)[RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_CONSTRUCTION_MARKER, uint8)];
			RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = 0;
			RCT2_GLOBAL(0x00F44198, uint32) = ghost_id;
			RCT2_GLOBAL(0x00F4419C, uint32) = ghost_id;
			RCT2_GLOBAL(0x00F441A0, uint32) = ghost_id;
			RCT2_GLOBAL(0x00F441A4, uint32) = ghost_id;
		}

		TRACK_PAINT_FUNCTION **trackTypeList = (TRACK_PAINT_FUNCTION**)RideTypeTrackPaintFunctionsOld[ride->type];
		if (trackTypeList == NULL) {
			trackTypeList = (TRACK_PAINT_FUNCTION**)RideTypeTrackPaintFunctions[ride->type];

			if (trackTypeList[trackType] != NULL)
				trackTypeList[trackType][direction](rideIndex, trackSequence, direction, height, mapElement);
		}
		else {
			uint32 *trackDirectionList = (uint32*)trackTypeList[trackType];

			// Have to call from this point as it pushes esi and expects callee to pop it
			RCT2_CALLPROC_X(
				0x006C4934,
				ride->type,
				(int)trackDirectionList,
				direction,
				height,
				(int)mapElement,
				rideIndex * sizeof(rct_ride),
				trackSequence
				);
		}
	}

	if (isEntranceStyleNone) {
		ride->entrance_style = RIDE_ENTRANCE_STYLE_NONE;
	}
}

/* rct2: 0x00664FD4 */
void viewport_entrance_paint_setup(uint8 direction, int height, rct_map_element* map_element){
	RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8_t) = VIEWPORT_INTERACTION_ITEM_LABEL;

	rct_drawpixelinfo* dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);

	if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_PATH_HEIGHTS &&
		dpi->zoom_level == 0){
		uint32 ebx =
			(map_element->properties.entrance.type << 4) |
			(map_element->properties.entrance.index & 0xF);

		if (RCT2_ADDRESS(0x0097B974, uint8)[ebx] & 0xF){

			int z = map_element->base_height * 8 + 3;
			uint32 image_id =
				z / 16 +
				RCT2_GLOBAL(RCT2_ADDRESS_CONFIG_HEIGHT_MARKERS,sint16) +
				0x20101689;

			image_id -= RCT2_GLOBAL(0x01359208, sint16);

			RCT2_GLOBAL(0x009DEA52, uint16) = 31;
			RCT2_GLOBAL(0x009DEA54, uint16) = 31;
			RCT2_GLOBAL(0x009DEA56, sint16) = z;
			RCT2_GLOBAL(0x009DEA56, uint16) += 64;

			sub_98197C(16, 0, image_id, 16, height, 1, 1, get_current_rotation());
		}
	}

	switch (map_element->properties.entrance.type){
	case ENTRANCE_TYPE_RIDE_ENTRANCE:
	case ENTRANCE_TYPE_RIDE_EXIT:
		viewport_ride_entrance_exit_paint_setup(direction, height, map_element);
		break;
	case ENTRANCE_TYPE_PARK_ENTRANCE:
		viewport_park_entrance_paint_setup(direction, height, map_element);
		break;
	}
}

/* rct2: 0x006B9CC4 */
void viewport_banner_paint_setup(uint8 direction, int height, rct_map_element* map_element)
{
	rct_drawpixelinfo* dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);

	RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8_t) = VIEWPORT_INTERACTION_ITEM_BANNER;

	if (dpi->zoom_level > 1 || RCT2_GLOBAL(0x9DEA6F, uint8_t) & 1) return;

	height -= 16;

	rct_scenery_entry* banner_scenery = g_bannerSceneryEntries[gBanners[map_element->properties.banner.index].type];

	direction += map_element->properties.banner.position;
	direction &= 3;

	RCT2_GLOBAL(0x9DEA56, uint16_t) = height + 2;

	RCT2_GLOBAL(0x9DEA52, uint32_t) = RCT2_ADDRESS(0x98D884, uint32)[direction * 2];

	uint32 base_id = (direction << 1) + banner_scenery->image;
	uint32 image_id = base_id;

	if (map_element->flags & MAP_ELEMENT_FLAG_GHOST)//if being placed
	{
		RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8_t) = VIEWPORT_INTERACTION_ITEM_NONE;
		image_id |= RCT2_ADDRESS(0x993CC4, uint32_t)[RCT2_GLOBAL(0x9AACBF, uint8)];
	}
	else{
		image_id |=
			(gBanners[map_element->properties.banner.index].colour << 19) |
			0x20000000;
	}

	sub_98197C(0, 0x15, image_id, 0, height, 1, 1, get_current_rotation());
	RCT2_GLOBAL(0x9DEA52, uint32) = RCT2_ADDRESS(0x98D888, uint32)[direction * 2];

	image_id++;
	sub_98197C(0, 0x15, image_id, 0, height, 1, 1, get_current_rotation());

	// Opposite direction
	direction ^= 2;
	direction--;
	// If text not showing / ghost
	if (direction >= 2 || (map_element->flags & MAP_ELEMENT_FLAG_GHOST)) return;

	uint16 scrollingMode = banner_scenery->banner.scrolling_mode;
	scrollingMode += direction;

	RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint32) = 0;
	RCT2_GLOBAL(0x13CE956, uint32_t) = 0;

	rct_string_id string_id = STR_NO_ENTRY;
	if (!(gBanners[map_element->properties.banner.index].flags & BANNER_FLAG_NO_ENTRY))
	{
		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint16) = gBanners[map_element->properties.banner.index].string_idx;
		string_id = STR_BANNER_TEXT;
	}
	if (gConfigGeneral.upper_case_banners) {
		format_string_to_upper(RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char), string_id, RCT2_ADDRESS(RCT2_ADDRESS_COMMON_FORMAT_ARGS, void));
	} else {
		format_string(RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char), string_id, RCT2_ADDRESS(RCT2_ADDRESS_COMMON_FORMAT_ARGS, void));
	}

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = 0x1C0;

	uint16 string_width = gfx_get_string_width(RCT2_ADDRESS(RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, char));
	uint16 scroll = (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) / 2) % string_width;

	sub_98199C(0, 0x15, scrolling_text_setup(string_id, scroll, scrollingMode), 0, height + 22, 1, 1, 0);
}

/**
 *
 *  rct2: 0x0068B3FB
 */
static void sub_68B3FB(int x, int y)
{
	rct_drawpixelinfo *dpi = RCT2_GLOBAL(0x0140E9A8, rct_drawpixelinfo*);

	RCT2_GLOBAL(0x141F56A, uint16_t) = 0;
	RCT2_GLOBAL(0x9E3138, uint8_t) = 0xFF;
	RCT2_GLOBAL(0x9E30B6, uint8_t) = 0xFF;
	RCT2_GLOBAL(0x9E323C, uint8_t) = 0xFF;
	RCT2_GLOBAL(0x9DE56A, uint16_t) = x;
	RCT2_GLOBAL(0x9DE56E, uint16_t) = y;
	RCT2_GLOBAL(0x9DE574, uint16_t) = x;
	RCT2_GLOBAL(0x9DE576, uint16_t) = y;

	rct_map_element* map_element = map_get_first_element_at(x >> 5, y >> 5);

	int dx = 0;
	switch (get_current_rotation()) {
	case 0:
		dx = x + y;
		break;
	case 1:
		x += 32;
		dx = y - x;
		break;
	case 2:
		x += 32;
		y += 32;
		dx = -(x + y);
		break;
	case 3:
		y += 32;
		dx = x - y;
		break;
	}
	dx >>= 1;
	// Display little yellow arrow when building footpaths?
	if ((RCT2_GLOBAL(RCT2_ADDRESS_MAP_SELECTION_FLAGS, uint16) & 4) &&
		RCT2_GLOBAL(0x9DE56A, uint16) == RCT2_GLOBAL(RCT2_ADDRESS_MAP_ARROW_X, uint16) &&
		RCT2_GLOBAL(0x9DE56E, uint16) == RCT2_GLOBAL(RCT2_ADDRESS_MAP_ARROW_Y, uint16)){
		uint8 arrowRotation =
			(get_current_rotation()
			+ (RCT2_GLOBAL(RCT2_ADDRESS_MAP_ARROW_DIRECTION, uint8) & 3)) & 3;

		uint32 imageId =
			arrowRotation +
			(RCT2_GLOBAL(RCT2_ADDRESS_MAP_ARROW_DIRECTION, uint8) & 0xFC) +
			0x20900C27;

		int arrowZ = RCT2_GLOBAL(RCT2_ADDRESS_MAP_ARROW_Z, uint16);

		RCT2_GLOBAL(0x9DE568, sint16) = x;
		RCT2_GLOBAL(0x9DE56C, sint16) = y;
		RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8) = VIEWPORT_INTERACTION_ITEM_NONE;
		RCT2_GLOBAL(0x9DEA52, uint16) = 0;
		RCT2_GLOBAL(0x9DEA54, uint16) = 0;
		RCT2_GLOBAL(0x9DEA56, uint16) = arrowZ + 18;

		sub_98197C(0, 0xFF, imageId, 0, arrowZ, 32, 32, get_current_rotation());
	}
	int bx = dx + 52;

	if (bx <= dpi->y)
		return;

	rct_map_element* element = map_element;//push map_element

	sint16 max_height = 0;
	do{
		max_height = max(max_height, element->clearance_height);
	} while (!map_element_is_last_for_tile(element++));

	element--;

	if (map_element_get_type(element) == MAP_ELEMENT_TYPE_SURFACE &&
		(element->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK) != 0){
		max_height = (element->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK) << 1;
	}

	max_height *= 8;

	dx -= max_height + 32;

	element = map_element;//pop map_element
	dx -= dpi->height;
	if (dx >= dpi->y)
		return;

	RCT2_GLOBAL(0x9DE568, sint16) = x;
	RCT2_GLOBAL(0x9DE56C, sint16) = y;
	RCT2_GLOBAL(0x9DE57C, uint16) = 0;
	do {
		int direction = (map_element->type + get_current_rotation()) & MAP_ELEMENT_DIRECTION_MASK;
		int height = map_element->base_height * 8;

		uint32_t dword_9DE574 = RCT2_GLOBAL(0x9DE574, uint32_t);
		RCT2_GLOBAL(0x9DE578, rct_map_element*) = map_element;
		//setup the painting of for example: the underground, signs, rides, scenery, etc.
		switch (map_element_get_type(map_element))
		{
		case MAP_ELEMENT_TYPE_SURFACE:
			RCT2_CALLPROC_X(0x66062C, 0, 0, direction, height, (int)map_element, 0, 0);
			break;
		case MAP_ELEMENT_TYPE_PATH:
			RCT2_CALLPROC_X(0x6A3590, 0, 0, direction, height, (int)map_element, 0, 0);
			break;
		case MAP_ELEMENT_TYPE_TRACK:
			viewport_track_paint_setup(direction, height, map_element);
			break;
		case MAP_ELEMENT_TYPE_SCENERY:
			RCT2_CALLPROC_X(0x6DFF47, 0, 0, direction, height, (int)map_element, 0, 0);
			break;
		case MAP_ELEMENT_TYPE_ENTRANCE:
			viewport_entrance_paint_setup(direction, height, map_element);
			break;
		case MAP_ELEMENT_TYPE_FENCE:
			RCT2_CALLPROC_X(0x6E44B0, 0, 0, direction, height, (int)map_element, 0, 0);
			break;
		case MAP_ELEMENT_TYPE_SCENERY_MULTIPLE:
			RCT2_CALLPROC_X(0x6B7F0C, 0, 0, direction, height, (int)map_element, 0, 0);
			break;
		case MAP_ELEMENT_TYPE_BANNER:
			viewport_banner_paint_setup(direction, height, map_element);
			break;
		default:
			// This is a little hack for taking care of undefined map_elements
			// 8cars MOM used a dirty version of this to skip drawing certain elements
			if (map_element_is_last_for_tile(map_element))
				return;
			map_element++;
			break;
		}
		RCT2_GLOBAL(0x9DE574, uint32_t) = dword_9DE574;
	} while (!map_element_is_last_for_tile(map_element++));
}

/**
 *
 *  rct2: 0x0068B60E
 */
static void viewport_blank_tiles_paint_setup(int x, int y)
{
	rct_drawpixelinfo *dpi = RCT2_GLOBAL(0x0140E9A8, rct_drawpixelinfo*);

	int dx;
	switch (get_current_rotation()) {
	case 0:
		dx = x + y;
		break;
	case 1:
		x += 32;
		dx = y - x;
		break;
	case 2:
		x += 32;
		y += 32;
		dx = -(x + y);
		break;
	case 3:
		y += 32;
		dx = x - y;
		break;
	}
	dx /= 2;
	dx -= 16;
	int bx = dx + 32;
	if (bx <= dpi->y) return;
	dx -= 20;
	dx -= dpi->height;
	if (dx >= dpi->y) return;
	RCT2_GLOBAL(0x9DE568, sint16) = x;
	RCT2_GLOBAL(0x9DE56C, sint16) = y;
	RCT2_GLOBAL(RCT2_ADDRESS_PAINT_SETUP_CURRENT_TYPE, uint8_t) = VIEWPORT_INTERACTION_ITEM_NONE;
	RCT2_CALLPROC_X(
		(int)RCT2_ADDRESS(0x98196C, uint32_t*)[get_current_rotation()],
		0xFF00,
		3123,
		y & 0xFF00,
		16,
		32,
		32,
		get_current_rotation()
	);
}

/**
 *
 *  rct2: 0x0068B2B7
 */
void sub_68B2B7(int x, int y)
{
	if (
		x < RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) &&
		y < RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) &&
		x >= 32 &&
		y >= 32
	) {
		RCT2_GLOBAL(0x0141E9B4, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9B8, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9BC, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9C0, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9C4, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9C8, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9CC, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9D0, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9D4, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9D8, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9DC, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9DB, uint8) |= 2;

		sub_68B3FB(x, y);
	} else {
		viewport_blank_tiles_paint_setup(x, y);
	}
}

/**
 *
 *  rct2: 0x0068B35F
 */
void map_element_paint_setup(int x, int y)
{
	rct_drawpixelinfo *dpi = RCT2_GLOBAL(0x0140E9A8, rct_drawpixelinfo*);
	if (
		x < RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) &&
		y < RCT2_GLOBAL(RCT2_ADDRESS_MAP_SIZE_UNITS, uint16) &&
		x >= 32 &&
		y >= 32
	) {
		RCT2_GLOBAL(0x0141E9B4, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9B8, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9BC, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9C0, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9C4, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9C8, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9CC, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9D0, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9D4, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9D8, uint32) = 0xFFFF;
		RCT2_GLOBAL(0x0141E9DC, uint32) = 0xFFFF;

		sub_68B3FB(x, y);
	} else {
		viewport_blank_tiles_paint_setup(x, y);
	}
}

/**
*
*  rct2: 0x0068B6C2
*/
void viewport_paint_setup()
{
	rct_drawpixelinfo* dpi = RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*);

	rct_xy16 mapTile = {
		.x = dpi->x & 0xFFE0,
		.y = (dpi->y - 16) & 0xFFE0
	};

	sint16 half_x = mapTile.x >> 1;

	uint16 num_vertical_quadrants = (dpi->height + 2128) >> 5;

	switch (get_current_rotation()){
	case 0:
		mapTile.x = mapTile.y - half_x;
		mapTile.y = mapTile.y + half_x;

		mapTile.x &= 0xFFE0;
		mapTile.y &= 0xFFE0;

		for (; num_vertical_quadrants > 0; --num_vertical_quadrants){
			map_element_paint_setup(mapTile.x, mapTile.y);
			sprite_paint_setup(mapTile.x, mapTile.y);

			sprite_paint_setup(mapTile.x - 32, mapTile.y + 32);

			map_element_paint_setup(mapTile.x, mapTile.y + 32);
			sprite_paint_setup(mapTile.x, mapTile.y + 32);

			mapTile.x += 32;
			sprite_paint_setup(mapTile.x, mapTile.y);

			mapTile.y += 32;
		}
		break;
	case 1:
		mapTile.x = -mapTile.y - half_x;
		mapTile.y = mapTile.y - half_x - 16;

		mapTile.x &= 0xFFE0;
		mapTile.y &= 0xFFE0;

		for (; num_vertical_quadrants > 0; --num_vertical_quadrants){
			map_element_paint_setup(mapTile.x, mapTile.y);
			sprite_paint_setup(mapTile.x, mapTile.y);

			sprite_paint_setup(mapTile.x - 32, mapTile.y - 32);

			map_element_paint_setup(mapTile.x - 32, mapTile.y);
			sprite_paint_setup(mapTile.x - 32, mapTile.y);

			mapTile.y += 32;
			sprite_paint_setup(mapTile.x, mapTile.y);

			mapTile.x -= 32;
		}
		break;
	case 2:
		mapTile.x = -mapTile.y + half_x;
		mapTile.y = -mapTile.y - half_x;

		mapTile.x &= 0xFFE0;
		mapTile.y &= 0xFFE0;

		for (; num_vertical_quadrants > 0; --num_vertical_quadrants){
			map_element_paint_setup(mapTile.x, mapTile.y);
			sprite_paint_setup(mapTile.x, mapTile.y);

			sprite_paint_setup(mapTile.x + 32, mapTile.y - 32);

			map_element_paint_setup(mapTile.x, mapTile.y - 32);
			sprite_paint_setup(mapTile.x, mapTile.y - 32);

			mapTile.x -= 32;

			sprite_paint_setup(mapTile.x, mapTile.y);

			mapTile.y -= 32;
		}
		break;
	case 3:
		mapTile.x = mapTile.y + half_x;
		mapTile.y = -mapTile.y + half_x - 16;

		mapTile.x &= 0xFFE0;
		mapTile.y &= 0xFFE0;

		for (; num_vertical_quadrants > 0; --num_vertical_quadrants){
			map_element_paint_setup(mapTile.x, mapTile.y);
			sprite_paint_setup(mapTile.x, mapTile.y);

			sprite_paint_setup(mapTile.x + 32, mapTile.y + 32);

			map_element_paint_setup(mapTile.x + 32, mapTile.y);
			sprite_paint_setup(mapTile.x + 32, mapTile.y);

			mapTile.y -= 32;

			sprite_paint_setup(mapTile.x, mapTile.y);

			mapTile.x += 32;
		}
		break;
	}
}

void sub_688217_helper(uint16 ax, uint8 flag)
{
	paint_struct *ps;
	paint_struct *ps_next = RCT2_GLOBAL(0x00EE7884, paint_struct*);

	do {
		ps = ps_next;
		ps_next = ps_next->next_quadrant_ps;
		if (ps_next == NULL) return;
	} while (ax > ps_next->var_18);

	RCT2_GLOBAL(0x00F1AD14, paint_struct*) = ps;

	do {
		ps = ps->next_quadrant_ps;
		if (ps == NULL) break;

		if (ps->var_18 > ax + 1) {
			ps->var_1B = 1 << 7;
		} else if (ps->var_18 == ax + 1) {
			ps->var_1B = (1 << 1) | (1 << 0);
		} else if (ps->var_18 == ax) {
			ps->var_1B = flag | (1 << 0);
		}
	} while (ps->var_18 <= ax + 1);

	ps = RCT2_GLOBAL(0x00F1AD14, paint_struct*);

	while (true) {
		while (true) {
			ps_next = ps->next_quadrant_ps;
			if (ps_next == NULL) return;
			if (ps_next->var_1B & (1 << 7)) return;
			if (ps_next->var_1B & (1 << 0)) break;
			ps = ps_next;
		}

		ps_next->var_1B &= ~(1 << 0);
		RCT2_GLOBAL(0x00F1AD18, paint_struct*) = ps;

		uint16 my_attached_x = ps_next->attached_x;
		uint16 my_attached_y = ps_next->attached_y;
		uint16 my_some_x = ps_next->some_x;
		uint16 my_some_y = ps_next->some_y;
		uint16 my_other_x = ps_next->other_x;
		uint16 my_other_y = ps_next->other_y;

		while (true) {
			ps = ps_next;
			ps_next = ps_next->next_quadrant_ps;
			if (ps_next == NULL) break;
			if (ps_next->var_1B & (1 << 7)) break;
			if (!(ps_next->var_1B & (1 << 1))) continue;

			int yes = 0;
			switch (get_current_rotation()) {
			case 0:
				if (my_some_y >= ps_next->some_x && my_other_y >= ps_next->attached_y && my_other_x >= ps_next->attached_x
					&& !(my_some_x < ps_next->some_y && my_attached_y < ps_next->other_y && my_attached_x < ps_next->other_x))
					yes = 1;
				break;
			case 1:
				if (my_some_y >= ps_next->some_x && my_other_y >= ps_next->attached_y && my_other_x < ps_next->attached_x
					&& !(my_some_x < ps_next->some_y && my_attached_y < ps_next->other_y && my_attached_x >= ps_next->other_x))
					yes = 1;
				break;
			case 2:
				if (my_some_y >= ps_next->some_x && my_other_y < ps_next->attached_y && my_other_x < ps_next->attached_x
					&& !(my_some_x < ps_next->some_y && my_attached_y >= ps_next->other_y && my_attached_x >= ps_next->other_x))
					yes = 1;
				break;
			case 3:
				if (my_some_y >= ps_next->some_x && my_other_y < ps_next->attached_y && my_other_x >= ps_next->attached_x
					&& !(my_some_x < ps_next->some_y && my_attached_y >= ps_next->other_y && my_attached_x < ps_next->other_x))
					yes = 1;
				break;
			}

			if (yes) {
				ps->next_quadrant_ps = ps_next->next_quadrant_ps;
				paint_struct *ps_temp = RCT2_GLOBAL(0x00F1AD18, paint_struct*)->next_quadrant_ps;
				RCT2_GLOBAL(0x00F1AD18, paint_struct*)->next_quadrant_ps = ps_next;
				ps_next->next_quadrant_ps = ps_temp;
				ps_next = ps;
			}
		}

		ps = RCT2_GLOBAL(0x00F1AD18, paint_struct*);
	}
}

/**
*
*  rct2: 0x00688217
*/
void sub_688217()
{
	paint_struct *ps = RCT2_GLOBAL(0x00EE7888, paint_struct*);
	paint_struct *ps_next;
	RCT2_GLOBAL(0x00EE7888, uint32) += 0x34; // 0x34 is size of paint_struct?
	RCT2_GLOBAL(0x00EE7884, paint_struct*) = ps;
	ps->next_quadrant_ps = NULL;
	uint32 edi = RCT2_GLOBAL(0x00F1AD0C, uint32);
	if (edi == -1)
		return;

	do {
		ps_next = RCT2_GLOBAL(0x00F1A50C + 4 * edi, paint_struct*);
		if (ps_next != NULL) {
			ps->next_quadrant_ps = ps_next;
			do {
				ps = ps_next;
				ps_next = ps_next->next_quadrant_ps;
			} while (ps_next != NULL);
		}
	} while (++edi <= RCT2_GLOBAL(0x00F1AD10, uint32));

	uint32 eax = RCT2_GLOBAL(0x00F1AD0C, uint32);

	sub_688217_helper(eax & 0xFFFF, 1 << 1);

	eax = RCT2_GLOBAL(0x00F1AD0C, uint32);

	while (++eax < RCT2_GLOBAL(0x00F1AD10, uint32))
		sub_688217_helper(eax & 0xFFFF, 0);
}

typedef struct paint_string_struct paint_string_struct;
struct paint_string_struct {
	rct_string_id string_id;		// 0x00
	paint_string_struct *next;		// 0x02
	uint16 x;						// 0x06
	uint16 y;						// 0x08
	uint8 args[16];					// 0x0A
	uint8 *y_offsets;				// 0x1A
};

static void draw_pixel_info_crop_by_zoom(rct_drawpixelinfo *dpi)
{
	int zoom = dpi->zoom_level;
	dpi->zoom_level = 0;
	dpi->x >>= zoom;
	dpi->y >>= zoom;
	dpi->width >>= zoom;
	dpi->height >>= zoom;
}

/**
 *
 *  rct2:0x006860C3
 */
static void viewport_draw_money_effects()
{
	utf8 buffer[256];

	paint_string_struct *ps = RCT2_GLOBAL(0x00F1AD20, paint_string_struct*);
	if (ps == NULL)
		return;

	rct_drawpixelinfo dpi = *(RCT2_GLOBAL(0x0140E9A8, rct_drawpixelinfo*));
	draw_pixel_info_crop_by_zoom(&dpi);

	do {
		format_string(buffer, ps->string_id, &ps->args);
		RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, uint16) = FONT_SPRITE_BASE_MEDIUM;
		gfx_draw_string_with_y_offsets(&dpi, buffer, 0, ps->x, ps->y, (sint8 *)ps->y_offsets);
	} while ((ps = ps->next) != NULL);
}

/**
 *
 *  rct2:0x00685CBF
 *  eax: left
 *  ebx: top
 *  edx: right
 *  esi: viewport
 *  edi: dpi
 *  ebp: bottom
 */
void viewport_paint(rct_viewport* viewport, rct_drawpixelinfo* dpi, int left, int top, int right, int bottom){
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) = viewport->flags;
	RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_ZOOM, uint16) = viewport->zoom;

	uint16 width = right - left;
	uint16 height = bottom - top;
	uint16 bitmask = 0xFFFF & (0xFFFF << viewport->zoom);

	width &= bitmask;
	height &= bitmask;
	left &= bitmask;
	top &= bitmask;

	RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_X, sint16) = left;
	RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_Y, sint16) = top;
	RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_WIDTH, uint16) = width;
	RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_HEIGHT, uint16) = height;

	width >>= viewport->zoom;

	RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_PITCH, uint16) = (dpi->width + dpi->pitch) - width;

	sint16 x = (sint16)(left - (sint16)(viewport->view_x & bitmask));
	x >>= viewport->zoom;
	x += viewport->x;

	sint16 y = (sint16)(top - (sint16)(viewport->view_y & bitmask));
	y >>= viewport->zoom;
	y += viewport->y;

	uint8* bits_pointer = x - dpi->x + (y - dpi->y)*(dpi->width + dpi->pitch) + dpi->bits;
	RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_BITS_PTR, uint8*) = bits_pointer;

	rct_drawpixelinfo* dpi2 = RCT2_ADDRESS(RCT2_ADDRESS_VIEWPORT_DPI, rct_drawpixelinfo);
	dpi2->y = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_Y, sint16);
	dpi2->height = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_HEIGHT, uint16);
	dpi2->zoom_level = (uint8)RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_ZOOM, uint16);

	//Splits the screen into 32 pixel columns and renders them.
	for (x = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_X, sint16) & 0xFFFFFFE0;
		x < RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_X, sint16) + RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_WIDTH, uint16);
		x += 32){

		int start_x = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_X, sint16);
		int width_col = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_WIDTH, uint16);
		bits_pointer = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_BITS_PTR, uint8*);
		int pitch = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_PITCH, uint16);
		int zoom = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_ZOOM, uint16);
		if (x >= start_x){
			int left_pitch = x - start_x;
			width_col -= left_pitch;
			bits_pointer += left_pitch >> zoom;
			pitch += left_pitch >> zoom;
			start_x = x;
		}

		int paint_right = start_x + width_col;
		if (paint_right >= x + 32){
			int right_pitch = paint_right - x - 32;
			paint_right -= right_pitch;
			pitch += right_pitch >> zoom;
		}
		width_col = paint_right - start_x;
		dpi2->x = start_x;
		dpi2->width = width_col;
		dpi2->bits = bits_pointer;
		dpi2->pitch = pitch;

		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & (VIEWPORT_FLAG_HIDE_VERTICAL | VIEWPORT_FLAG_HIDE_BASE | VIEWPORT_FLAG_UNDERGROUND_INSIDE)){
			uint8 colour = 0x0A;
			if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_INVISIBLE_SPRITES){
				colour = 0;
			}
			gfx_clear(dpi2, colour);
		}
		RCT2_GLOBAL(0xEE7880, uint32) = 0xF1A4CC;
		RCT2_GLOBAL(0x140E9A8, uint32) = (int)dpi2;
		int ebp = 0, ebx = 0, esi = 0, ecx = 0;
		painter_setup();
		viewport_paint_setup();
		sub_688217();
		sub_688485();

		int weather_colour = RCT2_ADDRESS(0x98195C, uint32)[RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER_GLOOM, uint8)];
		if ((weather_colour != -1) && (!(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_VIEWPORT_FLAGS, uint16) & VIEWPORT_FLAG_INVISIBLE_SPRITES)) && (!(RCT2_GLOBAL(0x9DEA6F, uint8) & 1))){
			gfx_fill_rect(dpi2, dpi2->x, dpi2->y, dpi2->width + dpi2->x - 1, dpi2->height + dpi2->y - 1, weather_colour);
		}
		viewport_draw_money_effects();
	}
}

/**
 *
 *  rct2: 0x0068958D
 */
void screen_pos_to_map_pos(sint16 *x, sint16 *y, int *direction)
{
	screen_get_map_xy(*x, *y, x, y, NULL);
	if (*x == (sint16)0x8000)
		return;

	int my_direction;
	int dist_from_center_x = abs(*x % 32);
	int dist_from_center_y = abs(*y % 32);
	if (dist_from_center_x > 8 && dist_from_center_x < 24 &&
		dist_from_center_y > 8 && dist_from_center_y < 24) {
		my_direction = 4;
	} else {
		sint16 mod_x = *x & 0x1F;
		sint16 mod_y = *y & 0x1F;
		if (mod_x <= 16) {
			if (mod_y < 16) {
				my_direction = 2;
			} else {
				my_direction = 3;
			}
		} else {
			if (mod_y < 16) {
				my_direction = 1;
			} else {
				my_direction = 0;
			}
		}
	}

	*x = *x & ~0x1F;
	*y = *y & ~0x1F;
	if (direction != NULL) *direction = my_direction;
}

rct_xy16 screen_coord_to_viewport_coord(rct_viewport *viewport, uint16 x, uint16 y)
{
	rct_xy16 ret;
	ret.x = ((x - viewport->x) << viewport->zoom) + viewport->view_x;
	ret.y = ((y - viewport->y) << viewport->zoom) + viewport->view_y;
	return ret;
}

rct_xy16 viewport_coord_to_map_coord(int x, int y, int z)
{
	rct_xy16 ret;
	switch (get_current_rotation()) {
	case 0:
		ret.x = -x / 2 + y + z;
		ret.y = x / 2 + y + z;
		break;
	case 1:
		ret.x = -x / 2 - y - z;
		ret.y = -x / 2 + y + z;
		break;
	case 2:
		ret.x = x / 2 - y - z;
		ret.y = -x / 2 - y - z;
		break;
	case 3:
		ret.x = x / 2 + y + z;
		ret.y = x / 2 - y - z;
		break;
	}
	return ret;
}

/**
 *
 *  rct2: 0x00664689
 */
void show_gridlines()
{
	rct_window *mainWindow;

	if (RCT2_GLOBAL(0x009E32B0, uint8) == 0) {
		if ((mainWindow = window_get_main()) != NULL) {
			if (!(mainWindow->viewport->flags & VIEWPORT_FLAG_GRIDLINES)) {
				mainWindow->viewport->flags |= VIEWPORT_FLAG_GRIDLINES;
				window_invalidate(mainWindow);
			}
		}
	}
	RCT2_GLOBAL(0x009E32B0, uint8)++;
}

/**
 *
 *  rct2: 0x006646B4
 */
void hide_gridlines()
{
	rct_window *mainWindow;

	RCT2_GLOBAL(0x009E32B0, uint8)--;
	if (RCT2_GLOBAL(0x009E32B0, uint8) == 0) {
		if ((mainWindow = window_get_main()) != NULL) {
			if (!gConfigGeneral.always_show_gridlines) {
				mainWindow->viewport->flags &= ~VIEWPORT_FLAG_GRIDLINES;
				window_invalidate(mainWindow);
			}
		}
	}
}

/**
 *
 *  rct2: 0x00664E8E
 */
void show_land_rights()
{
	rct_window *mainWindow;

	if (RCT2_GLOBAL(0x009E32B2, uint8) == 0) {
		if ((mainWindow = window_get_main()) != NULL) {
			if (!(mainWindow->viewport->flags & VIEWPORT_FLAG_LAND_OWNERSHIP)) {
				mainWindow->viewport->flags |= VIEWPORT_FLAG_LAND_OWNERSHIP;
				window_invalidate(mainWindow);
			}
		}
	}
	RCT2_GLOBAL(0x009E32B2, uint8)++;
}

/**
 *
 *  rct2: 0x00664EB9
 */
void hide_land_rights()
{
	rct_window *mainWindow;

	RCT2_GLOBAL(0x009E32B2, uint8)--;
	if (RCT2_GLOBAL(0x009E32B2, uint8) == 0) {
		if ((mainWindow = window_get_main()) != NULL) {
			if (mainWindow->viewport->flags & VIEWPORT_FLAG_LAND_OWNERSHIP) {
				mainWindow->viewport->flags &= ~VIEWPORT_FLAG_LAND_OWNERSHIP;
				window_invalidate(mainWindow);
			}
		}
	}
}

/**
 *
 *  rct2: 0x00664EDD
 */
void show_construction_rights()
{
	rct_window *mainWindow;

	if (RCT2_GLOBAL(0x009E32B3, uint8) == 0) {
		if ((mainWindow = window_get_main()) != NULL) {
			if (!(mainWindow->viewport->flags & VIEWPORT_FLAG_CONSTRUCTION_RIGHTS)) {
				mainWindow->viewport->flags |= VIEWPORT_FLAG_CONSTRUCTION_RIGHTS;
				window_invalidate(mainWindow);
			}
		}
	}
	RCT2_GLOBAL(0x009E32B3, uint8)++;
}

/**
 *
 *  rct2: 0x00664F08
 */
void hide_construction_rights()
{
	rct_window *mainWindow;

	RCT2_GLOBAL(0x009E32B3, uint8)--;
	if (RCT2_GLOBAL(0x009E32B3, uint8) == 0) {
		if ((mainWindow = window_get_main()) != NULL) {
			if (mainWindow->viewport->flags & VIEWPORT_FLAG_CONSTRUCTION_RIGHTS) {
				mainWindow->viewport->flags &= ~VIEWPORT_FLAG_CONSTRUCTION_RIGHTS;
				window_invalidate(mainWindow);
			}
		}
	}
}

/**
 *
 * rct2: 0x006CB70A
 */
void viewport_set_visibility(uint8 mode)
{
	rct_window* window = window_get_main();

	if (window != NULL) {
		rct_viewport* edi = window->viewport;
		uint32 invalidate = 0;

		switch (mode) {
		case 0: { //Set all these flags to 0, and invalidate if any were active
			uint16 mask = VIEWPORT_FLAG_UNDERGROUND_INSIDE | VIEWPORT_FLAG_SEETHROUGH_RIDES |
				VIEWPORT_FLAG_SEETHROUGH_SCENERY | VIEWPORT_FLAG_INVISIBLE_SUPPORTS |
				VIEWPORT_FLAG_LAND_HEIGHTS | VIEWPORT_FLAG_TRACK_HEIGHTS |
				VIEWPORT_FLAG_PATH_HEIGHTS | VIEWPORT_FLAG_INVISIBLE_PEEPS |
				VIEWPORT_FLAG_HIDE_BASE | VIEWPORT_FLAG_HIDE_VERTICAL;

			invalidate += edi->flags & mask;
			edi->flags &= ~mask;
			break;
		}
		case 1: //6CB79D
		case 4: //6CB7C4
			//Set underground on, invalidate if it was off
			invalidate += !(edi->flags & VIEWPORT_FLAG_UNDERGROUND_INSIDE);
			edi->flags |= VIEWPORT_FLAG_UNDERGROUND_INSIDE;
			break;
		case 2: //6CB7EB
			//Set track heights on, invalidate if off
			invalidate += !(edi->flags & VIEWPORT_FLAG_TRACK_HEIGHTS);
			edi->flags |= VIEWPORT_FLAG_TRACK_HEIGHTS;
			break;
		case 3: //6CB7B1
		case 5: //6CB7D8
			//Set underground off, invalidate if it was on
			invalidate += edi->flags & VIEWPORT_FLAG_UNDERGROUND_INSIDE;
			edi->flags &= ~((uint16)VIEWPORT_FLAG_UNDERGROUND_INSIDE);
			break;
		}
		if (invalidate != 0)
			window_invalidate(window);
	}
}

/**
 * Stores some info about the element pointed at, if requested for this particular type through the interaction mask.
 * rct2: 0x00688697
 */
void store_interaction_info(paint_struct *ps)
{
	if (RCT2_GLOBAL(0x0141F569, uint8) == 0) return;

	if (ps->sprite_type == VIEWPORT_INTERACTION_ITEM_NONE
		|| ps->sprite_type == 11 // 11 as a type seems to not exist, maybe part of the typo mentioned later on.
		|| ps->sprite_type > VIEWPORT_INTERACTION_ITEM_BANNER) return;

	uint16 mask;
	if (ps->sprite_type == VIEWPORT_INTERACTION_ITEM_BANNER)
		// I think CS made a typo here. Let's replicate the original behaviour.
		mask = 1 << (ps->sprite_type - 3);
	else
		mask = 1 << (ps->sprite_type - 1);

	if (!(RCT2_GLOBAL(0x009AC154, uint16) & mask)) {
		RCT2_GLOBAL(0x009AC148, uint8) = ps->sprite_type;
		RCT2_GLOBAL(0x009AC149, uint8) = ps->var_29;
		RCT2_GLOBAL(0x009AC14C, uint32) = ps->map_x;
		RCT2_GLOBAL(0x009AC14E, uint32) = ps->map_y;
		RCT2_GLOBAL(0x009AC150, rct_map_element*) = ps->mapElement;
	}
}

/**
 *
 *  rct2: 0x00679074
 */
void sub_679074(rct_drawpixelinfo *dpi, int imageId, int x, int y)
{
	RCT2_CALLPROC_X(0x00679074, 0, imageId, x, y, 0, (int)dpi, 0);
}

/**
 *
 *  rct2: 0x00679023
 */
void sub_679023(rct_drawpixelinfo *dpi, int imageId, int x, int y)
{
	RCT2_GLOBAL(0x00141F569, uint8) = 0;
	imageId &= 0xBFFFFFFF;
	if (imageId & 0x20000000) {
		RCT2_GLOBAL(0x00EDF81C, uint32) = 0x20000000;
		int index = (imageId >> 19) & 0x7F;
		if (imageId & 0x80000000) {
			index &= 0x1F;
		}
		int g1Index = RCT2_ADDRESS(0x0097FCBC, uint32)[index];
		RCT2_GLOBAL(0x009ABDA4, uint8*) = g1Elements[g1Index].offset;
	} else {
		RCT2_GLOBAL(0x00EDF81C, uint32) = 0;
	}
	sub_679074(dpi, imageId, x, y);
}

/**
 *
 *  rct2: 0x0068862C
 */
void sub_68862C()
{
	rct_drawpixelinfo *dpi = RCT2_GLOBAL(0x0140E9A8, rct_drawpixelinfo*);
	paint_struct *ps = RCT2_GLOBAL(0x00EE7884, paint_struct*), *old_ps, *next_ps, *attached_ps;

	while ((ps = ps->next_quadrant_ps) != NULL) {
		old_ps = ps;

		next_ps = ps;
		while (next_ps != NULL) {
			ps = next_ps;
			sub_679023(dpi, ps->image_id, ps->x, ps->y);
			store_interaction_info(ps);

			next_ps = ps->var_20;
		}

		attached_ps = ps->attached_ps;
		while (attached_ps != NULL) {
			sub_679023(
				dpi,
				attached_ps->image_id,
				(attached_ps->attached_x + ps->x) & 0xFFFF,
				(attached_ps->attached_y + ps->y) & 0xFFFF
			);
			store_interaction_info(ps);

			attached_ps = attached_ps->next_attached_ps;
		}

		ps = old_ps;
	}
}

/**
 *
 *  rct2: 0x00685ADC
 * screenX: eax
 * screenY: ebx
 * flags: edx
 * x: ax
 * y: cx
 * interactionType: bl
 * mapElement: edx
 * viewport: edi
 */
void get_map_coordinates_from_pos(int screenX, int screenY, int flags, sint16 *x, sint16 *y, int *interactionType, rct_map_element **mapElement, rct_viewport **viewport)
{
	RCT2_GLOBAL(0x9AC154, uint16_t) = flags & 0xFFFF;
	RCT2_GLOBAL(0x9AC148, uint8_t) = 0;
	rct_window* window = window_find_from_point(screenX, screenY);
	if (window != NULL && window->viewport != NULL)
	{
		rct_viewport* myviewport = window->viewport;
		RCT2_GLOBAL(0x9AC138 + 4, int16_t) = screenX;
		RCT2_GLOBAL(0x9AC138 + 6, int16_t) = screenY;
		screenX -= (int)myviewport->x;
		screenY -= (int)myviewport->y;
		if (screenX >= 0 && screenX < (int)myviewport->width && screenY >= 0 && screenY < (int)myviewport->height)
		{
			screenX <<= myviewport->zoom;
			screenY <<= myviewport->zoom;
			screenX += (int)myviewport->view_x;
			screenY += (int)myviewport->view_y;
			RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_ZOOM, uint16_t) = myviewport->zoom;
			screenX &= (0xFFFF << myviewport->zoom) & 0xFFFF;
			screenY &= (0xFFFF << myviewport->zoom) & 0xFFFF;
			RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_X, int16_t) = screenX;
			RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_Y, int16_t) = screenY;
			rct_drawpixelinfo* dpi = RCT2_ADDRESS(RCT2_ADDRESS_VIEWPORT_DPI, rct_drawpixelinfo);
			dpi->y = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_Y, int16_t);
			dpi->height = 1;
			dpi->zoom_level = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_ZOOM, uint16_t);
			dpi->x = RCT2_GLOBAL(RCT2_ADDRESS_VIEWPORT_PAINT_X, int16_t);
			dpi->width = 1;
			RCT2_GLOBAL(0xEE7880, uint32_t) = 0xF1A4CC;
			RCT2_GLOBAL(0x140E9A8, rct_drawpixelinfo*) = dpi;
			painter_setup();
			viewport_paint_setup();
			sub_688217();
			sub_68862C();
		}
		if (viewport != NULL) *viewport = myviewport;
	}
	if (interactionType != NULL) *interactionType = RCT2_GLOBAL(0x9AC148, uint8_t);
	if (x != NULL) *x = RCT2_GLOBAL(0x9AC14C, int16_t);
	if (y != NULL) *y = RCT2_GLOBAL(0x9AC14E, int16_t);
	if (mapElement != NULL) *mapElement = RCT2_GLOBAL(0x9AC150, rct_map_element*);
}

/**
 * Left, top, right and bottom represent 2D map coordinates at zoom 0.
 */
void viewport_invalidate(rct_viewport *viewport, int left, int top, int right, int bottom)
{
	int viewportLeft = viewport->view_x;
	int viewportTop = viewport->view_y;
	int viewportRight = viewport->view_x + viewport->view_width;
	int viewportBottom = viewport->view_y + viewport->view_height;
	if (right > viewportLeft && bottom > viewportTop) {
		left = max(left, viewportLeft);
		top = max(top, viewportTop);
		right = min(right, viewportRight);
		bottom = min(bottom, viewportBottom);

		uint8 zoom = 1 << viewport->zoom;
		left -= viewportLeft;
		top -= viewportTop;
		right -= viewportLeft;
		bottom -= viewportTop;
		left /= zoom;
		top /= zoom;
		right /= zoom;
		bottom /= zoom;
		left += viewport->x;
		top += viewport->y;
		right += viewport->x;
		bottom += viewport->y;
		gfx_set_dirty_blocks(left, top, right, bottom);
	}
}

rct_viewport *viewport_find_from_point(int screenX, int screenY)
{
	rct_window *w;
	rct_viewport *viewport;

	w = window_find_from_point(screenX, screenY);
	if (w == NULL)
		return NULL;

	viewport = w->viewport;
	if (viewport == NULL)
		return NULL;

	if (screenX < viewport->x || screenY < viewport->y)
		return NULL;
	if (screenX >= viewport->x + viewport->width || screenY >= viewport->y + viewport->height)
		return NULL;

	return viewport;
}

/**
 *
 *  rct2: 0x00688972
 *  In:
 *		screen_x: eax
 *		screen_y: ebx
 *  Out:
 *		x: ax
 *		y: bx
 *		map_element: edx ?
 *		viewport: edi
 */
void screen_get_map_xy(int screenX, int screenY, sint16 *x, sint16 *y, rct_viewport **viewport) {
	sint16 my_x, my_y;
	int z, interactionType;
	rct_viewport *myViewport;
	get_map_coordinates_from_pos(screenX, screenY, VIEWPORT_INTERACTION_MASK_TERRAIN, &my_x, &my_y, &interactionType, NULL, &myViewport);
	if (interactionType == VIEWPORT_INTERACTION_ITEM_NONE) {
		*x = 0x8000;
		return;
	}

	RCT2_GLOBAL(0x00F1AD34, sint16) = my_x;
	RCT2_GLOBAL(0x00F1AD36, sint16) = my_y;
	RCT2_GLOBAL(0x00F1AD38, sint16) = my_x + 31;
	RCT2_GLOBAL(0x00F1AD3A, sint16) = my_y + 31;

	rct_xy16 start_vp_pos = screen_coord_to_viewport_coord(myViewport, screenX, screenY);
	rct_xy16 map_pos = { my_x + 16, my_y + 16 };

	for (int i = 0; i < 5; i++) {
		z = map_element_height(map_pos.x, map_pos.y);
		map_pos = viewport_coord_to_map_coord(start_vp_pos.x, start_vp_pos.y, z);
		map_pos.x = clamp(RCT2_GLOBAL(0x00F1AD34, sint16), map_pos.x, RCT2_GLOBAL(0x00F1AD38, sint16));
		map_pos.y = clamp(RCT2_GLOBAL(0x00F1AD36, sint16), map_pos.y, RCT2_GLOBAL(0x00F1AD3A, sint16));
	}

	*x = map_pos.x;
	*y = map_pos.y;

	if (viewport != NULL) *viewport = myViewport;
}

/**
 *
 *  rct2: 0x006894D4
 */
void screen_get_map_xy_with_z(sint16 screenX, sint16 screenY, sint16 z, sint16 *mapX, sint16 *mapY)
{
	rct_viewport *viewport = viewport_find_from_point(screenX, screenY);
	if (viewport == NULL) {
		*mapX = 0x8000;
		return;
	}

	screenX = viewport->view_x + ((screenX - viewport->x) << viewport->zoom);
	screenY = viewport->view_y + ((screenY - viewport->y) << viewport->zoom);

	rct_xy16 mapPosition = viewport_coord_to_map_coord(screenX, screenY + z, 0);
	if (mapPosition.x < 0 || mapPosition.x >= (256 * 32) || mapPosition.y < 0 || mapPosition.y >(256 * 32)) {
		*mapX = 0x8000;
		return;
	}

	*mapX = mapPosition.x;
	*mapY = mapPosition.y;
}

/**
 *
 *  rct2: 0x00689604
 */
void screen_get_map_xy_quadrant(sint16 screenX, sint16 screenY, sint16 *mapX, sint16 *mapY, uint8 *quadrant)
{
	screen_get_map_xy(screenX, screenY, mapX, mapY, NULL);
	if (*mapX == (sint16)0x8000)
		return;

	*quadrant = map_get_tile_quadrant(*mapX, *mapY);
	*mapX = floor2(*mapX, 32);
	*mapY = floor2(*mapY, 32);
}

/**
 *
 *  rct2: 0x0068964B
 */
void screen_get_map_xy_quadrant_with_z(sint16 screenX, sint16 screenY, sint16 z, sint16 *mapX, sint16 *mapY, uint8 *quadrant)
{
	screen_get_map_xy_with_z(screenX, screenY, z, mapX, mapY);
	if (*mapX == (sint16)0x8000)
		return;

	*quadrant = map_get_tile_quadrant(*mapX, *mapY);
	*mapX = floor2(*mapX, 32);
	*mapY = floor2(*mapY, 32);
}

/**
 *
 *  rct2: 0x00689692
 */
void screen_get_map_xy_side(sint16 screenX, sint16 screenY, sint16 *mapX, sint16 *mapY, uint8 *side)
{
	screen_get_map_xy(screenX, screenY, mapX, mapY, NULL);
	if (*mapX == (sint16)0x8000)
		return;

	*side = map_get_tile_side(*mapX, *mapY);
	*mapX = floor2(*mapX, 32);
	*mapY = floor2(*mapY, 32);
}

/**
 *
 *  rct2: 0x006896DC
 */
void screen_get_map_xy_side_with_z(sint16 screenX, sint16 screenY, sint16 z, sint16 *mapX, sint16 *mapY, uint8 *side)
{
	screen_get_map_xy_with_z(screenX, screenY, z, mapX, mapY);
	if (*mapX == (sint16)0x8000)
		return;

	*side = map_get_tile_side(*mapX, *mapY);
	*mapX = floor2(*mapX, 32);
	*mapY = floor2(*mapY, 32);
}

/**
 * Get current viewport rotation.
 *
 * If an invalid rotation is detected and DEBUG_LEVEL_1 is enabled, an error
 * will be reported.
 *
 * @returns rotation in range 0-3 (inclusive)
 */
uint8 get_current_rotation()
{
	uint32 rotation = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_ROTATION, uint32);
	uint32 rotation_masked = rotation & 3;
#if DEBUG_LEVEL_1
	if (rotation != rotation_masked) {
	    log_error("Found wrong rotation %d! Will return %d instead.", rotation, rotation_masked);
	}
#endif // DEBUG_LEVEL_1
	return (uint8)rotation_masked;
}
