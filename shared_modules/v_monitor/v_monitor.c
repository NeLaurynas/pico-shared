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

void v_monitor_anim() {
	static constexpr i32 TICKS = 10; // every 100 or 200 ms
	static i32 frame = 0;
	static i32 idx = 0;

	if (frame == 0) {
		samples[idx] = adc_read();

		idx = (idx + 1) % SAMPLE_COUNT;
		if (sample_count < SAMPLE_COUNT) sample_count++;
	}

	frame = (frame + 1) % TICKS;
}

float v_monitor_voltage(const bool select_input) {
	if (select_input) adc_select_input(MOD_VMON_ADC);

	if (sample_count < SAMPLE_COUNT) return MOD_VMON_DEFAULT_REF;

	float const raw = utils_avg(samples, sample_count);
	float const v_out = raw * ADC_FACTOR;
	float const v_in = v_out * (MOD_VMON_RES_POS + MOD_VMON_RES_NEG) / MOD_VMON_RES_NEG;

	utils_printf("battery: %2.2f V (sample count: %d)\n", v_in, sample_count);

	return v_in;
}
