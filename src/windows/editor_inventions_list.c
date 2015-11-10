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
#include "../cursors.h"
#include "../input.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "../management/research.h"
#include "../object.h"
#include "../world/scenery.h"
#include "../interface/themes.h"
#include "../rct1.h"

#pragma region Widgets

enum {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_PAGE_BACKGROUND,
	WIDX_TAB_1,
	WIDX_PRE_RESEARCHED_SCROLL,
	WIDX_RESEARCH_ORDER_SCROLL,
	WIDX_PREVIEW,
	WIDX_RANDOM_SHUFFLE,
	WIDX_MOVE_ITEMS_TO_BOTTOM,
	WIDX_MOVE_ITEMS_TO_TOP
};

static rct_widget window_editor_inventions_list_widgets[] = {
	{ WWT_FRAME,			0,	0,		599,	0,		399,	STR_NONE,				STR_NONE				},
	{ WWT_CAPTION,			0,	1,		598,	1,		14,		STR_INVENTION_LIST,		STR_WINDOW_TITLE_TIP	},
	{ WWT_CLOSEBOX,			0,	587,	597,	2,		13,		STR_CLOSE_X,			STR_CLOSE_WINDOW_TIP	},
	{ WWT_RESIZE,			1,	0,		599,	43,		399,	STR_NONE,				STR_NONE				},
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x2000144E,				STR_NONE				},
	{ WWT_SCROLL,			1,	4,		371,	56,		175,	2,						STR_NONE				},
	{ WWT_SCROLL,			1,	4,		371,	189,	396,	2,						STR_NONE				},
	{ WWT_FLATBTN,			1,	431,	544,	106,	219,	0xFFFFFFFF,				STR_NONE				},
	{ WWT_DROPDOWN_BUTTON,	1,	375,	594,	385,	396,	STR_RANDOM_SHUFFLE,		STR_RANDOM_SHUFFLE_TIP	},
	{ WWT_DROPDOWN_BUTTON,	1,	375,	594,	372,	383,	2751,					STR_NONE				},
	{ WWT_DROPDOWN_BUTTON,	1,	375,	594,	359,	370,	2750,					STR_NONE				},
	{ WIDGETS_END }
};

static rct_widget window_editor_inventions_list_drag_widgets[] = {
	{ WWT_IMGBTN,			0,	0,		149,	0,		13,		STR_NONE,				STR_NONE				},
	{ WIDGETS_END }
};

#pragma endregion

#pragma region Events

static void window_editor_inventions_list_close(rct_window *w);
static void window_editor_inventions_list_mouseup(rct_window *w, int widgetIndex);
static void window_editor_inventions_list_update(rct_window *w);
static void window_editor_inventions_list_scrollgetheight(rct_window *w, int scrollIndex, int *width, int *height);
static void window_editor_inventions_list_scrollmousedown(rct_window *w, int scrollIndex, int x, int y);
static void window_editor_inventions_list_scrollmouseover(rct_window *w, int scrollIndex, int x, int y);
static void window_editor_inventions_list_tooltip(rct_window* w, int widgetIndex, rct_string_id *stringId);
static void window_editor_inventions_list_cursor(rct_window *w, int widgetIndex, int x, int y, int *cursorId);
static void window_editor_inventions_list_invalidate(rct_window *w);
static void window_editor_inventions_list_paint(rct_window *w, rct_drawpixelinfo *dpi);
static void window_editor_inventions_list_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex);

static void window_editor_inventions_list_drag_cursor(rct_window *w, int widgetIndex, int x, int y, int *cursorId);
static void window_editor_inventions_list_drag_moved(rct_window* w, int x, int y);
static void window_editor_inventions_list_drag_paint(rct_window *w, rct_drawpixelinfo *dpi);

// 0x0098177C
static rct_window_event_list window_editor_inventions_list_events = {
	window_editor_inventions_list_close,
	window_editor_inventions_list_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	window_editor_inventions_list_update,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_editor_inventions_list_scrollgetheight,
	window_editor_inventions_list_scrollmousedown,
	NULL,
	window_editor_inventions_list_scrollmouseover,
	NULL,
	NULL,
	NULL,
	window_editor_inventions_list_tooltip,
	window_editor_inventions_list_cursor,
	NULL,
	window_editor_inventions_list_invalidate,
	window_editor_inventions_list_paint,
	window_editor_inventions_list_scrollpaint
};

// 0x009817EC
static rct_window_event_list window_editor_inventions_list_drag_events = {
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
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	window_editor_inventions_list_drag_cursor,
	window_editor_inventions_list_drag_moved,
	NULL,
	window_editor_inventions_list_drag_paint,
	NULL
};

#pragma endregion

rct_research_item *_editorInventionsListDraggedItem;

#define WindowHighlightedItem(w) *((rct_research_item**)&(w->var_494))

static void window_editor_inventions_list_drag_open(rct_research_item *researchItem);
static void move_research_item(rct_research_item *beforeItem);

static int research_item_is_always_researched(rct_research_item *researchItem)
{
	return (researchItem->entryIndex & (RESEARCH_ENTRY_FLAG_RIDE_ALWAYS_RESEARCHED | RESEARCH_ENTRY_FLAG_SCENERY_SET_ALWAYS_RESEARCHED)) != 0;
}

/* rct2: 0x0068596F
 * Sets rides that are in use to be always researched
 */
static void research_rides_setup(){
	// Reset all objects to not required
	for (uint8 object_type = OBJECT_TYPE_RIDE; object_type < 11; object_type++){
		uint8* in_use = RCT2_ADDRESS(0x0098DA38, uint8*)[object_type];
		for (uint8 num_objects = object_entry_group_counts[object_type]; num_objects != 0; num_objects--){
			*in_use++ = 0;
		}
	}

	// Set research required for rides in use
	for (uint16 rideIndex = 0; rideIndex < 255; rideIndex++){
		rct_ride* ride = &g_ride_list[rideIndex];
		if (ride->type == RIDE_TYPE_NULL)continue;
		RCT2_ADDRESS(0x0098DA38, uint8*)[OBJECT_TYPE_RIDE][ride->subtype] |= 1;
	}

	for (rct_research_item* research = gResearchItems; research->entryIndex != RESEARCHED_ITEMS_END; research++){
		if (research->entryIndex & RESEARCH_ENTRY_FLAG_RIDE_ALWAYS_RESEARCHED)
			continue;

		// If not a ride
		if ((research->entryIndex & 0xFFFFFF) < 0x10000)
			continue;

		uint8 ride_base_type = (research->entryIndex >> 8) & 0xFF;

		uint8 object_index = research->entryIndex & 0xFF;
		rct_ride_type* ride_entry = GET_RIDE_ENTRY(object_index);

		uint8 master_found = 0;
		if (!(ride_entry->flags & RIDE_ENTRY_FLAG_SEPARATE_RIDE)){

			for (uint8 rideType = 0; rideType < object_entry_group_counts[OBJECT_TYPE_RIDE]; rideType++){
				rct_ride_type* master_ride = GET_RIDE_ENTRY(rideType);
				if (master_ride == NULL || (uint32)master_ride == 0xFFFFFFFF)
					continue;

				if (master_ride->flags & RIDE_ENTRY_FLAG_SEPARATE_RIDE)
					continue;

				// If master ride not in use
				if (!(RCT2_ADDRESS(0x0098DA38, uint8*)[OBJECT_TYPE_RIDE][rideType] & (1 << 0)))
					continue;

				if (ride_base_type == master_ride->ride_type[0] ||
					ride_base_type == master_ride->ride_type[1] ||
					ride_base_type == master_ride->ride_type[2]){
					master_found = 1;
					break;
				}
			}
		}

		if (!master_found){
			// If not in use
			if (!(RCT2_ADDRESS(0x0098DA38, uint8*)[OBJECT_TYPE_RIDE][object_index] & (1 << 0)))
				continue;
			if (ride_base_type != ride_entry->ride_type[0] &&
				ride_base_type != ride_entry->ride_type[1] &&
				ride_base_type != ride_entry->ride_type[2]){
				continue;
			}
		}

		research->entryIndex |= RESEARCH_ENTRY_FLAG_RIDE_ALWAYS_RESEARCHED;
		_editorInventionsListDraggedItem = research;
		move_research_item(gResearchItems);
		_editorInventionsListDraggedItem = NULL;
		research--;
	}
}

/* rct2: 0x0068590C
 * Sets the critical scenery sets to always researched
 */
static void research_scenery_sets_setup(){

	for (rct_object_entry* object = RCT2_ADDRESS(0x0098DA74, rct_object_entry);
		(object->flags & 0xFF) != 0xFF;
		object++){

		uint8 entry_type, entry_index;
		if (!find_object_in_entry_group(object, &entry_type, &entry_index))
			continue;

		if (entry_type != OBJECT_TYPE_SCENERY_SETS)
			continue;

		rct_research_item* research = gResearchItems;
		for (; research->entryIndex != RESEARCHED_ITEMS_END; research++){

			if ((research->entryIndex & 0xFFFFFF) != entry_index)
				continue;

			research->entryIndex |= RESEARCH_ENTRY_FLAG_SCENERY_SET_ALWAYS_RESEARCHED;
			_editorInventionsListDraggedItem = research;
			move_research_item(gResearchItems);
			_editorInventionsListDraggedItem = NULL;
		}
	}
}

/**
 *
 *  rct2: 0x00685901
 */
static void research_always_researched_setup()
{
	research_rides_setup();
	research_scenery_sets_setup();
}

/**
 *
 *  rct2: 0x00685A79
 */
static void sub_685A79()
{
	for (rct_research_item* research = gResearchItems;
		research->entryIndex != RESEARCHED_ITEMS_END_2;
		research++){

		// Clear the always researched flags.
		if (research->entryIndex > RESEARCHED_ITEMS_SEPARATOR){
			research->entryIndex &= 0x00FFFFFF;
		}
	}
}

/**
 *
 *  rct2: 0x0068563D
 */
static rct_string_id research_item_get_name(uint32 researchItem)
{
	rct_ride_type *rideEntry;
	rct_scenery_set_entry *sceneryEntry;

	if (researchItem < 0x10000) {
		sceneryEntry = g_scenerySetEntries[researchItem & 0xFF];
		if (sceneryEntry == NULL || sceneryEntry == (rct_scenery_set_entry*)0xFFFFFFFF)
			return 0;

		return sceneryEntry->name;
	}

	rideEntry = GET_RIDE_ENTRY(researchItem & 0xFF);
	if (rideEntry == NULL || rideEntry == (rct_ride_type*)0xFFFFFFFF)
		return 0;

	if (rideEntry->flags & RIDE_ENTRY_FLAG_SEPARATE_RIDE_NAME)
		return rideEntry->name;

	return ((researchItem >> 8) & 0xFF) + 2;
}

/**
 *
 *  rct2: 0x00685A93
 */
static void research_items_shuffle()
{
	rct_research_item *researchItem, *researchOrderBase, researchItemTemp;
	int i, ri, numNonResearchedItems;

	// Skip pre-researched items
	for (researchItem = gResearchItems; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR; researchItem++) {}
	researchItem++;
	researchOrderBase = researchItem;

	// Count non pre-researched items
	numNonResearchedItems = 0;
	for (; researchItem->entryIndex != RESEARCHED_ITEMS_END; researchItem++)
		numNonResearchedItems++;

	// Shuffle list
	for (i = 0; i < numNonResearchedItems; i++) {
		ri = rand() % numNonResearchedItems;
		if (ri == i)
			continue;

		researchItemTemp = researchOrderBase[i];
		researchOrderBase[i] = researchOrderBase[ri];
		researchOrderBase[ri] = researchItemTemp;
	}
}

static void research_items_make_all_unresearched()
{
	rct_research_item *researchItem, *nextResearchItem, researchItemTemp;

	int sorted;
	do {
		sorted = 1;
		for (researchItem = gResearchItems; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR; researchItem++) {
			if (research_item_is_always_researched(researchItem))
				continue;

			nextResearchItem = researchItem + 1;
			if (nextResearchItem->entryIndex == RESEARCHED_ITEMS_SEPARATOR || research_item_is_always_researched(nextResearchItem)) {
				// Bubble up always researched item or separator
				researchItemTemp = *researchItem;
				*researchItem = *nextResearchItem;
				*nextResearchItem = researchItemTemp;
				sorted = 0;

				if (researchItem->entryIndex == RESEARCHED_ITEMS_SEPARATOR)
					break;
			}
		}
	} while (!sorted);
}

static void research_items_make_all_researched()
{
	rct_research_item *researchItem, researchItemTemp;

	// Find separator
	for (researchItem = gResearchItems; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR; researchItem++) { }

	// Move separator below all items
	for (; (researchItem + 1)->entryIndex != RESEARCHED_ITEMS_END; researchItem++) {
		// Swap separator with research item
		researchItemTemp = *researchItem;
		*researchItem = *(researchItem + 1);
		*(researchItem + 1) = researchItemTemp;
	}
}

/**
 *
 *  rct2: 0x006855E7
 */
static void move_research_item(rct_research_item *beforeItem)
{
	rct_window *w;
	rct_research_item *researchItem, draggedItem;

	if (_editorInventionsListDraggedItem == beforeItem - 1)
		return;

	// Back up the dragged item
	draggedItem = *_editorInventionsListDraggedItem;

	// Remove dragged item from list
	researchItem = _editorInventionsListDraggedItem;
	do {
		*researchItem = *(researchItem + 1);
		researchItem++;
	} while (researchItem->entryIndex != RESEARCHED_ITEMS_END_2);
	// At end of this researchItem points to the end of the list

	if (beforeItem > _editorInventionsListDraggedItem)
		beforeItem--;

	// Add dragged item to list
	do {
		*researchItem = *(researchItem - 1);
		researchItem--;
	} while (researchItem != beforeItem);

	*researchItem = draggedItem;

	w = window_find_by_class(WC_EDITOR_INVENTION_LIST);
	if (w != NULL) {
		WindowHighlightedItem(w) = NULL;
		window_invalidate(w);
	}
}

/**
 *
 *  rct2: 0x0068558E
 */
static rct_research_item *window_editor_inventions_list_get_item_from_scroll_y(int scrollIndex, int y)
{
	rct_research_item *researchItem;

	researchItem = gResearchItems;

	if (scrollIndex != 0) {
		// Skip pre-researched items
		for (; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR; researchItem++) { }
		researchItem++;
	}

	for (; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR && researchItem->entryIndex != RESEARCHED_ITEMS_END; researchItem++) {
		y -= 10;
		if (y < 0)
			return researchItem;
	}

	return NULL;
}

/**
 *
 *  rct2: 0x006855BB
 */
static rct_research_item *window_editor_inventions_list_get_item_from_scroll_y_include_seps(int scrollIndex, int y)
{
	rct_research_item *researchItem;

	researchItem = gResearchItems;

	if (scrollIndex != 0) {
		// Skip pre-researched items
		for (; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR; researchItem++) { }
		researchItem++;
	}

	for (; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR && researchItem->entryIndex != RESEARCHED_ITEMS_END; researchItem++) {
		y -= 10;
		if (y < 0)
			return researchItem;
	}

	return researchItem;
}

static rct_research_item *get_research_item_at(int x, int y)
{
	rct_window *w;
	rct_widget *widget;
	int scrollY, outX, outY, outScrollArea, outScrollId;
	short widgetIndex;

	w = window_find_by_class(WC_EDITOR_INVENTION_LIST);
	if (w != NULL && w->x <= x && w->y < y && w->x + w->width > x && w->y + w->height > y) {
		widgetIndex = window_find_widget_from_point(w, x, y);
		widget = &w->widgets[widgetIndex];
		if (widgetIndex == WIDX_PRE_RESEARCHED_SCROLL || widgetIndex == WIDX_RESEARCH_ORDER_SCROLL) {
			RCT2_GLOBAL(RCT2_ADDRESS_CURSOR_DOWN_WIDGETINDEX, uint32) = widgetIndex;
			widget_scroll_get_part(w, widget, x, y, &outX, &outY, &outScrollArea, &outScrollId);
			if (outScrollArea == SCROLL_PART_VIEW) {
				outScrollId = outScrollId == 0 ? 0 : 1;

				scrollY = y - (w->y + widget->top) + w->scrolls[outScrollId].v_top + 5;
				return window_editor_inventions_list_get_item_from_scroll_y_include_seps(outScrollId, scrollY);
			}
		}
	}

	return NULL;
}

/**
 *
 *  rct2: 0x00684E04
 */
void window_editor_inventions_list_open()
{
	rct_window *w;

	w = window_bring_to_front_by_class(WC_EDITOR_INVENTION_LIST);
	if (w != NULL)
		return;

	research_always_researched_setup();

	w = window_create_centred(
		600,
		400,
		&window_editor_inventions_list_events,
		WC_EDITOR_INVENTION_LIST,
		WF_NO_SCROLLING
	);
	w->widgets = window_editor_inventions_list_widgets;
	w->enabled_widgets =
		(1 << WIDX_CLOSE) |
		(1 << WIDX_TAB_1) |
		(1 << WIDX_RANDOM_SHUFFLE) |
		(1 << WIDX_MOVE_ITEMS_TO_BOTTOM) |
		(1 << WIDX_MOVE_ITEMS_TO_TOP);
	window_init_scroll_widgets(w);
	w->var_4AE = 0;
	w->selected_tab = 0;
	WindowHighlightedItem(w) = NULL;
	_editorInventionsListDraggedItem = NULL;
}

/**
 *
 *  rct2: 0x006853D2
 */
static void window_editor_inventions_list_close(rct_window *w)
{
	// When used in-game (as a cheat)
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_EDITOR)) {
		gSilentResearch = true;
		sub_684AC3();
		gSilentResearch = false;
	}

	sub_685A79();
}

/**
 *
 *  rct2: 0x0068521B
 */
static void window_editor_inventions_list_mouseup(rct_window *w, int widgetIndex)
{
	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_RANDOM_SHUFFLE:
		research_items_shuffle();
		window_invalidate(w);
		break;
	case WIDX_MOVE_ITEMS_TO_TOP:
		research_items_make_all_researched();
		window_init_scroll_widgets(w);
		window_invalidate(w);
		break;
	case WIDX_MOVE_ITEMS_TO_BOTTOM:
		research_items_make_all_unresearched();
		window_init_scroll_widgets(w);
		window_invalidate(w);
		break;
	}
}

/**
 *
 *  rct2: 0x00685392
 */
static void window_editor_inventions_list_update(rct_window *w)
{
	w->frame_no++;
	window_event_invalidate_call(w);
	widget_invalidate(w, WIDX_TAB_1);

	if (_editorInventionsListDraggedItem == NULL)
		return;

	if (window_find_by_class(WC_EDITOR_INVENTION_LIST_DRAG) != NULL)
		return;

	_editorInventionsListDraggedItem = NULL;
	window_invalidate(w);
}

/**
 *
 *  rct2: 0x00685239
 */
static void window_editor_inventions_list_scrollgetheight(rct_window *w, int scrollIndex, int *width, int *height)
{
	rct_research_item *researchItem;

	*height = 0;

	// Count / skip pre-researched items
	for (researchItem = gResearchItems; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR; researchItem++)
		*height += 10;

	if (scrollIndex == 1) {
		researchItem++;

		// Count non pre-researched items
		*height = 0;
		for (; researchItem->entryIndex != RESEARCHED_ITEMS_END; researchItem++)
			*height += 10;
	}
}

/**
 *
 *  rct2: 0x006852D4
 */
static void window_editor_inventions_list_scrollmousedown(rct_window *w, int scrollIndex, int x, int y)
{
	rct_research_item *researchItem;

	researchItem = window_editor_inventions_list_get_item_from_scroll_y(scrollIndex, y);
	if (researchItem == NULL)
		return;

	if (researchItem->entryIndex < (uint32)RESEARCHED_ITEMS_END_2 && research_item_is_always_researched(researchItem))
		return;

	window_invalidate(w);
	window_editor_inventions_list_drag_open(researchItem);
}

/**
 *
 *  rct2: 0x00685275
 */
static void window_editor_inventions_list_scrollmouseover(rct_window *w, int scrollIndex, int x, int y)
{
	rct_research_item *researchItem;

	researchItem = window_editor_inventions_list_get_item_from_scroll_y(scrollIndex, y);
	if (researchItem != WindowHighlightedItem(w)) {
		WindowHighlightedItem(w) = researchItem;
		window_invalidate(w);
	}
}

/**
 *
 *  rct2: 0x0068526B
 */
static void window_editor_inventions_list_tooltip(rct_window* w, int widgetIndex, rct_string_id *stringId)
{
	RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint16) = 3159;
}

/**
 *
 *  rct2: 0x00685291
 */
static void window_editor_inventions_list_cursor(rct_window *w, int widgetIndex, int x, int y, int *cursorId)
{
	rct_research_item *researchItem;
	int scrollIndex;

	switch (widgetIndex) {
	case WIDX_PRE_RESEARCHED_SCROLL:
		scrollIndex = 0;
		break;
	case WIDX_RESEARCH_ORDER_SCROLL:
		scrollIndex = 1;
		break;
	default:
		return;
	}

	researchItem = window_editor_inventions_list_get_item_from_scroll_y(scrollIndex, y);
	if (researchItem == NULL)
		return;

	if (researchItem->entryIndex < (uint32)RESEARCHED_ITEMS_END_2 && research_item_is_always_researched(researchItem)) {
		return;
	}

	*cursorId = CURSOR_HAND_OPEN;
}

/**
 *
 *  rct2: 0x00685392
 */
static void window_editor_inventions_list_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	w->pressed_widgets |= 1 << WIDX_PREVIEW;
	w->pressed_widgets |= 1 << WIDX_TAB_1;

	w->widgets[WIDX_CLOSE].type =
		RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR ? WWT_EMPTY : WWT_CLOSEBOX;
}

/**
 *
 *  rct2: 0x00684EE0
 */
static void window_editor_inventions_list_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	rct_widget *widget;
	rct_research_item *researchItem;
	rct_string_id stringId;
	int x, y, width;

	window_draw_widgets(w, dpi);

	// Tab image
	x = w->x + w->widgets[WIDX_TAB_1].left;
	y = w->y + w->widgets[WIDX_TAB_1].top;
	gfx_draw_sprite(dpi, 5327 + (w->frame_no / 2) % 8, x, y, 0);

	// Pre-researched items label
	x = w->x + w->widgets[WIDX_PRE_RESEARCHED_SCROLL].left;
	y = w->y + w->widgets[WIDX_PRE_RESEARCHED_SCROLL].top - 11;
	gfx_draw_string_left(dpi, 3197, NULL, 0, x, y - 1);

	// Research order label
	x = w->x + w->widgets[WIDX_RESEARCH_ORDER_SCROLL].left;
	y = w->y + w->widgets[WIDX_RESEARCH_ORDER_SCROLL].top - 11;
	gfx_draw_string_left(dpi, 3198, NULL, 0, x, y - 1);

	// Preview background
	widget = &w->widgets[WIDX_PREVIEW];
	gfx_fill_rect(
		dpi,
		w->x + widget->left + 1,
		w->y + widget->top + 1,
		w->x + widget->right - 1,
		w->y + widget->bottom - 1,
		ColourMapA[w->colours[1]].darkest
	);

	researchItem = _editorInventionsListDraggedItem;
	if (researchItem == NULL)
		researchItem = WindowHighlightedItem(w);
	// If the research item is null or a list separator.
	if (researchItem == NULL || researchItem->entryIndex < 0)
		return;

	// Preview image
	x = w->x + ((widget->left + widget->right) / 2) + 1;
	y = w->y + ((widget->top + widget->bottom) / 2) + 1;

	int objectEntryType = 7;
	int eax = researchItem->entryIndex & 0xFFFFFF;
	if (eax >= 0x10000)
		objectEntryType = 0;

	void *chunk = object_entry_groups[objectEntryType].chunks[researchItem->entryIndex & 0xFF];

	if (chunk == NULL || chunk == (void*)0xFFFFFFFF)
		return;

	object_paint(objectEntryType, 3, objectEntryType, x, y, 0, (int)dpi, (int)chunk);

	// Item name
	x = w->x + ((widget->left + widget->right) / 2) + 1;
	y = w->y + widget->bottom + 3;
	width = w->width - w->widgets[WIDX_RESEARCH_ORDER_SCROLL].right - 6;
	stringId = research_item_get_name(eax);
	gfx_draw_string_centred_clipped(dpi, 1193, &stringId, 0, x, y, width);
	y += 15;

	// Item category
	x = w->x + w->widgets[WIDX_RESEARCH_ORDER_SCROLL].right + 4;
	stringId = 2253 + researchItem->category;
	gfx_draw_string_left(dpi, 3196, &stringId, 0, x, y);
}

/**
 *
 *  rct2: 0x006850BD
 */
static void window_editor_inventions_list_scrollpaint(rct_window *w, rct_drawpixelinfo *dpi, int scrollIndex)
{
	uint32 colour;
	rct_research_item *researchItem;
	int left, top, bottom, itemY, disableItemMovement;
	sint32 researchItemEndMarker;
	rct_string_id stringId;
	utf8 buffer[256], *ptr;

	// Draw background
	colour = ColourMapA[w->colours[1]].mid_light;
	colour = (colour << 24) | (colour << 16) | (colour << 8) | colour;
	gfx_clear(dpi, colour);

	researchItem = gResearchItems;

	if (scrollIndex == 1) {
		// Skip pre-researched items
		for (; researchItem->entryIndex != RESEARCHED_ITEMS_SEPARATOR; researchItem++) { }
		researchItem++;
		researchItemEndMarker = RESEARCHED_ITEMS_END;
	} else {
		researchItemEndMarker = RESEARCHED_ITEMS_SEPARATOR;
	}

	// Since this is now a do while need to conteract the +10
	itemY = -10;
	do{
		itemY += 10;
		if (itemY + 10 < dpi->y || itemY >= dpi->y + dpi->height)
			continue;

		colour = 142;
		if (WindowHighlightedItem(w) == researchItem) {
			if (_editorInventionsListDraggedItem == NULL) {
				// Highlight
				top = itemY;
				bottom = itemY + 9;
			} else {
				// Drop horizontal rule
				top = itemY - 1;
				bottom = itemY;
			}
			gfx_fill_rect(dpi, 0, top, w->width, bottom, 0x2000031);

			if (_editorInventionsListDraggedItem == NULL)
				colour = 14;
		}

		if (researchItem->entryIndex == RESEARCHED_ITEMS_SEPARATOR || researchItem->entryIndex == RESEARCHED_ITEMS_END)
			continue;

		if (researchItem == _editorInventionsListDraggedItem)
			continue;

		disableItemMovement = research_item_is_always_researched(researchItem);
		stringId = research_item_get_name(researchItem->entryIndex & 0xFFFFFF);

		ptr = buffer;
		if (!disableItemMovement) {
			ptr = utf8_write_codepoint(ptr, colour & 0xFF);
		}

		format_string(ptr, stringId, NULL);

		if (disableItemMovement) {
			RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, sint16) = -1;
			if ((colour & 0xFF) == 14 && _editorInventionsListDraggedItem == NULL)
				RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, sint16) = -2;
			colour = 64 | w->colours[1];
		} else {
			RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_FONT_SPRITE_BASE, sint16) = 224;
			colour = 0;
		}

		left = 1;
		top = itemY - 1;
		gfx_draw_string(dpi, buffer, colour, left, top);
	}while(researchItem++->entryIndex != researchItemEndMarker);
}

#pragma region Drag item

/**
 *
 *  rct2: 0x006852F4
 */
static void window_editor_inventions_list_drag_open(rct_research_item *researchItem)
{
	char buffer[256];
	int stringWidth;
	rct_string_id stringId;
	rct_window *w;

	window_close_by_class(WC_EDITOR_INVENTION_LIST_DRAG);
	_editorInventionsListDraggedItem = researchItem;

	stringId = research_item_get_name(researchItem->entryIndex & 0xFFFFFF);
	format_string(buffer, stringId, NULL);
	stringWidth = gfx_get_string_width(buffer);
	window_editor_inventions_list_drag_widgets[0].right = stringWidth;

	w = window_create(
		RCT2_GLOBAL(RCT2_ADDRESS_TOOLTIP_CURSOR_X, uint16) - (stringWidth / 2),
		RCT2_GLOBAL(RCT2_ADDRESS_TOOLTIP_CURSOR_Y, uint16) - 7,
		stringWidth,
		14,
		&window_editor_inventions_list_drag_events,
		WC_EDITOR_INVENTION_LIST_DRAG,
		WF_STICK_TO_FRONT | WF_TRANSPARENT | WF_NO_SNAPPING
	);
	w->widgets = window_editor_inventions_list_drag_widgets;
	w->colours[1] = 2;
	input_window_position_begin(
		w, 0, RCT2_GLOBAL(RCT2_ADDRESS_TOOLTIP_CURSOR_X, uint16), RCT2_GLOBAL(RCT2_ADDRESS_TOOLTIP_CURSOR_Y, uint16)
	);
}

/**
 *
 *  rct2: 0x0068549C
 */
static void window_editor_inventions_list_drag_cursor(rct_window *w, int widgetIndex, int x, int y, int *cursorId)
{
	rct_window *inventionListWindow;
	rct_research_item *researchItem;

	inventionListWindow = window_find_by_class(WC_EDITOR_INVENTION_LIST);
	if (inventionListWindow != NULL) {
		researchItem = get_research_item_at(x, y);
		if (researchItem != WindowHighlightedItem(inventionListWindow)) {
			WindowHighlightedItem(inventionListWindow) = researchItem;
			window_invalidate(inventionListWindow);
		}
	}

	*cursorId = CURSOR_HAND_CLOSED;
}

/**
 *
 *  rct2: 0x00685412
 */
static void window_editor_inventions_list_drag_moved(rct_window* w, int x, int y)
{
	rct_research_item *researchItem;

	researchItem = get_research_item_at(x, y);
	if (researchItem != NULL)
		move_research_item(researchItem);

	window_close(w);
	_editorInventionsListDraggedItem = NULL;
	window_invalidate_by_class(WC_EDITOR_INVENTION_LIST);
}

/**
 *
 *  rct2: 0x006853D9
 */
static void window_editor_inventions_list_drag_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	rct_string_id stringId;
	int x, y;

	x = w->x;
	y = w->y + 2;
	stringId = research_item_get_name(_editorInventionsListDraggedItem->entryIndex & 0xFFFFFF);
	gfx_draw_string_left(dpi, 1193, &stringId, 32, x, y);
}

#pragma endregion
