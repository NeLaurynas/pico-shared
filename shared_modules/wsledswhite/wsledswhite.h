// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef WSLEDSWHITE_H
#define WSLEDSWHITE_H
#include "shared_config.h"

extern u32 wsledswhite_buffer[MOD_WSLEDS_LED_COUNT];

void wsledswhite_init();

void wsledswhite_buffer_transfer();

void wsledswhite_rotate_buffer_left(u8 times);

#endif //WSLEDS_H
