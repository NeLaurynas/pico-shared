// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WSLEDS_H
#define WSLEDS_H
#include "shared_config.h"

extern u32 wsleds_buffer[MOD_WSLEDS_LED_COUNT];

void wsleds_init();

void wsleds_buffer_transfer();

void wsleds_rotate_buffer_left(u8 times);

#endif //WSLEDS_H
