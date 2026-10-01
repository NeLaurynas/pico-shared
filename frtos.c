// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "frtos.h"

#if PICO_SHARED_HAS_FREERTOS

#include <FreeRTOS.h>
#include <pico/time.h>
#include <task.h>

static configRUN_TIME_COUNTER_TYPE previous_total_time = 0;
static configRUN_TIME_COUNTER_TYPE previous_idle_time[configNUMBER_OF_CORES] = {0};
static bool has_previous_sample = false;

uint32_t frtos_runtime_counter() {
	return time_us_32();
}

static float clamp_percent(const float value) {
	if (value < 0.0f) return 0.0f;
	if (value > 100.0f) return 100.0f;
	return value;
}

void frtos_cpu_usage_reset() {
	has_previous_sample = false;
}

static configRUN_TIME_COUNTER_TYPE idle_runtime_counter(const size_t core) {
#if configNUMBER_OF_CORES == 1
	(void)core;
	return ulTaskGetRunTimeCounter(xTaskGetIdleTaskHandle());
#else
	return ulTaskGetRunTimeCounter(xTaskGetIdleTaskHandleForCore(core));
#endif
}

bool frtos_cpu_usage_percent_per_core(float *out_percent, const size_t core_count) {
	if (out_percent == nullptr) return false;
	for (size_t i = 0; i < core_count; i++) out_percent[i] = 0.0f;

	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
		frtos_cpu_usage_reset();
		return false;
	}

	const configRUN_TIME_COUNTER_TYPE total_time = (configRUN_TIME_COUNTER_TYPE)portGET_RUN_TIME_COUNTER_VALUE();
	const configRUN_TIME_COUNTER_TYPE total_delta = total_time - previous_total_time;
	// Every call advances the sample window; the first one after a reset only primes it.
	const bool valid = has_previous_sample && total_delta != 0;
	previous_total_time = total_time;
	has_previous_sample = true;

	for (size_t i = 0; i < configNUMBER_OF_CORES; i++) {
		const auto idle_time = idle_runtime_counter(i);
		const configRUN_TIME_COUNTER_TYPE idle_delta = idle_time - previous_idle_time[i];
		previous_idle_time[i] = idle_time;
		if (i >= core_count || !valid) continue;

		const float idle_percent = clamp_percent(((float)idle_delta * 100.0f) / (float)total_delta);
		out_percent[i] = 100.0f - idle_percent;
	}

	return valid;
}

bool frtos_cpu_usage_percent(float *out_percent) {
	if (out_percent == nullptr) return false;

	float core_percent[configNUMBER_OF_CORES];
	if (!frtos_cpu_usage_percent_per_core(core_percent, configNUMBER_OF_CORES)) {
		*out_percent = 0.0f;
		return false;
	}

	float total_percent = 0.0f;
	for (size_t i = 0; i < configNUMBER_OF_CORES; i++) total_percent += core_percent[i];
	*out_percent = total_percent / (float)configNUMBER_OF_CORES;
	return true;
}

#else

void frtos_cpu_usage_reset() {
}

bool frtos_cpu_usage_percent_per_core(float *out_percent, const size_t core_count) {
	if (out_percent != nullptr) {
		for (size_t i = 0; i < core_count; i++) out_percent[i] = 0.0f;
	}
	return false;
}

bool frtos_cpu_usage_percent(float *out_percent) {
	if (out_percent != nullptr) *out_percent = 0.0f;
	return false;
}

#endif
