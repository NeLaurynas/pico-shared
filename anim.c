// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "anim.h"

#include <math.h>
#include <stdlib.h>

#include "utils.h"

static void adjust_frame_by_speed_freq(const u16 frame, const u16 frame_count, const float speed,
								const float freq, u16 *divisor, float *adjusted_frame) {
	*divisor = frame_count / freq;
	*adjusted_frame = fmod(frame, *divisor) * speed;
}

u8 anim_color_reduction(const anim_direction_t direction, const u16 frame, const u16 frame_count, const float speed,
						const float freq) {

	float adjusted_frame;
	u16 divisor;
	adjust_frame_by_speed_freq(frame, frame_count, speed, freq, &divisor, &adjusted_frame);

	u8 reduction = utils_proportional_reduce(255, adjusted_frame, divisor, false);

	if (direction == TO_BRIGHT) reduction = 255 - reduction;
	// TODO: PULSE

	return reduction;
}

u32 anim_color_blend(const u32 color_from, const u32 color_to, const u16 frame, const u16 frame_count, const float speed,
					 const float freq) {
	float adjusted_frame;
	u16 divisor;
	adjust_frame_by_speed_freq(frame, frame_count, speed, freq, &divisor, &adjusted_frame);

	const u8 r1 = (color_from >> 16) & 0b11111111;
	const u8 g1 = (color_from >> 8) & 0b11111111;
	const u8 b1 = color_from & 0b11111111;
	const u8 r2 = (color_to >> 16) & 0b11111111;
	const u8 g2 = (color_to >> 8) & 0b11111111;
	const u8 b2 = color_to & 0b11111111;

	u8 r_diff = r1 < r2 ? r2 - r1 : r1 - r2;
	u8 g_diff = g1 < g2 ? g2 - g1 : g1 - g2;
	u8 b_diff = b1 < b2 ? b2 - b1 : b1 - b2;

	r_diff = utils_min(utils_proportional_reduce(r_diff, adjusted_frame, divisor, false), 255);
	g_diff = utils_min(utils_proportional_reduce(g_diff, adjusted_frame, divisor, false), 255);
	b_diff = utils_min(utils_proportional_reduce(b_diff, adjusted_frame, divisor, false), 255);

	return (r1 > r2 ? r1 - r_diff : r1 + r_diff) << 16 |
		(g1 > g2 ? g1 - g_diff : g1 + g_diff) << 8 |
		(b1 > b2 ? b1 - b_diff : b1 + b_diff);
}
