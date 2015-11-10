/*****************************************************************************
* Copyright (c) 2014 Maciek Baron
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
#include "../game.h"
#include "../interface/graph.h"
#include "../interface/widget.h"
#include "../interface/window.h"
#include "../localisation/date.h"
#include "../localisation/localisation.h"
#include "../management/finance.h"
#include "../management/marketing.h"
#include "../management/research.h"
#include "../ride/ride.h"
#include "../ride/ride_data.h"
#include "../scenario.h"
#include "../sprites.h"
#include "dropdown.h"
#include "../interface/themes.h"

enum {
	WINDOW_FINANCES_PAGE_SUMMARY,
	WINDOW_FINANCES_PAGE_FINANCIAL_GRAPH,
	WINDOW_FINANCES_PAGE_VALUE_GRAPH,
	WINDOW_FINANCES_PAGE_PROFIT_GRAPH,
	WINDOW_FINANCES_PAGE_MARKETING,
	WINDOW_FINANCES_PAGE_RESEARCH,
	WINDOW_FINANCES_PAGE_COUNT
};

enum {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_PAGE_BACKGROUND,
	WIDX_TAB_1,
	WIDX_TAB_2,
	WIDX_TAB_3,
	WIDX_TAB_4,
	WIDX_TAB_5,
	WIDX_TAB_6,

	WIDX_LOAN = 10,
	WIDX_LOAN_INCREASE,
	WIDX_LOAN_DECREASE,

	WIDX_ACITVE_CAMPAGINS_GROUP = 10,
	WIDX_CAMPAGINS_AVAILABLE_GROUP,
	WIDX_CAMPAIGN_1,
	WIDX_CAMPAIGN_2,
	WIDX_CAMPAIGN_3,
	WIDX_CAMPAIGN_4,
	WIDX_CAMPAIGN_5,
	WIDX_CAMPAIGN_6,

	WIDX_RESEARCH_FUNDING = 11,
	WIDX_RESEARCH_FUNDING_DROPDOWN_BUTTON,
	WIDX_TRANSPORT_RIDES = 14,
	WIDX_GENTLE_RIDES,
	WIDX_ROLLER_COASTERS,
	WIDX_THRILL_RIDES,
	WIDX_WATER_RIDES,
	WIDX_SHOPS_AND_STALLS,
	WIDX_SCENERY_AND_THEMING,
};

#pragma region Widgets

static rct_widget window_finances_summary_widgets[] = {
	{ WWT_FRAME,			0,	0,		529,	0,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_CAPTION,			0,	1,		528,	1,		14,		STR_FINANCIAL_SUMMARY,	STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,			0,	517,	527,	2,		13,		STR_CLOSE_X,			STR_CLOSE_WINDOW_TIP },
	{ WWT_RESIZE,			1,	0,		529,	43,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_SUMMARY_TAB_TIP },
	{ WWT_TAB,				1,	34,		64,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_CASH_TAB_TIP },
	{ WWT_TAB,				1,	65,		95,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_PARK_VALUE_TAB_TIP },
	{ WWT_TAB,				1,	96,		126,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_WEEKLY_PROFIT_TAB_TIP },
	{ WWT_TAB,				1,	127,	157,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_MARKETING_TAB_TIP },
	{ WWT_TAB,				1,	158,	188,	17,		43,		0x2000144E,				STR_FINANCES_RESEARCH_TIP },
	{ WWT_SPINNER,			1,	64,		153,	229,	240,	1917,					STR_NONE },
	{ WWT_DROPDOWN_BUTTON,	1,	142,	152,	230,	234,	STR_NUMERIC_UP,			STR_NONE },
	{ WWT_DROPDOWN_BUTTON,	1,	142,	152,	235,	239,	STR_NUMERIC_DOWN,		STR_NONE },
	{ WIDGETS_END },
};

static rct_widget window_finances_cash_widgets[] = {
	{ WWT_FRAME,			0,	0,		529,	0,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_CAPTION,			0,	1,		528,	1,		14,		STR_FINANCIAL_GRAPH,	STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,			0,	517,	527,	2,		13,		STR_CLOSE_X,			STR_CLOSE_WINDOW_TIP },
	{ WWT_RESIZE,			1,	0,		529,	43,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_SUMMARY_TAB_TIP },
	{ WWT_TAB,				1,	34,		64,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_CASH_TAB_TIP },
	{ WWT_TAB,				1,	65,		95,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_PARK_VALUE_TAB_TIP },
	{ WWT_TAB,				1,	96,		126,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_WEEKLY_PROFIT_TAB_TIP },
	{ WWT_TAB,				1,	127,	157,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_MARKETING_TAB_TIP },
	{ WWT_TAB,				1,	158,	188,	17,		43,		0x2000144E,				STR_FINANCES_RESEARCH_TIP },
	{ WIDGETS_END },
};

static rct_widget window_finances_park_value_widgets[] = {
	{ WWT_FRAME,			0,	0,		529,	0,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_CAPTION,			0,	1,		528,	1,		14,		STR_PARK_VALUE_GRAPH,	STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,			0,	517,	527,	2,		13,		STR_CLOSE_X,			STR_CLOSE_WINDOW_TIP },
	{ WWT_RESIZE,			1,	0,		529,	43,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_SUMMARY_TAB_TIP },
	{ WWT_TAB,				1,	34,		64,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_CASH_TAB_TIP },
	{ WWT_TAB,				1,	65,		95,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_PARK_VALUE_TAB_TIP },
	{ WWT_TAB,				1,	96,		126,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_WEEKLY_PROFIT_TAB_TIP },
	{ WWT_TAB,				1,	127,	157,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_MARKETING_TAB_TIP },
	{ WWT_TAB,				1,	158,	188,	17,		43,		0x2000144E,				STR_FINANCES_RESEARCH_TIP },
	{ WIDGETS_END },
};

static rct_widget window_finances_profit_widgets[] = {
	{ WWT_FRAME,			0,	0,		529,	0,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_CAPTION,			0,	1,		528,	1,		14,		STR_PROFIT_GRAPH,		STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,			0,	517,	527,	2,		13,		STR_CLOSE_X,			STR_CLOSE_WINDOW_TIP },
	{ WWT_RESIZE,			1,	0,		529,	43,		256,	0xFFFFFFFF,				STR_NONE },
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_SUMMARY_TAB_TIP },
	{ WWT_TAB,				1,	34,		64,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_CASH_TAB_TIP },
	{ WWT_TAB,				1,	65,		95,		17,		43,		0x2000144E,				STR_FINANCES_SHOW_PARK_VALUE_TAB_TIP },
	{ WWT_TAB,				1,	96,		126,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_WEEKLY_PROFIT_TAB_TIP },
	{ WWT_TAB,				1,	127,	157,	17,		43,		0x2000144E,				STR_FINANCES_SHOW_MARKETING_TAB_TIP },
	{ WWT_TAB,				1,	158,	188,	17,		43,		0x2000144E,				STR_FINANCES_RESEARCH_TIP },
	{ WIDGETS_END },
};

static rct_widget window_finances_marketing_widgets[] = {
	{ WWT_FRAME,			0,	0,		529,	0,		256,	0xFFFFFFFF,								STR_NONE },
	{ WWT_CAPTION,			0,	1,		528,	1,		14,		STR_MARKETING,							STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,			0,	517,	527,	2,		13,		STR_CLOSE_X,							STR_CLOSE_WINDOW_TIP },
	{ WWT_RESIZE,			1,	0,		529,	43,		256,	0xFFFFFFFF,								STR_NONE },
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x2000144E,								STR_FINANCES_SHOW_SUMMARY_TAB_TIP },
	{ WWT_TAB,				1,	34,		64,		17,		43,		0x2000144E,								STR_FINANCES_SHOW_CASH_TAB_TIP },
	{ WWT_TAB,				1,	65,		95,		17,		43,		0x2000144E,								STR_FINANCES_SHOW_PARK_VALUE_TAB_TIP },
	{ WWT_TAB,				1,	96,		126,	17,		43,		0x2000144E,								STR_FINANCES_SHOW_WEEKLY_PROFIT_TAB_TIP },
	{ WWT_TAB,				1,	127,	157,	17,		43,		0x2000144E,								STR_FINANCES_SHOW_MARKETING_TAB_TIP },
	{ WWT_TAB,				1,	158,	188,	17,		43,		0x2000144E,								STR_FINANCES_RESEARCH_TIP },
	{ WWT_GROUPBOX,			2,	3,		526,	47,		91,		STR_MARKETING_CAMPAIGNS_IN_OPERATION,	STR_NONE },
	{ WWT_GROUPBOX,			2,	3,		526,	47,		252,	STR_MARKETING_CAMPAIGNS_AVAILABLE,		STR_NONE },
	{ WWT_IMGBTN,			1,	8,		521,	0,		11,		0xFFFFFFFF,								STR_START_THIS_MARKETING_CAMPAIGN },
	{ WWT_IMGBTN,			1,	8,		521,	0,		11,		0xFFFFFFFF,								STR_START_THIS_MARKETING_CAMPAIGN },
	{ WWT_IMGBTN,			1,	8,		521,	0,		11,		0xFFFFFFFF,								STR_START_THIS_MARKETING_CAMPAIGN },
	{ WWT_IMGBTN,			1,	8,		521,	0,		11,		0xFFFFFFFF,								STR_START_THIS_MARKETING_CAMPAIGN },
	{ WWT_IMGBTN,			1,	8,		521,	0,		11,		0xFFFFFFFF,								STR_START_THIS_MARKETING_CAMPAIGN },
	{ WWT_IMGBTN,			1,	8,		521,	0,		11,		0xFFFFFFFF,								STR_START_THIS_MARKETING_CAMPAIGN },
	{ WIDGETS_END },
};

static rct_widget window_finances_research_widgets[] = {
	{ WWT_FRAME,			0,	0,		319,	0,		206,	0xFFFFFFFF,								STR_NONE },
	{ WWT_CAPTION,			0,	1,		318,	1,		14,		STR_RESEARCH_FUNDING,					STR_WINDOW_TITLE_TIP },
	{ WWT_CLOSEBOX,			0,	307,	317,	2,		13,		STR_CLOSE_X,							STR_CLOSE_WINDOW_TIP },
	{ WWT_RESIZE,			1,	0,		319,	43,		206,	0xFFFFFFFF,								STR_NONE },
	{ WWT_TAB,				1,	3,		33,		17,		43,		0x2000144E,								STR_FINANCES_SHOW_SUMMARY_TAB_TIP },
	{ WWT_TAB,				1,	34,		64,		17,		43,		0x2000144E,								STR_FINANCES_SHOW_CASH_TAB_TIP },
	{ WWT_TAB,				1,	65,		95,		17,		43,		0x2000144E,								STR_FINANCES_SHOW_PARK_VALUE_TAB_TIP },
	{ WWT_TAB,				1,	96,		126,	17,		43,		0x2000144E,								STR_FINANCES_SHOW_WEEKLY_PROFIT_TAB_TIP },
	{ WWT_TAB,				1,	127,	157,	17,		43,		0x2000144E,								STR_FINANCES_SHOW_MARKETING_TAB_TIP },
	{ WWT_TAB,				1,	158,	188,	17,		43,		0x2000144E,								STR_FINANCES_RESEARCH_TIP },
	{ WWT_GROUPBOX,			2,	3,		316,	47,		91,		STR_RESEARCH_FUNDING_,					STR_NONE },
	{ WWT_DROPDOWN,			2,	8,		167,	59,		70,		0xFFFFFFFF,								STR_SELECT_LEVEL_OF_RESEARCH_AND_DEVELOPMENT },
	{ WWT_DROPDOWN_BUTTON,	2,	156,	166,	60,		69,		876,									STR_SELECT_LEVEL_OF_RESEARCH_AND_DEVELOPMENT },
	{ WWT_GROUPBOX,			2,	3,		316,	96,		202,	STR_RESEARCH_PRIORITIES,				STR_NONE },
	{ WWT_CHECKBOX,			2,	8,		311,	108,	119,	STR_RESEARCH_TRANSPORT_RIDES,			STR_RESEARCH_NEW_TRANSPORT_RIDES },
	{ WWT_CHECKBOX,			2,	8,		311,	121,	132,	STR_RESEARCH_GENTLE_RIDES,				STR_RESEARCH_NEW_GENTLE_RIDES },
	{ WWT_CHECKBOX,			2,	8,		311,	134,	145,	STR_RESEARCH_ROLLER_COASTERS,			STR_RESEARCH_NEW_ROLLER_COASTERS },
	{ WWT_CHECKBOX,			2,	8,		311,	147,	158,	STR_RESEARCH_THRILL_RIDES,				STR_RESEARCH_NEW_THRILL_RIDES },
	{ WWT_CHECKBOX,			2,	8,		311,	160,	171,	STR_RESEARCH_WATER_RIDES,				STR_RESEARCH_NEW_WATER_RIDES },
	{ WWT_CHECKBOX,			2,	8,		311,	173,	184,	STR_RESEARCH_SHOPS_AND_STALLS,			STR_RESEARCH_NEW_SHOPS_AND_STALLS },
	{ WWT_CHECKBOX,			2,	8,		311,	186,	197,	STR_RESEARCH_SCENERY_AND_THEMING,		STR_RESEARCH_NEW_SCENERY_AND_THEMING },
	{ WIDGETS_END },
};

static rct_widget *window_finances_page_widgets[] = {
	window_finances_summary_widgets,
	window_finances_cash_widgets,
	window_finances_park_value_widgets,
	window_finances_profit_widgets,
	window_finances_marketing_widgets,
	window_finances_research_widgets
};

#pragma endregion

#pragma region Events

static void window_finances_summary_mouseup(rct_window *w, int widgetIndex);
static void window_finances_summary_mousedown(int widgetIndex, rct_window*w, rct_widget* widget);
static void window_finances_summary_update(rct_window *w);
static void window_finances_summary_invalidate(rct_window *w);
static void window_finances_summary_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_finances_financial_graph_mouseup(rct_window *w, int widgetIndex);
static void window_finances_financial_graph_update(rct_window *w);
static void window_finances_financial_graph_invalidate(rct_window *w);
static void window_finances_financial_graph_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_finances_park_value_graph_mouseup(rct_window *w, int widgetIndex);
static void window_finances_park_value_graph_update(rct_window *w);
static void window_finances_park_value_graph_invalidate(rct_window *w);
static void window_finances_park_value_graph_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_finances_profit_graph_mouseup(rct_window *w, int widgetIndex);
static void window_finances_profit_graph_update(rct_window *w);
static void window_finances_profit_graph_invalidate(rct_window *w);
static void window_finances_profit_graph_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_finances_marketing_mouseup(rct_window *w, int widgetIndex);
static void window_finances_marketing_update(rct_window *w);
static void window_finances_marketing_invalidate(rct_window *w);
static void window_finances_marketing_paint(rct_window *w, rct_drawpixelinfo *dpi);

static void window_finances_research_mouseup(rct_window *w, int widgetIndex);
static void window_finances_research_mousedown(int widgetIndex, rct_window*w, rct_widget* widget);
static void window_finances_research_dropdown(rct_window *w, int widgetIndex, int dropdownIndex);
static void window_finances_research_update(rct_window *w);
static void window_finances_research_invalidate(rct_window *w);
static void window_finances_research_paint(rct_window *w, rct_drawpixelinfo *dpi);

// 0x00988EB8
static rct_window_event_list window_finances_summary_events = {
	NULL,
	window_finances_summary_mouseup,
	NULL,
	window_finances_summary_mousedown,
	NULL,
	NULL,
	window_finances_summary_update,
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
	window_finances_summary_invalidate,
	window_finances_summary_paint,
	NULL
};

// 0x00988F28
static rct_window_event_list window_finances_financial_graph_events = {
	NULL,
	window_finances_financial_graph_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	window_finances_financial_graph_update,
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
	window_finances_financial_graph_invalidate,
	window_finances_financial_graph_paint,
	NULL
};

// 0x00988F98
static rct_window_event_list window_finances_value_graph_events = {
	NULL,
	window_finances_park_value_graph_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	window_finances_park_value_graph_update,
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
	window_finances_park_value_graph_invalidate,
	window_finances_park_value_graph_paint,
	NULL
};

// 0x00989008
static rct_window_event_list window_finances_profit_graph_events = {
	NULL,
	window_finances_profit_graph_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	window_finances_profit_graph_update,
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
	window_finances_profit_graph_invalidate,
	window_finances_profit_graph_paint,
	NULL
};

// 0x00989078
static rct_window_event_list window_finances_marketing_events = {
	NULL,
	window_finances_marketing_mouseup,
	NULL,
	NULL,
	NULL,
	NULL,
	window_finances_marketing_update,
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
	window_finances_marketing_invalidate,
	window_finances_marketing_paint,
	NULL
};

// 0x009890E8
static rct_window_event_list window_finances_research_events = {
	NULL,
	window_finances_research_mouseup,
	NULL,
	window_finances_research_mousedown,
	window_finances_research_dropdown,
	NULL,
	window_finances_research_update,
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
	window_finances_research_invalidate,
	window_finances_research_paint,
	NULL
};

static rct_window_event_list *window_finances_page_events[] = {
	&window_finances_summary_events,
	&window_finances_financial_graph_events,
	&window_finances_value_graph_events,
	&window_finances_profit_graph_events,
	&window_finances_marketing_events,
	&window_finances_research_events
};

static void window_finances_set_colours();

#pragma endregion

#pragma region Enabled widgets

static uint32 window_finances_page_enabled_widgets[] = {
	(1 << WIDX_CLOSE) |
	(1 << WIDX_TAB_1) |
	(1 << WIDX_TAB_2) |
	(1 << WIDX_TAB_3) |
	(1 << WIDX_TAB_4) |
	(1 << WIDX_TAB_5) |
	(1 << WIDX_TAB_6) |
	(1 << WIDX_LOAN_INCREASE) |
	(1 << WIDX_LOAN_DECREASE),

	(1 << WIDX_CLOSE) |
	(1 << WIDX_TAB_1) |
	(1 << WIDX_TAB_2) |
	(1 << WIDX_TAB_3) |
	(1 << WIDX_TAB_4) |
	(1 << WIDX_TAB_5) |
	(1 << WIDX_TAB_6),

	(1 << WIDX_CLOSE) |
	(1 << WIDX_TAB_1) |
	(1 << WIDX_TAB_2) |
	(1 << WIDX_TAB_3) |
	(1 << WIDX_TAB_4) |
	(1 << WIDX_TAB_5) |
	(1 << WIDX_TAB_6),

	(1 << WIDX_CLOSE) |
	(1 << WIDX_TAB_1) |
	(1 << WIDX_TAB_2) |
	(1 << WIDX_TAB_3) |
	(1 << WIDX_TAB_4) |
	(1 << WIDX_TAB_5) |
	(1 << WIDX_TAB_6),

	(1 << WIDX_CLOSE) |
	(1 << WIDX_TAB_1) |
	(1 << WIDX_TAB_2) |
	(1 << WIDX_TAB_3) |
	(1 << WIDX_TAB_4) |
	(1 << WIDX_TAB_5) |
	(1 << WIDX_TAB_6) |
	(1 << WIDX_CAMPAIGN_1) |
	(1 << WIDX_CAMPAIGN_2) |
	(1 << WIDX_CAMPAIGN_3) |
	(1 << WIDX_CAMPAIGN_4) |
	(1 << WIDX_CAMPAIGN_5) |
	(1 << WIDX_CAMPAIGN_6),

	(1 << WIDX_CLOSE) |
	(1 << WIDX_TAB_1) |
	(1 << WIDX_TAB_2) |
	(1 << WIDX_TAB_3) |
	(1 << WIDX_TAB_4) |
	(1 << WIDX_TAB_5) |
	(1 << WIDX_TAB_6) |

	(1 << WIDX_RESEARCH_FUNDING) |
	(1 << WIDX_RESEARCH_FUNDING_DROPDOWN_BUTTON) |
	(1 << WIDX_TRANSPORT_RIDES) |
	(1 << WIDX_GENTLE_RIDES) |
	(1 << WIDX_ROLLER_COASTERS) |
	(1 << WIDX_THRILL_RIDES) |
	(1 << WIDX_WATER_RIDES) |
	(1 << WIDX_SHOPS_AND_STALLS) |
	(1 << WIDX_SCENERY_AND_THEMING)
};

static uint32 window_finances_page_hold_down_widgets[] = {
	(1 << WIDX_LOAN_INCREASE) |
	(1 << WIDX_LOAN_DECREASE),

	0,
	0,
	0,
	0,
	0
};

#pragma endregion

const int window_finances_tab_animation_loops[] = { 16, 32, 32, 32, 38, 16 };

static void window_finances_set_page(rct_window *w, int page);
static void window_finances_set_pressed_tab(rct_window *w);
static void window_finances_draw_tab_images(rct_drawpixelinfo *dpi, rct_window *w);

/**
 *
 *  rct2: 0x0069DDF1
 */
void window_finances_open()
{
	rct_window *w;

	w = window_bring_to_front_by_class(WC_FINANCES);
	if (w == NULL) {
		w = window_create_auto_pos(530, 257, window_finances_page_events[0], WC_FINANCES, WF_10);
		w->number = 0;
		w->frame_no = 0;

		research_update_uncompleted_types();
	}

	w->page = WINDOW_FINANCES_PAGE_SUMMARY;
	window_invalidate(w);
	w->width = 530;
	w->height = 257;
	window_invalidate(w);

	w->widgets = window_finances_page_widgets[WINDOW_FINANCES_PAGE_SUMMARY];
	w->enabled_widgets = window_finances_page_enabled_widgets[WINDOW_FINANCES_PAGE_SUMMARY];
	w->hold_down_widgets = window_finances_page_hold_down_widgets[WINDOW_FINANCES_PAGE_SUMMARY];
	w->event_handlers = window_finances_page_events[WINDOW_FINANCES_PAGE_SUMMARY];
	w->pressed_widgets = 0;
	w->disabled_widgets = 0;
	window_init_scroll_widgets(w);
}

/**
 *
 *  rct2: 0x0069DDE1
 */
void window_finances_research_open()
{
	rct_window *w;

	window_finances_open();
	w = window_find_by_class(WC_FINANCES);
	if (w != NULL)
		window_finances_set_page(w, WINDOW_FINANCES_PAGE_RESEARCH);
}

#pragma region Summary page

/**
 *
 *  rct2: 0x0069CA99
 */
static void window_finances_summary_mouseup(rct_window *w, int widgetIndex)
{
	if (widgetIndex == WIDX_CLOSE)
		window_close(w);
	else if (widgetIndex >= WIDX_TAB_1 && widgetIndex <= WIDX_TAB_6)
		window_finances_set_page(w, widgetIndex - WIDX_TAB_1);
}

/**
 *
 *  rct2: 0x0069CAB0
 */
static void window_finances_summary_mousedown(int widgetIndex, rct_window*w, rct_widget* widget)
{
	money32 newLoan;

	switch (widgetIndex) {
	case WIDX_LOAN_INCREASE:
		newLoan = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32) + MONEY(1000, 00);
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_CANT_BORROW_ANY_MORE_MONEY;
		finance_set_loan(newLoan);
		break;
	case WIDX_LOAN_DECREASE:
		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32) > 0) {
			newLoan = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32) - MONEY(1000, 00);
			RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_CANT_PAY_BACK_LOAN;
			finance_set_loan(newLoan);
		}
		break;
	}
}

/**
 *
 *  rct2: 0x0069CBA6
 */
static void window_finances_summary_update(rct_window *w)
{
	// Tab animation
	if (++w->frame_no >= window_finances_tab_animation_loops[w->page])
		w->frame_no = 0;
	widget_invalidate(w, WIDX_TAB_1);
}

/**
 *
 *  rct2: 0x0069C732
 */
static void window_finances_summary_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	if (w->widgets != window_finances_page_widgets[WINDOW_FINANCES_PAGE_SUMMARY]) {
		w->widgets = window_finances_page_widgets[WINDOW_FINANCES_PAGE_SUMMARY];
		window_init_scroll_widgets(w);
	}

	window_finances_set_pressed_tab(w);
	RCT2_GLOBAL(0x013CE952 + 6, money32) = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32);
}

/**
 *
 *  rct2: 0x0069C771
 */
static void window_finances_summary_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	int i, j, x, y;

	window_draw_widgets(w, dpi);
	window_finances_draw_tab_images(dpi, w);

	x = w->x + 8;
	y = w->y + 47;

	// Expenditure / Income heading
	draw_string_left_underline(dpi, STR_FINANCES_SUMMARY_EXPENDITURE_INCOME, NULL, 0, x, y - 1);
	y += 14;

	// Expenditure / Income row labels
	for (i = 0; i < 14; i++) {
		// Darken every even row
		if (i % 2 == 0)
			gfx_fill_rect(dpi, x, y, x + 513 - 2, y + 9, ColourMapA[w->colours[1]].lighter | 0x1000000);

		gfx_draw_string_left(dpi, STR_FINANCES_SUMMARY_RIDE_CONSTRUCTION + i, NULL, 0, x, y - 1);
		y += 10;
	}

	// Expenditure / Income values for each month
	x = w->x + 118;
	sint16 currentMonthYear = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, sint16);
	for (i = 4; i >= 0; i--) {
		y = w->y + 47;

		sint16 monthyear = currentMonthYear - i;
		if (monthyear < 0)
			continue;

		// Month heading
		RCT2_GLOBAL(0x0013CE952, uint16) = STR_FINANCES_SUMMARY_MONTH_HEADING;
		RCT2_GLOBAL(0x0013CE952 + 2, uint16) = monthyear;
		draw_string_right_underline(
			dpi,
			monthyear == currentMonthYear ? 1193 : 1191,
			(void*)0x0013CE952,
			0,
			x + 80,
			y - 1
		);
		y += 14;

		// Month expenditures
		money32 profit = 0;
		money32 *expenditures = &RCT2_ADDRESS(RCT2_ADDRESS_EXPENDITURE_TABLE, money32)[i * 14];
		for (j = 0; j < 14; j++) {
			money32 expenditure = expenditures[j];
			if (expenditure != 0) {
				profit += expenditure;
				gfx_draw_string_right(
					dpi,
					expenditure >= 0 ? STR_FINANCES_SUMMARY_INCOME_VALUE : STR_FINANCES_SUMMARY_EXPENDITURE_VALUE,
					&expenditure,
					0,
					x + 80,
					y - 1
				);
			}
			y += 10;
		}
		y += 4;

		// Month profit
		gfx_draw_string_right(
			dpi,
			profit >= 0 ? STR_FINANCES_SUMMARY_INCOME_VALUE : STR_FINANCES_SUMMARY_LOSS_VALUE,
			&profit,
			0,
			x + 80,
			y - 1
		);
		gfx_fill_rect(dpi, x + 10, y - 2, x + 10 + 70, y - 2, 10);

		x += 80;
	}


	// Horizontal rule below expenditure / income table
	gfx_fill_rect_inset(dpi, w->x + 8, w->y + 223, w->x + 8 + 513, w->y + 223 + 1, w->colours[1], 0x20);

	// Loan and interest rate
	gfx_draw_string_left(dpi, STR_FINANCES_SUMMARY_LOAN, NULL, 0, w->x + 4, w->y + 229);
	RCT2_GLOBAL(0x0013CE952, uint16) = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_INTEREST_RATE, uint8);
	gfx_draw_string_left(dpi, STR_FINANCES_SUMMARY_AT_X_PER_YEAR, (void*)0x0013CE952, 0, w->x + 156, w->y + 229);

	// Current cash
	RCT2_GLOBAL(0x0013CE952, money32) = DECRYPT_MONEY(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, money32));
	gfx_draw_string_left(dpi, STR_CASH_LABEL, (void*)0x0013CE952, 0, w->x + 4, w->y + 244);

	// Objective related financial information
	if (RCT2_GLOBAL(RCT2_ADDRESS_OBJECTIVE_TYPE, uint8) == OBJECTIVE_MONTHLY_FOOD_INCOME) {
		// Last month's profit from food, drink and merchandise
		money32 lastMonthProfit = 0;
		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, uint16) != 0) {
			lastMonthProfit += RCT2_GLOBAL(0x01357898, money32);
			lastMonthProfit += RCT2_GLOBAL(0x0135789C, money32);
			lastMonthProfit += RCT2_GLOBAL(0x013578A0, money32);
			lastMonthProfit += RCT2_GLOBAL(0x013578A4, money32);
		}
		RCT2_GLOBAL(0x0013CE952, money32) = lastMonthProfit;
		gfx_draw_string_left(dpi, STR_LAST_MONTH_PROFIT_FROM_FOOD_DRINK_MERCHANDISE_SALES_LABEL, (void*)0x0013CE952, 0, w->x + 280, w->y + 229);
	} else {
		// Park value and company value
		gfx_draw_string_left(dpi, STR_PARK_VALUE_LABEL, (void*)RCT2_ADDRESS_CURRENT_PARK_VALUE, 0, w->x + 280, w->y + 229);
		gfx_draw_string_left(dpi, STR_COMPANY_VALUE_LABEL, (void*)RCT2_ADDRESS_CURRENT_COMPANY_VALUE, 0, w->x + 280, w->y + 244);
	}
}

#pragma endregion

#pragma region Financial graph page

/**
 *
 *  rct2: 0x0069CF70
 */
static void window_finances_financial_graph_mouseup(rct_window *w, int widgetIndex)
{
	if (widgetIndex == WIDX_CLOSE)
		window_close(w);
	else if (widgetIndex >= WIDX_TAB_1 && widgetIndex <= WIDX_TAB_6)
		window_finances_set_page(w, widgetIndex - WIDX_TAB_1);
}

/**
 *
 *  rct2: 0x0069CF8B
 */
static void window_finances_financial_graph_update(rct_window *w)
{
	// Tab animation
	if (++w->frame_no >= window_finances_tab_animation_loops[w->page])
		w->frame_no = 0;
	widget_invalidate(w, WIDX_TAB_2);
}

/**
 *
 *  rct2: 0x0069CBDB
 */
static void window_finances_financial_graph_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	if (w->widgets != window_finances_page_widgets[WINDOW_FINANCES_PAGE_FINANCIAL_GRAPH]) {
		w->widgets = window_finances_page_widgets[WINDOW_FINANCES_PAGE_FINANCIAL_GRAPH];
		window_init_scroll_widgets(w);
	}

	window_finances_set_pressed_tab(w);
}

/**
 *
 *  rct2: 0x0069CC10
 */
static void window_finances_financial_graph_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	int i, x, y, graphLeft, graphTop, graphRight, graphBottom;

	window_draw_widgets(w, dpi);
	window_finances_draw_tab_images(dpi, w);

	rct_widget *pageWidget = &window_finances_cash_widgets[WIDX_PAGE_BACKGROUND];
	graphLeft = w->x + pageWidget->left + 4;
	graphTop = w->y + pageWidget->top + 15;
	graphRight = w->x + pageWidget->right - 4;
	graphBottom = w->y + pageWidget->bottom - 4;

	// Cash (less loan)
	money32 cashLessLoan =
		DECRYPT_MONEY(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, money32)) -
		RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32);

	gfx_draw_string_left(
		dpi,
		cashLessLoan >= 0 ?
			STR_FINANCES_FINANCIAL_GRAPH_CASH_LESS_LOAN_POSITIVE : STR_FINANCES_FINANCIAL_GRAPH_CASH_LESS_LOAN_NEGATIVE,
		&cashLessLoan,
		0,
		graphLeft,
		graphTop - 11
	);

	// Graph
	gfx_fill_rect_inset(dpi, graphLeft, graphTop, graphRight, graphBottom, w->colours[1], 0x30);

	// Calculate the Y axis scale (log2 of highest [+/-]balance)
	int yAxisScale = 0;
	for (i = 0; i < 64; i++) {
		money32 balance = gCashHistory[i];
		if (balance == MONEY32_UNDEFINED)
			continue;

		// Modifier balance then keep halfing until less than 127 pixels
		balance = abs(balance) >> yAxisScale;
		while (balance > 127) {
			balance /= 2;
			yAxisScale++;
		}
	}

	// Y axis labels
	x = graphLeft + 18;
	y = graphTop + 14;
	money32 axisBase;
	for (axisBase = MONEY(12,00); axisBase >= MONEY(-12,00); axisBase -= MONEY(6,00)) {
		money32 axisValue = axisBase << yAxisScale;
		gfx_draw_string_right(dpi, STR_FINANCES_FINANCIAL_GRAPH_CASH_VALUE, &axisValue, 0, x + 70, y);
		y += 39;
	}

	// X axis labels and values
	x = graphLeft + 98;
	y = graphTop + 17;
	graph_draw_money32(dpi, gCashHistory, 64, x, y, yAxisScale, 128);
}

#pragma endregion

#pragma region Value graph page

/**
 *
 *  rct2: 0x0069D338
 */
static void window_finances_park_value_graph_mouseup(rct_window *w, int widgetIndex)
{
	if (widgetIndex == WIDX_CLOSE)
		window_close(w);
	else if (widgetIndex >= WIDX_TAB_1 && widgetIndex <= WIDX_TAB_6)
		window_finances_set_page(w, widgetIndex - WIDX_TAB_1);
}

/**
 *
 *  rct2: 0x0069D353
 */
static void window_finances_park_value_graph_update(rct_window *w)
{
	// Tab animation
	if (++w->frame_no >= window_finances_tab_animation_loops[w->page])
		w->frame_no = 0;
	widget_invalidate(w, WIDX_TAB_2);
}

/**
 *
 *  rct2: 0x0069CFC0
 */
static void window_finances_park_value_graph_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	if (w->widgets != window_finances_page_widgets[WINDOW_FINANCES_PAGE_VALUE_GRAPH]) {
		w->widgets = window_finances_page_widgets[WINDOW_FINANCES_PAGE_VALUE_GRAPH];
		window_init_scroll_widgets(w);
	}

	window_finances_set_pressed_tab(w);
}

/**
 *
 *  rct2: 0x0069CFF5
 */
static void window_finances_park_value_graph_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	int i, x, y, graphLeft, graphTop, graphRight, graphBottom;

	window_draw_widgets(w, dpi);
	window_finances_draw_tab_images(dpi, w);

	rct_widget *pageWidget = &window_finances_cash_widgets[WIDX_PAGE_BACKGROUND];
	graphLeft = w->x + pageWidget->left + 4;
	graphTop = w->y + pageWidget->top + 15;
	graphRight = w->x + pageWidget->right - 4;
	graphBottom = w->y + pageWidget->bottom - 4;

	// Park value
	money32 parkValue = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PARK_VALUE, money32);
	gfx_draw_string_left(
		dpi,
		STR_FINACNES_PARK_VALUE,
		&parkValue,
		0,
		graphLeft,
		graphTop - 11
	);

	// Graph
	gfx_fill_rect_inset(dpi, graphLeft, graphTop, graphRight, graphBottom, w->colours[1], 0x30);

	// Calculate the Y axis scale (log2 of highest [+/-]balance)
	int yAxisScale = 0;
	for (i = 0; i < 64; i++) {
		money32 balance = gParkValueHistory[i];
		if (balance == MONEY32_UNDEFINED)
			continue;

		// Modifier balance then keep halfing until less than 255 pixels
		balance = abs(balance) >> yAxisScale;
		while (balance > 255) {
			balance /= 2;
			yAxisScale++;
		}
	}

	// Y axis labels
	x = graphLeft + 18;
	y = graphTop + 14;
	money32 axisBase;
	for (axisBase = MONEY(24,00); axisBase >= MONEY(0,00); axisBase -= MONEY(6,00)) {
		money32 axisValue = axisBase << yAxisScale;
		gfx_draw_string_right(dpi, STR_FINANCES_FINANCIAL_GRAPH_CASH_VALUE, &axisValue, 0, x + 70, y);
		y += 39;
	}

	// X axis labels and values
	x = graphLeft + 98;
	y = graphTop + 17;
	graph_draw_money32(dpi, gParkValueHistory, 64, x, y, yAxisScale, 0);
}

#pragma endregion

#pragma region Profit graph page

/**
 *
 *  rct2: 0x0069D715
 */
static void window_finances_profit_graph_mouseup(rct_window *w, int widgetIndex)
{
	if (widgetIndex == WIDX_CLOSE)
		window_close(w);
	else if (widgetIndex >= WIDX_TAB_1 && widgetIndex <= WIDX_TAB_6)
		window_finances_set_page(w, widgetIndex - WIDX_TAB_1);
}

/**
 *
 *  rct2: 0x0069D730
 */
static void window_finances_profit_graph_update(rct_window *w)
{
	// Tab animation
	if (++w->frame_no >= window_finances_tab_animation_loops[w->page])
		w->frame_no = 0;
	widget_invalidate(w, WIDX_TAB_2);
}

/**
 *
 *  rct2: 0x0069D388
 */
static void window_finances_profit_graph_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	if (w->widgets != window_finances_page_widgets[WINDOW_FINANCES_PAGE_PROFIT_GRAPH]) {
		w->widgets = window_finances_page_widgets[WINDOW_FINANCES_PAGE_PROFIT_GRAPH];
		window_init_scroll_widgets(w);
	}

	window_finances_set_pressed_tab(w);
}

/**
 *
 *  rct2: 0x0069D3BD
 */
static void window_finances_profit_graph_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	int i, x, y, graphLeft, graphTop, graphRight, graphBottom;

	window_draw_widgets(w, dpi);
	window_finances_draw_tab_images(dpi, w);

	rct_widget *pageWidget = &window_finances_cash_widgets[WIDX_PAGE_BACKGROUND];
	graphLeft = w->x + pageWidget->left + 4;
	graphTop = w->y + pageWidget->top + 15;
	graphRight = w->x + pageWidget->right - 4;
	graphBottom = w->y + pageWidget->bottom - 4;

	// Weekly profit
	money32 weeklyPofit = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PROFIT, money32);
	gfx_draw_string_left(
		dpi,
		weeklyPofit >= 0 ? STR_FINANCES_WEEKLY_PROFIT_POSITIVE : STR_FINANCES_WEEKLY_PROFIT_LOSS,
		&weeklyPofit,
		0,
		graphLeft,
		graphTop - 11
	);

	// Graph
	gfx_fill_rect_inset(dpi, graphLeft, graphTop, graphRight, graphBottom, w->colours[1], 0x30);

	// Calculate the Y axis scale (log2 of highest [+/-]balance)
	int yAxisScale = 0;
	for (i = 0; i < 64; i++) {
		money32 balance = gWeeklyProfitHistory[i];
		if (balance == MONEY32_UNDEFINED)
			continue;

		// Modifier balance then keep halfing until less than 127 pixels
		balance = abs(balance) >> yAxisScale;
		while (balance > 127) {
			balance /= 2;
			yAxisScale++;
		}
	}

	// Y axis labels
	x = graphLeft + 18;
	y = graphTop + 14;
	money32 axisBase;
	for (axisBase = MONEY(12,00); axisBase >= MONEY(-12,00); axisBase -= MONEY(6,00)) {
		money32 axisValue = axisBase << yAxisScale;
		gfx_draw_string_right(dpi, STR_FINANCES_FINANCIAL_GRAPH_CASH_VALUE, &axisValue, 0, x + 70, y);
		y += 39;
	}

	// X axis labels and values
	x = graphLeft + 98;
	y = graphTop + 17;
	graph_draw_money32(dpi, gWeeklyProfitHistory, 64, x, y, yAxisScale, 128);
}

#pragma endregion

#pragma region Marketing page

/**
 *
 *  rct2: 0x0069D9F9
 */
static void window_finances_marketing_mouseup(rct_window *w, int widgetIndex)
{
	if (widgetIndex == WIDX_CLOSE)
		window_close(w);
	else if (widgetIndex >= WIDX_TAB_1 && widgetIndex <= WIDX_TAB_6)
		window_finances_set_page(w, widgetIndex - WIDX_TAB_1);
	else if (widgetIndex >= WIDX_CAMPAIGN_1 && widgetIndex <= WIDX_CAMPAIGN_6)
		window_new_campaign_open(widgetIndex - WIDX_CAMPAIGN_1);

}

/**
 *
 *  rct2: 0x0069DA2F
 */
static void window_finances_marketing_update(rct_window *w)
{
	// Tab animation
	if (++w->frame_no >= window_finances_tab_animation_loops[w->page])
		w->frame_no = 0;
	widget_invalidate(w, WIDX_TAB_5);
}

/**
 *
 *  rct2: 0x0069D765
 */
static void window_finances_marketing_invalidate(rct_window *w)
{
	int i;

	colour_scheme_update(w);

	if (w->widgets != window_finances_page_widgets[WINDOW_FINANCES_PAGE_MARKETING]) {
		w->widgets = window_finances_page_widgets[WINDOW_FINANCES_PAGE_MARKETING];
		window_init_scroll_widgets(w);
	}

	window_finances_set_pressed_tab(w);

	// Count number of active campaigns
	int numActiveCampaigns = 0;
	for (i = 0; i < ADVERTISING_CAMPAIGN_COUNT; i++)
		if (gMarketingCampaignDaysLeft[i] != 0)
			numActiveCampaigns++;

	int y = max(1, numActiveCampaigns) * 10 + 92;

	// Update group box positions
	window_finances_marketing_widgets[WIDX_ACITVE_CAMPAGINS_GROUP].bottom = y - 20;
	window_finances_marketing_widgets[WIDX_CAMPAGINS_AVAILABLE_GROUP].top = y - 13;

	// Update new campagin button visibility
	for (i = 0; i < ADVERTISING_CAMPAIGN_COUNT; i++) {
		rct_widget *campaginButton = &window_finances_marketing_widgets[WIDX_CAMPAIGN_1 + i];

		campaginButton->type = 0;

		if (gMarketingCampaignDaysLeft[i] != 0)
			continue;

		if (!marketing_is_campaign_type_applicable(i))
			continue;

		campaginButton->type = WWT_DROPDOWN_BUTTON;
		campaginButton->top = y;
		campaginButton->bottom = y + 11;
		y += 12;
	}
}

/**
 *
 *  rct2: 0x0069D834
 */
static void window_finances_marketing_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	int i, x, y, weeksRemaining;
	rct_ride *ride;

	window_draw_widgets(w, dpi);
	window_finances_draw_tab_images(dpi, w);

	x = w->x + 8;
	y = w->y + 62;

	int noCampaignsActive = 1;
	for (i = 0; i < ADVERTISING_CAMPAIGN_COUNT; i++) {
		if (gMarketingCampaignDaysLeft[i] == 0)
			continue;

		noCampaignsActive = 0;
		RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint16) = RCT2_GLOBAL(RCT2_ADDRESS_PARK_NAME, rct_string_id);
		RCT2_GLOBAL(0x013CE952 + 2, uint32) = RCT2_GLOBAL(RCT2_ADDRESS_PARK_NAME_ARGS, uint32);

		// Set special parameters
		switch (i) {
		case ADVERTISING_CAMPAIGN_RIDE_FREE:
		case ADVERTISING_CAMPAIGN_RIDE:
			ride = GET_RIDE(gMarketingCampaignRideIndex[i]);
			RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint16) = ride->name;
			RCT2_GLOBAL(0x013CE952 + 2, uint32) = ride->name_arguments;
			break;
		case ADVERTISING_CAMPAIGN_FOOD_OR_DRINK_FREE:
			RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint16) = ShopItemStringIds[gMarketingCampaignRideIndex[i]].plural;
			break;
		}

		// Advertisement
		gfx_draw_string_left_clipped(dpi, STR_VOUCHERS_FOR_FREE_ENTRY_TO + i, (void*)0x013CE952, 0, x + 4, y, 296);

		// Duration
		weeksRemaining = (gMarketingCampaignDaysLeft[i] % 128);
		gfx_draw_string_left(dpi, weeksRemaining == 1 ? STR_1_WEEK_REMAINING : STR_X_WEEKS_REMAINING, &weeksRemaining, 0, x + 304, y);

		y += 10;
	}

	if (noCampaignsActive) {
		gfx_draw_string_left(dpi, STR_MARKETING_CAMPAGINS_NONE, NULL, 0, x + 4, y);
		y += 10;
	}
	y += 31;

	// Draw campaign button text
	for (i = 0; i < ADVERTISING_CAMPAIGN_COUNT; i++) {
		rct_widget *campaginButton = &window_finances_marketing_widgets[WIDX_CAMPAIGN_1 + i];

		if (campaginButton->type == WWT_EMPTY)
			continue;

		money32 pricePerWeek = AdvertisingCampaignPricePerWeek[i];

		// Draw button text
		gfx_draw_string_left(dpi, STR_MARKETING_VOUCHERS_FOR_FREE_ENTRY_TO_THE_PARK + i, NULL, 0, x + 4, y - 1);
		gfx_draw_string_left(dpi, STR_MARKETING_PER_WEEK, &pricePerWeek, 0, x + 310, y - 1);

		y += 12;
	}
}

#pragma endregion

#pragma region Research page

/**
 *
 *  rct2: 0x0069DB3F
 */
static void window_finances_research_mouseup(rct_window *w, int widgetIndex)
{
	int activeResearchTypes;

	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_TAB_1:
	case WIDX_TAB_2:
	case WIDX_TAB_3:
	case WIDX_TAB_4:
	case WIDX_TAB_5:
	case WIDX_TAB_6:
		window_finances_set_page(w, widgetIndex - WIDX_TAB_1);
		break;
	case WIDX_TRANSPORT_RIDES:
	case WIDX_GENTLE_RIDES:
	case WIDX_ROLLER_COASTERS:
	case WIDX_THRILL_RIDES:
	case WIDX_WATER_RIDES:
	case WIDX_SHOPS_AND_STALLS:
	case WIDX_SCENERY_AND_THEMING:
		activeResearchTypes = RCT2_GLOBAL(RCT2_ADDRESS_ACTIVE_RESEARCH_TYPES, uint16);
		activeResearchTypes ^= 1 << (widgetIndex - WIDX_TRANSPORT_RIDES);
		research_set_priority(activeResearchTypes);
		break;
	}
}

/**
 *
 *  rct2: 0x0069DB66
 */
static void window_finances_research_mousedown(int widgetIndex, rct_window *w, rct_widget* widget)
{
	rct_widget *dropdownWidget;
	int i;

	if (widgetIndex != WIDX_RESEARCH_FUNDING_DROPDOWN_BUTTON)
		return;

	dropdownWidget = widget - 1;

	for (i = 0; i < 4; i++) {
		gDropdownItemsFormat[i] = 1142;
		gDropdownItemsArgs[i] = STR_NO_FUNDING + i;
	}
	window_dropdown_show_text_custom_width(
		w->x + dropdownWidget->left,
		w->y + dropdownWidget->top,
		dropdownWidget->bottom - dropdownWidget->top + 1,
		w->colours[1],
		DROPDOWN_FLAG_STAY_OPEN,
		4,
		dropdownWidget->right - dropdownWidget->left - 3
	);

	int currentResearchLevel = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_RESEARCH_LEVEL, uint8);
	dropdown_set_checked(currentResearchLevel, true);
}

/**
 *
 *  rct2: 0x0069DB6D
 */
static void window_finances_research_dropdown(rct_window *w, int widgetIndex, int dropdownIndex)
{
	if (widgetIndex != WIDX_RESEARCH_FUNDING_DROPDOWN_BUTTON || dropdownIndex == -1)
		return;

	research_set_funding(dropdownIndex);
}

/**
 *
 *  rct2: 0x0069DC23
 */
static void window_finances_research_update(rct_window *w)
{
	// Tab animation
	if (++w->frame_no >= window_finances_tab_animation_loops[w->page])
		w->frame_no = 0;
	widget_invalidate(w, WIDX_TAB_6);
}

/**
 *
 *  rct2: 0x0069DA64
 */
static void window_finances_research_invalidate(rct_window *w)
{
	colour_scheme_update(w);

	if (w->widgets != window_finances_page_widgets[WINDOW_FINANCES_PAGE_RESEARCH]) {
		w->widgets = window_finances_page_widgets[WINDOW_FINANCES_PAGE_RESEARCH];
		window_init_scroll_widgets(w);
	}

	window_finances_set_pressed_tab(w);
	if (RCT2_GLOBAL(RCT2_ADDRESS_RESEARH_PROGRESS_STAGE, uint8) == RESEARCH_STAGE_FINISHED_ALL) {
		window_finances_research_widgets[WIDX_RESEARCH_FUNDING].type = WWT_EMPTY;
		window_finances_research_widgets[WIDX_RESEARCH_FUNDING_DROPDOWN_BUTTON].type = WWT_EMPTY;
	}
	int currentResearchLevel = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_RESEARCH_LEVEL, uint8);

	// Current funding
	window_finances_research_widgets[WIDX_RESEARCH_FUNDING].image = STR_NO_FUNDING + currentResearchLevel;

	// Checkboxes
	int activeResearchTypes = RCT2_GLOBAL(RCT2_ADDRESS_ACTIVE_RESEARCH_TYPES, uint16);
	int uncompletedResearchTypes = gResearchUncompletedCategories;
	for (int i = 0; i < 7; i++) {
		int mask = 1 << i;
		int widgetMask = 1 << (i + WIDX_TRANSPORT_RIDES);

		// Set checkbox disabled if research type is complete
		if (uncompletedResearchTypes & mask) {
			w->disabled_widgets &= ~widgetMask;

			// Set checkbox ticked if research type is active
			if (activeResearchTypes & mask)
				w->pressed_widgets |= widgetMask;
			else
				w->pressed_widgets &= ~widgetMask;
		} else {
			w->disabled_widgets |= widgetMask;
			w->pressed_widgets &= ~widgetMask;
		}
	}
}

/**
 *
 *  rct2: 0x0069DAF0
 */
static void window_finances_research_paint(rct_window *w, rct_drawpixelinfo *dpi)
{
	window_draw_widgets(w, dpi);
	window_finances_draw_tab_images(dpi, w);

	window_research_funding_page_paint(w, dpi, WIDX_RESEARCH_FUNDING);
}

#pragma endregion

#pragma region Common

/**
 *
 *  rct2: 0x0069CAC5
 */
static void window_finances_set_page(rct_window *w, int page)
{
	w->page = page;
	w->frame_no = 0;
	if (w->viewport != NULL) {
		w->viewport->width = 0;
		w->viewport = NULL;
	}

	w->enabled_widgets = window_finances_page_enabled_widgets[page];
	w->hold_down_widgets = window_finances_page_hold_down_widgets[page];
	w->event_handlers = window_finances_page_events[page];
	w->widgets = window_finances_page_widgets[page];
	w->disabled_widgets = 0;
	w->pressed_widgets = 0;

	window_invalidate(w);
	if (w->page == WINDOW_FINANCES_PAGE_RESEARCH) {
		w->width = 320;
		w->height = 207;
	} else {
		w->width = 530;
		w->height = 257;
	}
	window_event_resize_call(w);
	window_event_invalidate_call(w);

	window_init_scroll_widgets(w);
	window_invalidate(w);
}

static void window_finances_set_pressed_tab(rct_window *w)
{
	int i;
	for (i = 0; i < WINDOW_FINANCES_PAGE_COUNT; i++)
		w->pressed_widgets &= ~(1 << (WIDX_TAB_1 + i));
	w->pressed_widgets |= 1LL << (WIDX_TAB_1 + w->page);
}

static void window_finances_draw_tab_image(rct_drawpixelinfo *dpi, rct_window *w, int page, int spriteIndex)
{
	int widgetIndex = WIDX_TAB_1 + page;

	if (!(w->disabled_widgets & (1LL << widgetIndex))) {
		if (w->page == page) {
			int frame = w->frame_no / 2;
			if (page == WINDOW_FINANCES_PAGE_SUMMARY)
				frame %= 8;
			spriteIndex += frame;
		}

		gfx_draw_sprite(dpi, spriteIndex, w->x + w->widgets[widgetIndex].left, w->y + w->widgets[widgetIndex].top, 0);
	}
}

static void window_finances_draw_tab_images(rct_drawpixelinfo *dpi, rct_window *w)
{
	window_finances_draw_tab_image(dpi, w, WINDOW_FINANCES_PAGE_SUMMARY, SPR_TAB_FINANCES_SUMMARY_0);
	window_finances_draw_tab_image(dpi, w, WINDOW_FINANCES_PAGE_FINANCIAL_GRAPH, SPR_TAB_FINANCES_FINANCIAL_GRAPH_0);
	window_finances_draw_tab_image(dpi, w, WINDOW_FINANCES_PAGE_VALUE_GRAPH, SPR_TAB_FINANCES_VALUE_GRAPH_0);
	window_finances_draw_tab_image(dpi, w, WINDOW_FINANCES_PAGE_PROFIT_GRAPH, SPR_TAB_FINANCES_PROFIT_GRAPH_0);
	window_finances_draw_tab_image(dpi, w, WINDOW_FINANCES_PAGE_MARKETING, SPR_TAB_FINANCES_MARKETING_0);
	window_finances_draw_tab_image(dpi, w, WINDOW_FINANCES_PAGE_RESEARCH, SPR_TAB_FINANCES_RESEARCH_0);
}

#pragma endregion
