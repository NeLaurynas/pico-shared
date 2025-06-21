// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "wsleds.h"

#include <math.h>
#include <pio_wsleds.pio.h>
#include <stdlib.h>
#include <string.h>
#include <hardware/dma.h>
#include <hardware/pio.h>
#include <pico/time.h>

#include "anim.h"
#include "utils.h"
#include "wsleds_data.h"

// for square
// static const u8 line_width = (u8)sqrt(MOD_WSLEDS_LED_COUNT);

u32 wsleds_buffer[MOD_WSLEDS_LED_COUNT] = { 0 };

void wsleds_buffer_transfer() {
	dma_channel_transfer_from_buffer_now(MOD_WSLEDS_DMA_CH, wsleds_buffer, MOD_WSLEDS_LED_COUNT);
}

void wsleds_init() {
	// init DMA
	if (dma_channel_is_claimed(MOD_WSLEDS_DMA_CH)) utils_error_mode(25);
	dma_channel_claim(MOD_WSLEDS_DMA_CH);
	dma_channel_config dma_c = dma_channel_get_default_config(MOD_WSLEDS_DMA_CH);
	channel_config_set_transfer_data_size(&dma_c, DMA_SIZE_32);
	channel_config_set_read_increment(&dma_c, true); // incr true - we loop through MOD_WSLEDS_LED_COUNT size buffer
	channel_config_set_write_increment(&dma_c, false);
	channel_config_set_dreq(&dma_c, pio_get_dreq(MOD_WSLEDS_PIO, MOD_WSLEDS_SM, true));
	dma_channel_configure(MOD_WSLEDS_DMA_CH, &dma_c, &MOD_WSLEDS_PIO->txf[MOD_WSLEDS_SM], wsleds_buffer, MOD_WSLEDS_LED_COUNT,
	                      false);
	sleep_ms(1);

	// get clock divider
	const auto clk_div = utils_calculate_pio_clk_div_ns(98);
	utils_printf("WSLEDS PIO CLK DIV: %f\n", clk_div);

	// init PIO
	const auto offset = pio_add_program(MOD_WSLEDS_PIO, &pio_wsleds_program);
	if (offset < 0) utils_error_mode(26);
	if (pio_sm_is_claimed(MOD_WSLEDS_PIO, MOD_WSLEDS_SM)) utils_error_mode(27);
	pio_sm_claim(MOD_WSLEDS_PIO, MOD_WSLEDS_SM);
	pio_wsleds_program_init(MOD_WSLEDS_PIO, MOD_WSLEDS_SM, offset, MOD_WSLEDS_PIN, clk_div);
	pio_sm_set_enabled(MOD_WSLEDS_PIO, MOD_WSLEDS_SM, true);
	sleep_ms(1);

	// buffer_transfer();
}

void wsleds_rotate_buffer_left(const u8 times) {
	static u32 temp[MOD_WSLEDS_LED_COUNT] = { 0 };
	if (times == 0) return;

	for (auto t = 0; t < times; t++) {
		for (auto i = 0; i < 8; i++) for (auto j = 0; j < 8; j++) temp[(7 - j) * 8 + i] = wsleds_buffer[i * 8 + j];

		for (auto i = 0; i < MOD_WSLEDS_LED_COUNT; i++) {
			wsleds_buffer[i] = temp[i];
		}
	}
}

// static void anim_countdown() {
// 	static bool init = false;
// 	static i8 number = 0;
// 	static u16 frame = 0;
// 	static constexpr u16 FRAME_TICKS = 100; // every second
//
// 	if (!init) {
// 		number = 9;
// 		frame = 0;
// 		init = true;
// 		memset(wsleds_buffer, 0, sizeof(wsleds_buffer));
// 	}
//
// 	if (number == -2) {
// 		// deinit
// 		init = false;
// 		// state.phase = PHASE_EXPLOSION;
// 		memset(wsleds_buffer, 0, sizeof(wsleds_buffer));
// 		buffer_transfer();
// 		return;
// 	}
//
// 	if (frame % FRAME_TICKS == 0) {
// 		if (number >= 0) {
// 			memcpy(wsleds_buffer, WSLEDS_NUMBERS[number], sizeof(wsleds_buffer));
// 			rotate_buffer_left(3);
// 		}
// 		number--;
// 	}
// 	for (u8 i = 0; i < MOD_WSLEDS_LED_COUNT; i++) { // why use led count config, when a lot assumes it's 8x8...
// 		if (wsleds_buffer[i] == 0) continue;
//
// 		u32 color = COLOR_RED;
// 		if (number + 1 >= 0 && number + 1 <= 2) {
// 			color = COLOR_GREEN;
// 		} else if (number + 1 >= 3 && number + 1 <= 5) {
// 			color = COLOR_YELLOW;
// 		}
//
// 		wsleds_buffer[i] = reduce_brightness(anim_color_reduction(TO_DIM, frame, FRAME_TICKS, 1.00f, 1), color);
// 	}
//
// 	if (number != -2) {
// 		buffer_transfer();
// 		frame = (frame + 1) % FRAME_TICKS;
// 	}
// }
