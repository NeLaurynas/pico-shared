// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PICO_SHARED_FRTOS_H
#define PICO_SHARED_FRTOS_H

#include <stdbool.h>

/**
 * Returns CPU busy percentage since the previous sample.
 *
 * The first call primes the sample window and returns false. Non-FreeRTOS builds
 * also return false.
 */
bool frtos_cpu_usage_percent(float *out_percent);

void frtos_cpu_usage_reset();

#endif
