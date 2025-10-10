// Copyright (C) 2024 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "utils.h"

#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <hardware/pwm.h>
#include <pico/rand.h>
#include <pico/status_led.h>
#include <pico/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared_config.h"
#include "shared_modules/memory/memory.h"

static bool crc_init = false;
static u32 *crc_tab;

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

void utils_random_bytes(u8 *buffer, const size_t len) {
	for (size_t i = 0; i < len; i += 4) {
		const auto r = get_rand_32();
		const auto chunk = utils_min((size_t)4, len - i);
		memcpy(buffer + i, &r, chunk);
	}
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

float utils_calculate_pwm_divider(const u32 top, const float freq_khz) {
	const auto clock = clock_get_hz(clk_sys);

	if (freq_khz <= 0.0f) {
		return 1.0f;
	}

	const float freq_hz = freq_khz * 1000.0f;
	float divider = (float)clock / (freq_hz * (top + 1));

	// Clamp the divider between 1.0 and 256.0
	if (divider < 1.0f) {
		utils_printf("!!! DIVIDER LESS THAN 1 (%f), CONSIDER ADJUSTING TOP\n", divider);
		divider = 1.0f;
	} else if (divider > 256.0f) {
		utils_printf("!!! DIVIDER MORE THAN 256 (%f), CONSIDER ADJUSTING TOP\n", divider);
		divider = 256.0f;
	}

	return divider;
}

inline u32 utils_time_diff_ms(const u32 start_us, const u32 end_us) {
	return (end_us - start_us) / 1000;
}

inline u32 utils_time_diff_us(const u32 start_us, const u32 end_us) {
	return end_us - start_us;
}

inline i32 utils_time_after_us(const u32 time, const u32 target_time) {
	return (i32)(target_time - time);
}

[[noreturn]] void utils_error_mode(const i32 code) {
	utils_internal_led(false);
	const i32 long_blink = code / 10;
	const i32 short_blink = code % 10;
	// ReSharper disable once CppDFAEndlessLoop
	for (;;) {
		utils_printf("!!! ERROR MODE: %ld\n", code);
		for (auto i = 0; i < long_blink; i++) {
			utils_internal_led(true);
			sleep_ms(500);
			utils_internal_led(false);
			sleep_ms(500);
		}
		sleep_ms(400);
		for (auto i = 0; i < short_blink; i++) {
			utils_internal_led(true);
			sleep_ms(75);
			utils_internal_led(false);
			sleep_ms(700);
		}
		sleep_ms(3'000); // sleep for 3 seconds
	}
}

void utils_internal_led(const bool on) {
	status_led_set_state(on);
}

i32 utils_proportional_reduce(const i32 number, i32 step, const i32 total_steps, const bool invert) {
	if (step >= total_steps) step = total_steps;
	if (number == 0) return 0;

	const auto result = (i32)((float)number / total_steps * step);
	if (invert) return number - result;
	return result;
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

void utils_crc_init() {
	if (crc_init) return;

	crc_tab = malloc(256 * sizeof *crc_tab);
	if (crc_tab == nullptr) {
		utils_printf("!!! Couldn't allocate memory for crc tab (free memory: %u kB)\n", memory_remaining_heap() / 1024);
		return;
	}

	for (u32 i = 0; i < 256; i++) {
		u32 c = i;
		for (int k = 0; k < 8; k++) c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
		crc_tab[i] = c;
	}

	crc_init = true;
}

u32 utils_crc(const void *data, const size_t len) {
	if (unlikely(!crc_init)) {
		utils_printf("!!! Call utils_crc_init first!\n");
		return 0;
	}

	const u8 *p = data;
	u32 c = 0xFFFFFFFFu;
	for (size_t i = 0; i < len; i++) c = crc_tab[(c ^ p[i]) & 0xFFu] ^ (c >> 8);
	return c ^ 0xFFFFFFFFu;
}

void utils_generate_id(char *dst, const size_t len) {
	static constexpr char SYMBOLS[] =
		"1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.~_-";
	static constexpr size_t SYMBOLS_LEN = sizeof SYMBOLS - 1;

	if (dst == nullptr || len == 0) return;

	for (size_t i = 0; i < len; ++i) {
		const auto idx = utils_random_in_range(0, SYMBOLS_LEN - 1);
		dst[i] = SYMBOLS[idx];
	}
	utils_printf("len=%zu, id=%.*s\n", len, (int)len, dst);
}

void utils_base64_encode(const u8 *input, const size_t len, char *output, const size_t out_cap) {
	static constexpr char symbols[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (unlikely(out_cap == 0)) return;
	const size_t need = ((len + 2) / 3) * 4 + 1;
	if (unlikely(out_cap < need)) return;

	size_t i = 0, o = 0;
	while (i + 2 < len) {
		const u32 v = ((u32)input[i] << 16) | ((u32)input[i + 1] << 8) | (u32)input[i + 2];
		output[o++] = symbols[(v >> 18) & 0x3F];
		output[o++] = symbols[(v >> 12) & 0x3F];
		output[o++] = symbols[(v >> 6) & 0x3F];
		output[o++] = symbols[v & 0x3F];
		i += 3;
	}

	if (i + 1 < len) {
		const u32 v = ((u32)input[i] << 16) | ((u32)input[i + 1] << 8);
		output[o++] = symbols[(v >> 18) & 0x3F];
		output[o++] = symbols[(v >> 12) & 0x3F];
		output[o++] = symbols[(v >> 6) & 0x3F];
		output[o++] = '=';
	} else if (i < len) {
		const u32 v = ((u32)input[i] << 16);
		output[o++] = symbols[(v >> 18) & 0x3F];
		output[o++] = symbols[(v >> 12) & 0x3F];
		output[o++] = '=';
		output[o++] = '=';
	}

	output[o] = '\0';
}
