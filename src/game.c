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

#include "addresses.h"
#include "audio/audio.h"
#include "config.h"
#include "game.h"
#include "editor.h"
#include "world/footpath.h"
#include "input.h"
#include "localisation/localisation.h"
#include "interface/screenshot.h"
#include "interface/viewport.h"
#include "interface/widget.h"
#include "interface/window.h"
#include "management/finance.h"
#include "management/marketing.h"
#include "management/news_item.h"
#include "management/research.h"
#include "network/network.h"
#include "object.h"
#include "openrct2.h"
#include "peep/peep.h"
#include "peep/staff.h"
#include "platform/platform.h"
#include "ride/ride.h"
#include "ride/ride_ratings.h"
#include "ride/vehicle.h"
#include "ride/track.h"
#include "scenario.h"
#include "title.h"
#include "tutorial.h"
#include "util/sawyercoding.h"
#include "util/util.h"
#include "windows/error.h"
#include "windows/tooltip.h"
#include "world/climate.h"
#include "world/map_animation.h"
#include "world/park.h"
#include "world/scenery.h"
#include "world/sprite.h"
#include "world/water.h"

int gGameSpeed = 1;
float gDayNightCycle = 0;
bool gInUpdateCode = false;

GAME_COMMAND_CALLBACK_POINTER* game_command_callback = 0;
GAME_COMMAND_CALLBACK_POINTER* game_command_callback_table[] = {
	0,
	game_command_callback_ride_construct_new,
	game_command_callback_ride_construct_placed_front,
	game_command_callback_ride_construct_placed_back,
	game_command_callback_ride_remove_track_piece,
};

int game_command_callback_get_index(GAME_COMMAND_CALLBACK_POINTER* callback)
{
	for (int i = 0; i < countof(game_command_callback_table); i++ ) {
		if (game_command_callback_table[i] == callback) {
			return i;
		}
	}
	return 0;
}

GAME_COMMAND_CALLBACK_POINTER* game_command_callback_get_callback(int index)
{
	if (index < countof(game_command_callback_table)) {
		return game_command_callback_table[index];
	}
	return 0;
}

void game_increase_game_speed()
{
	gGameSpeed = min(gConfigGeneral.debugging_tools ? 5 : 4, gGameSpeed + 1);
	if (gGameSpeed == 5)
		gGameSpeed = 8;
	window_invalidate_by_class(WC_TOP_TOOLBAR);
}

void game_reduce_game_speed()
{
	gGameSpeed = max(1, gGameSpeed - 1);
	if (gGameSpeed == 7)
		gGameSpeed = 4;
	window_invalidate_by_class(WC_TOP_TOOLBAR);
}

/**
 *
 *  rct2: 0x0066B5C0 (part of 0x0066B3E8)
 */
void game_create_windows()
{
	window_main_open();
	window_top_toolbar_open();
	window_game_bottom_toolbar_open();
	window_resize_gui(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16), RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16));
}

/**
*
*  rct2: 0x006838BD
*/
void update_palette_effects()
{
	rct_water_type* water_type = (rct_water_type*)object_entry_groups[OBJECT_TYPE_WATER].chunks[0];

	if (RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) == 1) {
		// change palette to lighter color during lightning
		int palette = 1532;

		if ((sint32)water_type != -1) {
			palette = water_type->image_id;
		}
		rct_g1_element g1_element = g1Elements[palette];
		int xoffset = g1_element.x_offset;
		xoffset = xoffset * 4;
		for (int i = 0; i < g1_element.width; i++) {
			RCT2_ADDRESS(RCT2_ADDRESS_PALETTE + xoffset, uint8)[(i * 4) + 0] = -((0xFF - g1_element.offset[(i * 3) + 0]) / 2) - 1;
			RCT2_ADDRESS(RCT2_ADDRESS_PALETTE + xoffset, uint8)[(i * 4) + 1] = -((0xFF - g1_element.offset[(i * 3) + 1]) / 2) - 1;
			RCT2_ADDRESS(RCT2_ADDRESS_PALETTE + xoffset, uint8)[(i * 4) + 2] = -((0xFF - g1_element.offset[(i * 3) + 2]) / 2) - 1;
		}
		RCT2_GLOBAL(0x014241BC, uint32) = 2;
		platform_update_palette(RCT2_ADDRESS(RCT2_ADDRESS_PALETTE, uint8), 10, 236);
		RCT2_GLOBAL(0x014241BC, uint32) = 0;
		RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8)++;
	} else {
		if (RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) == 2) {
			// change palette back to normal after lightning
			int palette = 1532;

			if ((sint32)water_type != -1) {
				palette = water_type->image_id;
			}

			rct_g1_element g1_element = g1Elements[palette];
			int xoffset = g1_element.x_offset;
			xoffset = xoffset * 4;
			for (int i = 0; i < g1_element.width; i++) {
				RCT2_ADDRESS(RCT2_ADDRESS_PALETTE + xoffset, uint8)[(i * 4) + 0] = g1_element.offset[(i * 3) + 0];
				RCT2_ADDRESS(RCT2_ADDRESS_PALETTE + xoffset, uint8)[(i * 4) + 1] = g1_element.offset[(i * 3) + 1];
				RCT2_ADDRESS(RCT2_ADDRESS_PALETTE + xoffset, uint8)[(i * 4) + 2] = g1_element.offset[(i * 3) + 2];
			}
		}

		// animate the water/lava/chain movement palette
		int q = 0;
		int weather_colour = RCT2_ADDRESS(0x98195C, uint32)[RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_WEATHER_GLOOM, uint8)];
		if (weather_colour != -1) {
			q = 1;
			if (weather_colour != 0x2000031) {
				q = 2;
			}
		}
		uint32 j = RCT2_GLOBAL(RCT2_ADDRESS_PALETTE_EFFECT_FRAME_NO, uint32);
		j = (((uint16)((~j / 2) * 128) * 15) >> 16);
		int p = 1533;
		if ((sint32)water_type != -1) {
			p = water_type->var_06;
		}
		rct_g1_element g1_element = g1Elements[q + p];
		uint8* vs = &g1_element.offset[j * 3];
		uint8* vd = RCT2_ADDRESS(0x01424A18, uint8);
		int n = 5;
		for (int i = 0; i < n; i++) {
			vd[0] = vs[0];
			vd[1] = vs[1];
			vd[2] = vs[2];
			vs += 9;
			if (vs >= &g1_element.offset[9 * n]) {
				vs -= 9 * n;
			}
			vd += 4;
		}

		p = 1536;
		if ((sint32)water_type != -1) {
			p = water_type->var_0A;
		}
		g1_element = g1Elements[q + p];
		vs = &g1_element.offset[j * 3];
		n = 5;
		for (int i = 0; i < n; i++) {
			vd[0] = vs[0];
			vd[1] = vs[1];
			vd[2] = vs[2];
			vs += 9;
			if (vs >= &g1_element.offset[9 * n]) {
				vs -= 9 * n;
			}
			vd += 4;
		}

		j = ((uint16)(RCT2_GLOBAL(RCT2_ADDRESS_PALETTE_EFFECT_FRAME_NO, uint32) * -960) * 3) >> 16;
		p = 1539;
		g1_element = g1Elements[q + p];
		vs = &g1_element.offset[j * 3];
		vd += 12;
		n = 3;
		for (int i = 0; i < n; i++) {
			vd[0] = vs[0];
			vd[1] = vs[1];
			vd[2] = vs[2];
			vs += 3;
			if (vs >= &g1_element.offset[3 * n]) {
				vs -= 3 * n;
			}
			vd += 4;
		}

		RCT2_GLOBAL(0x014241BC, uint32) = 2;
		platform_update_palette(RCT2_ADDRESS(RCT2_ADDRESS_PALETTE, uint8), 230, 16);
		RCT2_GLOBAL(0x014241BC, uint32) = 0;
		if (RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) == 2) {
			RCT2_GLOBAL(0x014241BC, uint32) = 2;
			platform_update_palette(RCT2_ADDRESS(RCT2_ADDRESS_PALETTE, uint8), 10, 236);
			RCT2_GLOBAL(0x014241BC, uint32) = 0;
			RCT2_GLOBAL(RCT2_ADDRESS_LIGHTNING_ACTIVE, uint8) = 0;
		}
	}
	if (RCT2_GLOBAL(0x009E2C4C, uint32) == 2 || RCT2_GLOBAL(0x009E2C4C, uint32) == 1) {
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_CAP_BPP, uint32) != 8) {
			RCT2_GLOBAL(0x009E2C78, int) = 1;
		}
	}
}

void game_update()
{
	int i, numUpdates;

	// 0x006E3AEC // screen_game_process_mouse_input();
	screenshot_check();
	game_handle_keyboard_input();

	// Determine how many times we need to update the game
	if (gGameSpeed > 1) {
		numUpdates = 1 << (gGameSpeed - 1);
	} else {
		numUpdates = RCT2_GLOBAL(RCT2_ADDRESS_TICKS_SINCE_LAST_UPDATE, uint16) / 31;
		numUpdates = clamp(1, numUpdates, 4);
	}

	if (network_get_mode() == NETWORK_MODE_CLIENT && network_get_status() == NETWORK_STATUS_CONNECTED && network_get_authstatus() == NETWORK_AUTH_OK) {
		if (network_get_server_tick() - RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) >= 10) {
			// make sure client doesn't fall behind the server too much
			numUpdates += 10;
		}
	} else {
		if (RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint8) != 0) {
			numUpdates = 0;
		}
	}

	// Update the game one or more times
	for (i = 0; i < numUpdates; i++) {
		game_logic_update();
		start_title_music();

		if (gGameSpeed > 1)
			continue;

		// Possibly smooths viewport scrolling, I don't see a difference though
		if (RCT2_GLOBAL(0x009E2D74, uint32) == 1) {
			RCT2_GLOBAL(0x009E2D74, uint32) = 0;
			break;
		} else {
			if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_STATE, uint8) == INPUT_STATE_RESET ||
				RCT2_GLOBAL(RCT2_ADDRESS_INPUT_STATE, uint8) == INPUT_STATE_NORMAL
			) {
				if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & INPUT_FLAG_VIEWPORT_SCROLLING) {
					RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_VIEWPORT_SCROLLING;
					break;
				}
			} else {
				break;
			}
		}
	}

	// Always perform autosave check, even when paused
	scenario_autosave_check();

	network_update();
	news_item_update_current();
	window_dispatch_update_all();

	RCT2_GLOBAL(0x009A8C28, uint8) = 0;

	RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_VIEWPORT_SCROLLING;

	// the flickering frequency is reduced by 4, compared to the original
	// it was done due to inability to reproduce original frequency
	// and decision that the original one looks too fast
	if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) % 4 == 0)
		RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) ^= (1 << 15);

	// Handle guest map flashing
	RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) &= ~(1 << 1);
	if (RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) & (1 << 0))
		RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) |= (1 << 1);
	RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) &= ~(1 << 0);

	// Handle staff map flashing
	RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) &= ~(1 << 3);
	if (RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) & (1 << 2))
		RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) |= (1 << 3);
	RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_MAP_FLASHING_FLAGS, uint16) &= ~(1 << 2);

	window_map_tooltip_update_visibility();

	// Input
	RCT2_GLOBAL(0x0141F568, uint8) = RCT2_GLOBAL(0x0013CA740, uint8);
	game_handle_input();
}

void game_logic_update()
{
	///////////////////////////
	gInUpdateCode = true;
	///////////////////////////
	network_update();
	if (network_get_mode() == NETWORK_MODE_CLIENT && network_get_status() == NETWORK_STATUS_CONNECTED && network_get_authstatus() == NETWORK_AUTH_OK) {
		if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) >= network_get_server_tick()) {
			// dont run past the server
			return;
		}
	}
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32)++;
	RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_TICKS, uint32)++;
	RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_AGE, sint16)++;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_AGE, sint16) == 0)
		RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_AGE, sint16)--;

	sub_68B089();
	scenario_update();
	climate_update();
	map_update_tiles();
	sub_6A876D();
	peep_update_all();
	vehicle_update_all();
	sprite_misc_update_all();
	ride_update_all();
	park_update();
	research_update();
	ride_ratings_update_all();
	ride_measurements_update();
	///////////////////////////
	gInUpdateCode = false;
	///////////////////////////

	map_animation_invalidate_all();
	vehicle_sounds_update();
	peep_update_crowd_noise();
	climate_update_sound();
	editor_open_windows_for_current_step();

	RCT2_GLOBAL(RCT2_ADDRESS_SAVED_AGE, uint16)++;

	// Update windows
	//window_dispatch_update_all();

	if (RCT2_GLOBAL(RCT2_ADDRESS_ERROR_TYPE, uint8) != 0) {
		rct_string_id title_text = STR_UNABLE_TO_LOAD_FILE;
		rct_string_id body_text = RCT2_GLOBAL(RCT2_ADDRESS_ERROR_STRING_ID, uint16);
		if (RCT2_GLOBAL(RCT2_ADDRESS_ERROR_TYPE, uint8) == 254) {
			title_text = RCT2_GLOBAL(RCT2_ADDRESS_ERROR_STRING_ID, uint16);
			body_text = 0xFFFF;
		}
		RCT2_GLOBAL(RCT2_ADDRESS_ERROR_TYPE, uint8) = 0;

		window_error_open(title_text, body_text);
	}
}

/**
 *
 *  rct2: 0x0069C62C
 *
 * @param cost (ebp)
 */
static int game_check_affordability(int cost)
{
	if (cost <= 0)return cost;
	if (RCT2_GLOBAL(0x141F568, uint8) & 0xF0)return cost;

	if (!(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32)&(1 << 8))){
		if (cost <= (sint32)(DECRYPT_MONEY(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, sint32))))return cost;
	}
	RCT2_GLOBAL(RCT2_ADDRESS_COMMON_FORMAT_ARGS, uint32) = cost;

	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 827;
	return MONEY32_UNDEFINED;
}

static GAME_COMMAND_POINTER* new_game_command_table[62];

/**
 *
 *  rct2: 0x006677F2
 *
 * @param flags (ebx)
 * @param command (esi)
 */
int game_do_command(int eax, int ebx, int ecx, int edx, int esi, int edi, int ebp)
{
	return game_do_command_p(esi, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
}

/**
*
*  rct2: 0x006677F2 with pointers as arguments
*
* @param flags (ebx)
* @param command (esi)
*/
int game_do_command_p(int command, int *eax, int *ebx, int *ecx, int *edx, int *esi, int *edi, int *ebp)
{
	int cost, flags, insufficientFunds;
	int original_ebx, original_edx, original_esi, original_edi, original_ebp;

	*esi = command;
	original_ebx = *ebx;
	original_edx = *edx;
	original_esi = *esi;
	original_edi = *edi;
	original_ebp = *ebp;

	flags = *ebx;
	RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16) = 0xFFFF;

	// Increment nest count
	RCT2_GLOBAL(0x009A8C28, uint8)++;

	// Remove ghost scenery so it doesn't interfere with incoming network command
	if ((flags & GAME_COMMAND_FLAG_NETWORKED) && !(flags & GAME_COMMAND_FLAG_GHOST) &&
		(command == GAME_COMMAND_PLACE_FENCE ||
		command == GAME_COMMAND_PLACE_SCENERY ||
		command == GAME_COMMAND_PLACE_LARGE_SCENERY ||
		command == GAME_COMMAND_PLACE_BANNER ||
		command == GAME_COMMAND_PLACE_PATH)) {
		scenery_remove_ghost_tool_placement();
	}

	*ebx &= ~GAME_COMMAND_FLAG_APPLY;

	// First call for validity and price check
	new_game_command_table[command](eax, ebx, ecx, edx, esi, edi, ebp);
	cost = *ebx;

	if (cost != MONEY32_UNDEFINED) {
		// Check funds
		insufficientFunds = 0;
		if (RCT2_GLOBAL(0x009A8C28, uint8) == 1 && !(flags & GAME_COMMAND_FLAG_2) && !(flags & GAME_COMMAND_FLAG_5) && cost != 0)
			insufficientFunds = game_check_affordability(cost);

		if (insufficientFunds != MONEY32_UNDEFINED) {
			*ebx = original_ebx;
			*edx = original_edx;
			*esi = original_esi;
			*edi = original_edi;
			*ebp = original_ebp;

			if (!(flags & GAME_COMMAND_FLAG_APPLY)) {
				// Decrement nest count
				RCT2_GLOBAL(0x009A8C28, uint8)--;
				return cost;
			}

			if (network_get_mode() != NETWORK_MODE_NONE && !(flags & GAME_COMMAND_FLAG_NETWORKED) && !(flags & GAME_COMMAND_FLAG_GHOST) && !(flags & GAME_COMMAND_FLAG_5) && RCT2_GLOBAL(0x009A8C28, uint8) == 1 /* Send only top-level commands */) {
				if (command != GAME_COMMAND_LOAD_OR_QUIT) { // Disable these commands over the network
					network_send_gamecmd(*eax, *ebx, *ecx, *edx, *esi, *edi, *ebp, game_command_callback_get_index(game_command_callback));
					if (network_get_mode() == NETWORK_MODE_CLIENT) { // Client sent the command to the server, do not run it locally, just return.  It will run when server sends it
						game_command_callback = 0;
						// Decrement nest count
						RCT2_GLOBAL(0x009A8C28, uint8)--;
						return cost;
					}
				}
			}

			// Second call to actually perform the operation
			new_game_command_table[command](eax, ebx, ecx, edx, esi, edi, ebp);

			if (game_command_callback) {
				game_command_callback(*eax, *ebx, *ecx, *edx, *esi, *edi, *ebp);
				game_command_callback = 0;
			}

			*edx = *ebx;

			if (*edx != MONEY32_UNDEFINED && *edx < cost)
				cost = *edx;

			// Decrement nest count
			RCT2_GLOBAL(0x009A8C28, uint8)--;
			if (RCT2_GLOBAL(0x009A8C28, uint8) != 0)
				return cost;

			//
			if (!(flags & 0x20)) {
				// Update money balance
				finance_payment(cost, RCT2_GLOBAL(RCT2_ADDRESS_NEXT_EXPENDITURE_TYPE, uint8) / 4);
				if (RCT2_GLOBAL(0x0141F568, uint8) == RCT2_GLOBAL(0x013CA740, uint8)) {
					// Create a +/- money text effect
					if (cost != 0)
						money_effect_create(cost);
				}
			}

			return cost;
		}
	}

	// Error occured

	// Decrement nest count
	RCT2_GLOBAL(0x009A8C28, uint8)--;

	// Show error window
	if (RCT2_GLOBAL(0x009A8C28, uint8) == 0 && (flags & GAME_COMMAND_FLAG_APPLY) && RCT2_GLOBAL(0x0141F568, uint8) == RCT2_GLOBAL(0x013CA740, uint8) && !(flags & GAME_COMMAND_FLAG_ALLOW_DURING_PAUSED))
		window_error_open(RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16), RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TEXT, uint16));

	return MONEY32_UNDEFINED;
}

void pause_toggle()
{
	RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint32) ^= 1;
	window_invalidate_by_class(WC_TOP_TOOLBAR);
	if (RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint32) & 1) {
		pause_sounds();
		unpause_sounds();
	} else {
		unpause_sounds();
	}
}

/**
 *
 *  rct2: 0x00667C15
 */
void game_pause_toggle(int *eax, int *ebx, int *ecx, int *edx, int *esi, int *edi, int *ebp)
{
	if (*ebx & GAME_COMMAND_FLAG_APPLY)
		pause_toggle();

	*ebx = 0;
}

/**
 *
 *  rct2: 0x0066DB5F
 */
static void game_load_or_quit(int *eax, int *ebx, int *ecx, int *edx, int *esi, int *edi, int *ebp)
{
	if (*ebx & GAME_COMMAND_FLAG_APPLY) {
		switch (*edx & 0xFF) {
		case 0:
			RCT2_GLOBAL(RCT2_ADDRESS_SAVE_PROMPT_MODE, uint16) = *edi & 0xFF;
			window_save_prompt_open();
			break;
		case 1:
			window_close_by_class(WC_SAVE_PROMPT);
			break;
		default:
			game_load_or_quit_no_save_prompt();
			break;
		}
	}
	*ebx = 0;
}

/**
 *
 *  rct2: 0x00674F40
 */
static int open_landscape_file_dialog()
{
	int result;
	format_string((char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, STR_LOAD_LANDSCAPE_DIALOG_TITLE, 0);
	safe_strncpy((char*)0x0141EF68, (char*)RCT2_ADDRESS_LANDSCAPES_PATH, MAX_PATH);
	format_string((char*)0x0141EE68, STR_RCT2_LANDSCAPE_FILE, 0);
	pause_sounds();
	result = platform_open_common_file_dialog(1, (char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, (char*)0x0141EF68, "*.SV6;*.SV4;*.SC6", (char*)0x0141EE68);
	unpause_sounds();
	// window_proc
	return result;
}

/**
 *
 *  rct2: 0x00674EB6
 */
static int open_load_game_dialog()
{
	int result;
	format_string((char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, STR_LOAD_GAME_DIALOG_TITLE, 0);
	safe_strncpy((char*)0x0141EF68, (char*)RCT2_ADDRESS_SAVED_GAMES_PATH, MAX_PATH);
	format_string((char*)0x0141EE68, STR_RCT2_SAVED_GAME, 0);
	pause_sounds();
	result = platform_open_common_file_dialog(1, (char*)RCT2_ADDRESS_COMMON_STRING_FORMAT_BUFFER, (char*)0x0141EF68, "*.SV6", (char*)0x0141EE68);
	unpause_sounds();
	// window_proc
	return result;
}

/**
 *
 *  rct2: 0x0066DC0F
 */
static void load_landscape()
{
	window_loadsave_open(LOADSAVETYPE_LOAD | LOADSAVETYPE_LANDSCAPE, NULL);
	return;

	if (open_landscape_file_dialog() == 0) {
		gfx_invalidate_screen();
	} else {
		// Set default filename
		char *esi = (char*)0x0141EF67;
		while (1) {
			esi++;
			if (*esi == '.')
				break;
			if (*esi != 0)
				continue;
			strcpy(esi, ".SC6");
			break;
		}
		safe_strncpy((char*)RCT2_ADDRESS_SAVED_GAMES_PATH_2, (char*)0x0141EF68, MAX_PATH);

		editor_load_landscape((char*)0x0141EF68);
		if (1) {
			gfx_invalidate_screen();
			rct2_endupdate();
		} else {
			RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_AGE, uint16) = 0;
			rct2_endupdate();
		}
	}
}

/**
 * Converts all the user strings and news item strings to UTF-8.
 */
void game_convert_strings_to_utf8()
{
	utf8 buffer[512];

	// User strings
	for (int i = 0; i < MAX_USER_STRINGS; i++) {
		utf8 *userString = &gUserStrings[i * USER_STRING_MAX_LENGTH];

		if (!str_is_null_or_empty(userString)) {
			rct2_to_utf8(buffer, userString);
			memcpy(userString, buffer, 31);
			userString[31] = 0;
		}
	}

	// News items
	for (int i = 0; i < MAX_NEWS_ITEMS; i++) {
		rct_news_item *newsItem = news_item_get(i);

		if (!str_is_null_or_empty(newsItem->text)) {
			rct2_to_utf8(buffer, newsItem->text);
			memcpy(newsItem->text, buffer, 255);
			newsItem->text[255] = 0;
		}
	}
}

/**
 * Converts all the user strings and news item strings to RCT2 encoding.
 */
void game_convert_strings_to_rct2(rct_s6_data *s6)
{
	char buffer[512];

	// User strings
	for (int i = 0; i < MAX_USER_STRINGS; i++) {
		char *userString = &s6->custom_strings[i * USER_STRING_MAX_LENGTH];

		if (!str_is_null_or_empty(userString)) {
			utf8_to_rct2(buffer, userString);
			memcpy(userString, buffer, 31);
			userString[31] = 0;
		}
	}

	// News items
	for (int i = 0; i < MAX_NEWS_ITEMS; i++) {
		rct_news_item *newsItem = &s6->news_items[i];

		if (!str_is_null_or_empty(newsItem->text)) {
			utf8_to_rct2(buffer, newsItem->text);
			memcpy(newsItem->text, buffer, 255);
			newsItem->text[255] = 0;
		}
	}
}

/**
 *
 *  rct2: 0x00675E1B
 */
int game_load_sv6(SDL_RWops* rw)
{
	int i, j;

	if (!sawyercoding_validate_checksum(rw)) {
		log_error("invalid checksum");

		RCT2_GLOBAL(RCT2_ADDRESS_ERROR_TYPE, uint8) = 255;
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_FILE_CONTAINS_INVALID_DATA;
		return 0;
	}

	rct_s6_header *s6Header = (rct_s6_header*)0x009E34E4;
	rct_s6_info *s6Info = (rct_s6_info*)0x0141F570;

	// Read first chunk
	sawyercoding_read_chunk(rw, (uint8*)s6Header);
	if (s6Header->type == S6_TYPE_SAVEDGAME) {
		// Read packed objects
		if (s6Header->num_packed_objects > 0) {
			j = 0;
			for (i = 0; i < s6Header->num_packed_objects; i++)
				j += object_load_packed(rw);
			if (j > 0)
				object_list_load();
		}
	}

	uint8 load_success = object_read_and_load_entries(rw);

	// Read flags (16 bytes)
	sawyercoding_read_chunk(rw, (uint8*)RCT2_ADDRESS_CURRENT_MONTH_YEAR);

	// Read map elements
	memset((void*)RCT2_ADDRESS_MAP_ELEMENTS, 0, MAX_MAP_ELEMENTS * sizeof(rct_map_element));
	sawyercoding_read_chunk(rw, (uint8*)RCT2_ADDRESS_MAP_ELEMENTS);

	// Read game data, including sprites
	sawyercoding_read_chunk(rw, (uint8*)0x010E63B8);

	if (!load_success){
		set_load_objects_fail_reason();
		if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & INPUT_FLAG_5){
			//call 0x0040705E Sets cursor position and something else. Calls maybe wind func 8 probably pointless
			RCT2_GLOBAL(0x14241BC, uint32) = 0;
			RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_5;
		}

		return 0;//This never gets called
	}

	// The rest is the same as in scenario_load
	reset_loaded_objects();
	map_update_tile_pointers();
	reset_0x69EBE4();
	openrct2_reset_object_tween_locations();
	game_convert_strings_to_utf8();
	game_fix_save_vars(); // OpenRCT2 fix broken save games

	return 1;
}

// OpenRCT2 workaround to recalculate some values which are saved redundantly in the save to fix corrupted files.
// For example recalculate guest count by looking at all the guests instead of trusting the value in the file.
void game_fix_save_vars() {

	// Recalculates peep count after loading a save to fix corrupted files
	rct_peep* peep;
	uint16 spriteIndex;
	uint16 peepCount = 0;
	FOR_ALL_GUESTS(spriteIndex, peep) {
		if(!peep->outside_of_park)
			peepCount++;
	}

	RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_IN_PARK, uint16) = peepCount;

	// Fixes broken saves where a surface element could be null
	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 256; x++) {
			rct_map_element *mapElement = map_get_surface_element_at(x, y);

			if (mapElement == NULL)
			{
				log_error("Null map element at x = %d and y = %d. Fixing...", x, y);
				map_element_insert(x, y, 14, 0);
			}
		}
	}
}

// Load game state for multiplayer
int game_load_network(SDL_RWops* rw)
{
	int i, j;

	rct_s6_header *s6Header = (rct_s6_header*)0x009E34E4;
	rct_s6_info *s6Info = (rct_s6_info*)0x0141F570;

	// Read first chunk
	sawyercoding_read_chunk(rw, (uint8*)s6Header);
	if (s6Header->type == S6_TYPE_SAVEDGAME) {
		// Read packed objects
		if (s6Header->num_packed_objects > 0) {
			j = 0;
			for (i = 0; i < s6Header->num_packed_objects; i++)
				j += object_load_packed(rw);
			if (j > 0)
				object_list_load();
		}
	}

	uint8 load_success = object_read_and_load_entries(rw);

	// Read flags (16 bytes)
	sawyercoding_read_chunk(rw, (uint8*)RCT2_ADDRESS_CURRENT_MONTH_YEAR);

	// Read map elements
	memset((void*)RCT2_ADDRESS_MAP_ELEMENTS, 0, MAX_MAP_ELEMENTS * sizeof(rct_map_element));
	sawyercoding_read_chunk(rw, (uint8*)RCT2_ADDRESS_MAP_ELEMENTS);

	// Read game data, including sprites
	sawyercoding_read_chunk(rw, (uint8*)0x010E63B8);

	// Read checksum
	uint32 checksum;
	SDL_RWread(rw, &checksum, sizeof(uint32), 1);

	// Read other data not in normal save files
	RCT2_GLOBAL(RCT2_ADDRESS_GAME_PAUSED, uint32) = SDL_ReadLE32(rw);

	if (!load_success){
		set_load_objects_fail_reason();
		if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & INPUT_FLAG_5){
			//call 0x0040705E Sets cursor position and something else. Calls maybe wind func 8 probably pointless
			RCT2_GLOBAL(0x14241BC, uint32) = 0;
			RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_5;
		}

		return 0;//This never gets called
	}

	// The rest is the same as in scenario load and play
	reset_loaded_objects();
	map_update_tile_pointers();
	reset_0x69EBE4();
	openrct2_reset_object_tween_locations();
	return 1;
}

/**
 *
 *  rct2: 0x00675E1B
 */
int game_load_save(const char *path)
{
	log_verbose("loading saved game, %s", path);

	safe_strncpy((char*)0x0141EF68, path, MAX_PATH);
	safe_strncpy((char*)RCT2_ADDRESS_SAVED_GAMES_PATH_2, path, MAX_PATH);

	safe_strncpy(gScenarioSaveName, path_get_filename(path), MAX_PATH);
	path_remove_extension(gScenarioSaveName);

	SDL_RWops* rw = SDL_RWFromFile(path, "rb");
	if (rw == NULL) {
		log_error("unable to open %s", path);
		RCT2_GLOBAL(RCT2_ADDRESS_ERROR_TYPE, uint8) = 255;
		RCT2_GLOBAL(RCT2_ADDRESS_GAME_COMMAND_ERROR_TITLE, uint16) = STR_FILE_CONTAINS_INVALID_DATA;
		return 0;
	}

	if (!game_load_sv6(rw)) {
		title_load();
		rct2_endupdate();
		SDL_RWclose(rw);
		return 0;
	}
	SDL_RWclose(rw);

	game_load_init();
	if (network_get_mode() == NETWORK_MODE_SERVER) {
		network_send_map();
	}
	return 1;
}

void game_load_init()
{
	rct_window *mainWindow;

	RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) = SCREEN_FLAGS_PLAYING;
	viewport_init_all();
	game_create_windows();
	mainWindow = window_get_main();

	mainWindow->viewport_target_sprite = -1;
	mainWindow->saved_view_x = RCT2_GLOBAL(RCT2_ADDRESS_SAVED_VIEW_X, sint16);
	mainWindow->saved_view_y = RCT2_GLOBAL(RCT2_ADDRESS_SAVED_VIEW_Y, sint16);
	uint8 _cl = (RCT2_GLOBAL(RCT2_ADDRESS_SAVED_VIEW_ZOOM_AND_ROTATION, sint16) & 0xFF) - mainWindow->viewport->zoom;
	mainWindow->viewport->zoom = RCT2_GLOBAL(RCT2_ADDRESS_SAVED_VIEW_ZOOM_AND_ROTATION, sint16) & 0xFF;
	*((char*)(&RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_ROTATION, sint32))) = RCT2_GLOBAL(RCT2_ADDRESS_SAVED_VIEW_ZOOM_AND_ROTATION, sint16) >> 8;
	if (_cl != 0) {
		if (_cl < 0) {
			_cl = -_cl;
			mainWindow->viewport->view_width >>= _cl;
			mainWindow->viewport->view_height >>= _cl;
		} else {
			mainWindow->viewport->view_width <<= _cl;
			mainWindow->viewport->view_height <<= _cl;
		}
	}
	mainWindow->saved_view_x -= mainWindow->viewport->view_width >> 1;
	mainWindow->saved_view_y -= mainWindow->viewport->view_height >> 1;
	window_invalidate(mainWindow);

	reset_all_sprite_quadrant_placements();
	scenery_set_default_placement_configuration();
	window_new_ride_init_vars();
	RCT2_GLOBAL(RCT2_ADDRESS_WINDOW_UPDATE_TICKS, uint16) = 0;
	if (RCT2_GLOBAL(0x0013587C4, uint32) == 0)		// this check is not in scenario play
		finance_update_loan_hash();

	load_palette();
	gfx_invalidate_screen();
	window_update_all();

	gGameSpeed = 1;

	scenario_set_filename((char*)0x0135936C);
}

/*
 *
 * rct2: 0x0069E9A7
 * Call after a rotation or loading of a save to reset sprite quadrants
 */
void reset_all_sprite_quadrant_placements()
{
	for (rct_sprite* spr = g_sprite_list; spr < (rct_sprite*)RCT2_ADDRESS_SPRITES_NEXT_INDEX; spr++)
		if (spr->unknown.sprite_identifier != 0xFF)
			sprite_move(spr->unknown.x, spr->unknown.y, spr->unknown.z, spr);
}

/**
 *
 *  rct2: 0x0066DBB7
 */
static void load_game()
{
	window_loadsave_open(LOADSAVETYPE_LOAD | LOADSAVETYPE_GAME, NULL);
	return;

	if (open_load_game_dialog() == 0) {
		gfx_invalidate_screen();
	} else {
		// Set default filename
		char *esi = (char*)0x0141EF67;
		while (1) {
			esi++;
			if (*esi == '.')
				break;
			if (*esi != 0)
				continue;
			strcpy(esi, ".SV6");
			break;
		}
		safe_strncpy((char*)RCT2_ADDRESS_SAVED_GAMES_PATH_2, (char*)0x0141EF68, MAX_PATH);

		if (game_load_save((char *)0x0141EF68)) {
			gfx_invalidate_screen();
			rct2_endupdate();
		} else {
			RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_AGE, uint16) = 0;
			rct2_endupdate();
		}
	}
}

/**
 *
 *  rct2: 0x006750E9
 */
static int show_save_game_dialog(char *resultPath)
{
	rct_s6_info *s6Info = (rct_s6_info*)0x0141F570;

	int result;
	char title[256];
	char filename[MAX_PATH];
	char filterName[256];

	format_string(title, STR_SAVE_GAME_1040, NULL);
	safe_strncpy(filename, RCT2_ADDRESS(RCT2_ADDRESS_SAVED_GAMES_PATH_2, char), MAX_PATH);
	format_string(filterName, STR_RCT2_SAVED_GAME, NULL);

	pause_sounds();
	result = platform_open_common_file_dialog(0, title, filename, "*.SV6", filterName);
	unpause_sounds();

	if (result)
		safe_strncpy(resultPath, filename, MAX_PATH);
	return result;
}

void save_game()
{
	if (!gFirstTimeSave) {
		utf8 path[MAX_PATH];

		log_verbose("Saving to %s", gScenarioSaveName);

		platform_get_user_directory(path, "save");

		strcat(path, gScenarioSaveName);
		strcat(path, ".sv6");

		SDL_RWops* rw = SDL_RWFromFile(path, "wb+");
		if (rw != NULL) {
			scenario_save(rw, 0x80000000);
			log_verbose("Saved to %s", gScenarioSaveName);
			SDL_RWclose(rw);
		}
	} else {
		save_game_as();
	}

}
void save_game_as()
{
	window_loadsave_open(LOADSAVETYPE_SAVE | LOADSAVETYPE_GAME, gScenarioSaveName);
}


void game_autosave()
{
	utf8 path[MAX_PATH];
	utf8 backupPath[MAX_PATH];

	platform_get_user_directory(path, "save");
	safe_strncpy(backupPath, path, MAX_PATH);

	strcat(path, "autosave.sv6");
	strcat(backupPath, "autosave.sv6.bak");

	if (platform_file_exists(path)) {
		platform_file_copy(path, backupPath, true);
	}

	SDL_RWops* rw = SDL_RWFromFile(path, "wb+");
	if (rw != NULL) {
		scenario_save(rw, 0x80000000);
		SDL_RWclose(rw);
	}
}

/**
*
*  rct2: 0x006E3838
*/
void rct2_exit_reason(rct_string_id title, rct_string_id body){
	// Before this would set a quit message

	char exit_title[255];
	format_string(exit_title, title, 0);

	char exit_body[255];
	format_string(exit_body, body, 0);

	log_error(exit_title);
	log_error(exit_body);

	rct2_exit();
}


/**
 *
 *  rct2: 0x006E3879
 */
void rct2_exit()
{
	//audio_close();
	//Post quit message does not work in 0x6e3879 as its windows only.
	openrct2_finish();
}

/**
 *
 *  rct2: 0x0066DB79
 */
void game_load_or_quit_no_save_prompt()
{
	if (RCT2_GLOBAL(RCT2_ADDRESS_SAVE_PROMPT_MODE, uint16) < 1) {
		game_do_command(0, 1, 0, 1, GAME_COMMAND_LOAD_OR_QUIT, 0, 0);
		tool_cancel();
		if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & 2)
			load_landscape();
		else
			load_game();
	} else if (RCT2_GLOBAL(RCT2_ADDRESS_SAVE_PROMPT_MODE, uint16) == 1) {
		game_do_command(0, 1, 0, 1, GAME_COMMAND_LOAD_OR_QUIT, 0, 0);
		tool_cancel();
		if (RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) & INPUT_FLAG_5) {
			// RCT2_CALLPROC_EBPSAFE(0x0040705E); Function not required resets cursor position.
			RCT2_GLOBAL(RCT2_ADDRESS_INPUT_FLAGS, uint32) &= ~INPUT_FLAG_5;
		}
		gGameSpeed = 1;
		title_load();
		rct2_endupdate();
	} else {
		rct2_exit();
	}
}

static GAME_COMMAND_POINTER* new_game_command_table[62] = {
	game_command_set_ride_appearance,
	game_command_set_land_height,
	game_pause_toggle,
	game_command_place_track,
	game_command_remove_track,
	game_load_or_quit,
	game_command_create_ride,
	game_command_demolish_ride,
	game_command_set_ride_status,
	game_command_set_ride_vehicles,
	game_command_set_ride_name,
	game_command_set_ride_setting,
	game_command_place_ride_entrance_or_exit,
	game_command_remove_ride_entrance_or_exit,
	game_command_remove_scenery,
	game_command_place_scenery,
	game_command_set_water_height,
	game_command_place_footpath,
	game_command_place_footpath_from_track,
	game_command_remove_footpath,
	game_command_change_surface_style,
	game_command_set_ride_price,
	game_command_set_peep_name,
	game_command_raise_land,
	game_command_lower_land,
	game_command_smooth_land,
	game_command_raise_water,
	game_command_lower_water,
	game_command_set_brakes_speed,
	game_command_hire_new_staff_member,
	game_command_set_staff_patrol,
	game_command_fire_staff_member,
	game_command_set_staff_order,
	game_command_set_park_name,
	game_command_set_park_open,
	game_command_buy_land_rights,
	game_command_place_park_entrance,
	game_command_remove_park_entrance,
	game_command_set_maze_track,
	game_command_set_park_entrance_fee,
	game_command_update_staff_colour,
	game_command_place_fence,
	game_command_remove_fence,
	game_command_place_large_scenery,
	game_command_remove_large_scenery,
	game_command_set_current_loan,
	game_command_set_research_funding,
	game_command_place_track_design,
	game_command_start_campaign,
	game_command_place_maze_design,
	game_command_place_banner,
	game_command_remove_banner,
	game_command_set_scenery_colour,
	game_command_set_fence_colour,
	game_command_set_large_scenery_colour,
	game_command_set_banner_colour,
	game_command_set_land_ownership,
	game_command_clear_scenery,
	game_command_set_banner_name,
	game_command_set_sign_name,
	game_command_set_banner_style,
	game_command_set_sign_style
};
