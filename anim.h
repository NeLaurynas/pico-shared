// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ANIM_H
#define ANIM_H

#include "shared_config.h"

typedef enum {
	TO_BRIGHT, TO_DIM, PULSE
} anim_direction_t;

/**
 * Reduces color brightness (returns color reduction number)
 *
 * @param speed  How fast animation will go to an end (will hold)
 * @param freq How many times in frame_ticks it will get repeated
 */
u8 anim_color_reduction(const anim_direction_t direction, const u16 frame, const u16 frame_count, const float speed, const float freq);
// TODO: maybe return color (like below, because it's always used with reduce_brightness from wsleds.c)

/**
 * Switches from color to color (returns blended color
 *
 * @param speed  How fast animation will go to an end (will hold)
 * @param freq How many times in frame_ticks it will get repeated
 */
u32 anim_color_blend(const u32 color_from, const u32 color_to, const u16 frame, const u16 frame_count, const float speed, const float freq);

u32 anim_reduce_brightness(const u32 reduction, const u32 color);

#endif //ANIM_H
