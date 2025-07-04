// Copyright (C) 2024 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <hardware/pwm.h>

#include "shared_config.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0])) // will not work for decayed arrays

#define DMA_IRQ(irq) (irq == 0 ? DMA_IRQ_0 : DMA_IRQ_1)

static inline u8 utils_min_u8(const u8 x, const u8 y) {
	return x < y ? x : y;
}

static inline u16 utils_min_u16(const u16 x, const u16 y) {
	return x < y ? x : y;
}

static inline u32 utils_min_u32(const u32 x, const u32 y) {
	return x < y ? x : y;
}

static inline i32 utils_min_i32(const i32 x, const i32 y) {
	return x < y ? x : y;
}

#define utils_min(x, y) \
_Generic((x), \
u8: utils_min_u8, \
u16: utils_min_u16, \
u32: utils_min_u32, \
i32: utils_min_i32 \
)(x, y)

static inline u8 utils_max_u8(const u8 x, const u8 y) {
	return x > y ? x : y;
}

static inline u16 utils_max_u16(const u16 x, const u16 y) {
	return x > y ? x : y;
}

static inline u32 utils_max_u32(const u32 x, const u32 y) {
	return x > y ? x : y;
}

static inline i32 utils_max_i32(const i32 x, const i32 y) {
	return x > y ? x : y;
}

#define utils_max(x, y) \
_Generic((x), \
u8:  utils_max_u8,\
u16: utils_max_u16, \
u32: utils_max_u32, \
i32: utils_max_i32 \
)(x, y)

static void utils_swap_u8(u8 *x, u8 *y) {
	const auto tmp = *x;
	*x = *y;
	*y = tmp;
}

static void utils_swap_u16(u16 *x, u16 *y) {
	const auto tmp = *x;
	*x = *y;
	*y = tmp;
}

static void utils_swap_u32(u32 *x, u32 *y) {
	const auto tmp = *x;
	*x = *y;
	*y = tmp;
}

static void utils_swap_i32(i32 *x, i32 *y) {
	const auto tmp = *x;
	*x = *y;
	*y = tmp;
}

#define utils_swap(x, y) \
_Generic((x), \
u8*: utils_swap_u8, \
u16*: utils_swap_u16, \
u32*: utils_swap_u32, \
i32*: utils_swap_i32 \
)(x, y)

#define UTILS_AVG_CHUNK_SIZE 10

typedef u32 (*array_elem_reader_t)(const void *, size_t);

static float utils_avg_base(const void *array, const size_t array_size, const array_elem_reader_t reader) {
	if (!array || array_size == 0 || UTILS_AVG_CHUNK_SIZE == 0) return 0.0f;

	const size_t full_chunks = array_size / UTILS_AVG_CHUNK_SIZE;
	const size_t remainder = array_size % UTILS_AVG_CHUNK_SIZE;

	float chunk_averages_sum = 0.0f;
	int chunk_count = 0;

	size_t idx = 0;
	for (auto i = 0; i < full_chunks; i++) {
		u32 chunk_sum = 0;
		for (size_t y = 0; y < UTILS_AVG_CHUNK_SIZE; y++) chunk_sum += reader(array, idx++);

		const float chunk_avg = (float)chunk_sum / (float)UTILS_AVG_CHUNK_SIZE;
		chunk_averages_sum += chunk_avg;
		chunk_count++;
	}

	if (remainder > 0) {
		u32 chunk_sum = 0;
		for (size_t i = 0; i < remainder; i++) chunk_sum += reader(array, idx++);

		const float chunk_avg = (float)chunk_sum / (float)remainder;
		chunk_averages_sum += chunk_avg;
		chunk_count++;
	}

	return chunk_averages_sum / (float)chunk_count;
}

static u32 reader_u8(const void *array, const size_t index) {
	return ((const u8*)array)[index];
}

static u32 reader_u16(const void *array, const size_t index) {
	return ((const u16*)array)[index];
}

static u32 reader_u32(const void *array, const size_t index) {
	return ((const u32*)array)[index];
}

static u32 reader_i32(const void *array, const size_t index) {
	return ((const i32*)array)[index];
}

static inline float utils_avg_u8(const u8 *array, const size_t array_size) {
	return utils_avg_base(array, array_size, reader_u8);
}

static inline float utils_avg_u16(const u16 *array, const size_t array_size) {
	return utils_avg_base(array, array_size, reader_u16);
}

static inline float utils_avg_u32(const u32 *array, const size_t array_size) {
	return utils_avg_base(array, array_size, reader_u32);
}

static inline float utils_avg_i32(const i32 *array, const size_t array_size) {
	return utils_avg_base(array, array_size, reader_i32);
}

#define utils_avg(array, array_size) \
 _Generic((array), \
u8*  : utils_avg_u8,  \
u16*: utils_avg_u16, \
u32*: utils_avg_u32, \
i32*: utils_avg_i32  \
)(array, array_size)

u32 utils_random_in_range(u32 fromInclusive, u32 toInclusive);

float utils_print_onboard_temp();

void utils_print_cpu_speed();

float utils_calculate_pio_clk_div(float instruction_execution_in_us);

float utils_calculate_pio_clk_div_ns(float instruction_execution_in_ns);

/**
 * Calculates PWM clock. 1 khz = every 1 ms, 2 khz = ever 0.5 ms, etc
 *
 * @param top PWM TOP configuration (wrap)
 * @param freq_khz Desired frequency (check debug messages if clock can be generated)
 */
float utils_calculate_pwm_divider(u32 top, float freq_khz);

#if defined(DBG) && DBG
#define utils_printf(...) printf(__VA_ARGS__)
#else
#define utils_printf(...) (void)0
#endif

u32 utils_time_diff_ms(u32 start_us, u32 end_us);

u32 utils_time_diff_us(u32 start_us, u32 end_us);

void utils_error_mode(i32 code);

void utils_internal_led(bool on);

i32 utils_proportional_reduce(i32 number, i32 step, i32 total_steps, bool invert);

i32 utils_scaled_pwm_percentage(i32 val, i32 deadzone, i32 max_val);

u16 *utils_pwm_cc_for_16bit(u8 slice, u8 channel);

void utils_print_time_elapsed(const char *title, u32 start_us);

#endif //UTILS_H
