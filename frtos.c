// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "frtos.h"

#if PICO_SHARED_HAS_FREERTOS

#include <FreeRTOS.h>
#include <pico/time.h>
#include <task.h>

static configRUN_TIME_COUNTER_TYPE previous_total_time = 0;
static configRUN_TIME_COUNTER_TYPE previous_idle_time = 0;
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
	previous_total_time = 0;
	previous_idle_time = 0;
	has_previous_sample = false;
}

bool frtos_cpu_usage_percent(float *out_percent) {
	if (out_percent == nullptr) return false;

	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
		*out_percent = 0.0f;
		frtos_cpu_usage_reset();
		return false;
	}

	const configRUN_TIME_COUNTER_TYPE total_time = (configRUN_TIME_COUNTER_TYPE)portGET_RUN_TIME_COUNTER_VALUE();
	const configRUN_TIME_COUNTER_TYPE idle_time = ulTaskGetIdleRunTimeCounter();

	if (!has_previous_sample) {
		previous_total_time = total_time;
		previous_idle_time = idle_time;
		has_previous_sample = true;
		*out_percent = 0.0f;
		return false;
	}

	const configRUN_TIME_COUNTER_TYPE total_delta = total_time - previous_total_time;
	const configRUN_TIME_COUNTER_TYPE idle_delta = idle_time - previous_idle_time;

	previous_total_time = total_time;
	previous_idle_time = idle_time;

	if (total_delta == 0) {
		*out_percent = 0.0f;
		return false;
	}

	const float idle_percent = clamp_percent(((float)idle_delta * 100.0f) / (float)total_delta);
	*out_percent = clamp_percent(100.0f - idle_percent);
	return true;
}

#else

void frtos_cpu_usage_reset() {
}

bool frtos_cpu_usage_percent(float *out_percent) {
	if (out_percent != nullptr) *out_percent = 0.0f;
	return false;
}

#endif
