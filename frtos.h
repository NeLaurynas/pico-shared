// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stddef.h>

/**
 * Returns per-core CPU busy percentages since the previous sample.
 *
 * Fills up to the configured core count and clears extra entries. The first
 * call primes the sample window and returns false. Non-FreeRTOS builds and
 * calls before the scheduler starts also return false.
 */
bool frtos_cpu_usage_percent_per_core(float *out_percent, size_t core_count);

/**
 * Returns the average CPU busy percentage since the previous sample.
 *
 * This shares the sample window with frtos_cpu_usage_percent_per_core; a caller
 * should use one function or the other for each sampling stream.
 */
bool frtos_cpu_usage_percent(float *out_percent);

void frtos_cpu_usage_reset();
