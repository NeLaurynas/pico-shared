// Copyright (C) 2024 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/pwm.h>
#include <pico/time.h>

#include "shared_config.h"
#include "pico/rand.h"

inline u8 utils_min_u8(const u8 x, const u8 y) {
	return x < y ? x : y;
}

inline u16 utils_min_u16(const u16 x, const u16 y) {
	return x < y ? x : y;
}

inline u32 utils_min_u32(const u32 x, const u32 y) {
	return x < y ? x : y;
}

void utils_swap_u8(u8 *x, u8 *y) {
	const auto tmp = *x;
	*x = *y;
	*y = tmp;
}

void utils_swap_u16(u16 *x, u16 *y) {
	const auto tmp = *x;
	*x = *y;
	*y = tmp;
}

void utils_swap_u32(u32 *x, u32 *y) {
	const auto tmp = *x;
	*x = *y;
	*y = tmp;
}

u32 utils_random_in_range(u32 fromInclusive, u32 toInclusive) {
	if (fromInclusive > toInclusive) {
		const auto tmp = toInclusive;
		toInclusive = fromInclusive;
		fromInclusive = tmp;
	}

	const auto range = toInclusive - fromInclusive + 1; // +1 because to is inclusive
	const auto rnd = get_rand_32();
	return fromInclusive + (rnd % range);
}

float utils_print_onboard_temp() {
	constexpr float conversionFactor = 3.3f / (1 << 12);

	const float adc = (float)adc_read() * conversionFactor;
	const float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

#if DBG
	printf("Onboard temperature = %.02f C\n", tempC);
#endif

	return tempC;
}

void utils_print_cpu_speed() {
#if DBG
	const auto freq_hz = clock_get_hz(clk_sys);

	const float freq_mhz = (float)freq_hz / 1'000'000.0f;
	printf("System clock: %.2f MHz\n", freq_mhz);
#endif
}

float utils_calculate_pio_clk_div(const float instruction_execution_in_us) {
	const auto frequency_hz = clock_get_hz(clk_sys);
	auto const clk_div = ((float)frequency_hz * instruction_execution_in_us) / 1'000'000.0f;
	return clk_div;
}

float utils_calculate_pio_clk_div_ns(const float instruction_execution_in_ns) {
	return utils_calculate_pio_clk_div(instruction_execution_in_ns / 1'000.0f);
}

inline u32 utils_time_diff_ms(const u32 start_us, const u32 end_us) {
	return (end_us - start_us) / 1000;
}

inline u32 utils_time_diff_us(const u32 start_us, const u32 end_us) {
	return end_us - start_us;
}

void utils_error_mode(const i32 code) {
	utils_internal_led(false);
	const u8 long_blink = code / 10;
	const u8 short_blink = code % 10;
	// ReSharper disable once CppDFAEndlessLoop
	for (;;) {
		utils_printf("!!! ERROR MODE: %d\n", code);
		for (u8 i = 0; i < long_blink; i++) {
			utils_internal_led(true);
			sleep_ms(500);
			utils_internal_led(false);
			sleep_ms(500);
		}
		sleep_ms(400);
		for (u8 i = 0; i < short_blink; i++) {
			utils_internal_led(true);
			sleep_ms(75);
			utils_internal_led(false);
			sleep_ms(700);
		}
		sleep_ms(3'000); // sleep for 3 seconds
	}
}

void utils_internal_led(const bool on) {
	if (INTERNAL_LED == 0) return; // don't touch wifi led...
	gpio_put(INTERNAL_LED, on);
}

u16 utils_proportional_reduce(const u16 number, u16 step, const u16 total_steps) {
	if (step >= total_steps) step = total_steps;
	return (float)number / total_steps * step;
}

i32 utils_scaled_pwm_percentage(const i32 val, const i32 deadzone, const i32 max_val) {
	const i32 x = abs(val);
	if (x <= deadzone) {
		return 0;
	}
	if (x >= max_val) {
		return 100;
	}

	return (x - deadzone) * 100 / (max_val - deadzone);
}

u16 *utils_pwm_cc_for_16bit(const u8 slice, const u8 channel) {
	assert(channel == 0 || channel == 1);

	return (u16*)&pwm_hw->slice[slice].cc + channel;
}

void utils_print_time_elapsed(const char *title, const u32 start_us) {
	const auto end = time_us_32();
	const auto elapsed = utils_time_diff_us(start_us, end);
	const float elapsed_ms = (float)elapsed / 1000.0f;
	utils_printf("'%s' took: %.2f ms (%ld us)\n", title, elapsed_ms, elapsed);
}
