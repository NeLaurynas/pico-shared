// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "v_monitor.h"

#include <hardware/adc.h>

#include "shared_config.h"
#include "utils.h"

#define SAMPLE_COUNT 25
#define ADC_FACTOR (MOD_VMON_VREF / (1 << 12))

static i32 sample_count = 0;
static u32 samples[SAMPLE_COUNT] = { };

void v_monitor_init() {
	adc_init();
	adc_gpio_init(MOD_VMON_ADC_PIN);
	adc_select_input(MOD_VMON_ADC);
}

void v_monitor_sample(const bool select_input) {
	static i32 idx = 0;

	if (select_input) adc_select_input(MOD_VMON_ADC);
	samples[idx] = adc_read();

	idx = (idx + 1) % SAMPLE_COUNT;
	if (unlikely(sample_count < SAMPLE_COUNT)) sample_count++;
}

float v_monitor_voltage(const bool print_result) {
	if (unlikely(sample_count == 0)) return MOD_VMON_DEFAULT_REF;

	float const raw = utils_avg(samples, sample_count);
	float const v_out = raw * ADC_FACTOR;
	float const v_in = v_out * (MOD_VMON_RES_POS + MOD_VMON_RES_NEG) / MOD_VMON_RES_NEG;

	if (print_result) utils_printf("bat: %2.3f V (samples: %d)\n", v_in, sample_count);

	return v_in;
}
