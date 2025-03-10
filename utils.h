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

i32 utils_proportional_reduce(i32 number, i32 step, i32 total_steps);

i32 utils_scaled_pwm_percentage(i32 val, i32 deadzone, i32 max_val);

u16 *utils_pwm_cc_for_16bit(u8 slice, u8 channel);

void utils_print_time_elapsed(const char *title, u32 start_us);

#endif //UTILS_H
