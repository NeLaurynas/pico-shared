// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "cpu_cores.h"

#include <hardware/adc.h>
#include <hardware/clocks.h>
#include <pico/mutex.h>

#if defined(RASPBERRYPI_PICO2_W) && CYW43_PIO_CLOCK_DIV_DYNAMIC
#include <pico/cyw43_driver.h>
#endif

#include "shared_config.h"
#include "utils.h"

static bool inited = false;
auto_init_mutex(cpu_adc_mutex);

#if defined(RASPBERRYPI_PICO2_W) && !CYW43_PIO_CLOCK_DIV_DYNAMIC
static_assert(false, "Pico 2 W clock setup requires CYW43_PIO_CLOCK_DIV_DYNAMIC=1");
#endif

#if defined(RASPBERRYPI_PICO2_W) && CYW43_PIO_CLOCK_DIV_DYNAMIC
#define CPU_CYW43_TARGET_PIO_CLK_KHZ 75'000u
#define CPU_CYW43_PIO_CLKDIV_FRAC_SCALE 0b1'0000'0000u
#define CPU_CYW43_PIO_CLKDIV_FRAC_MASK  0b1111'1111u
#endif

#if defined(RASPBERRYPI_PICO2_W) && CYW43_PIO_CLOCK_DIV_DYNAMIC
static void set_cyw43_pio_clock_divisor(const u32 freq_khz) {
	// 150 MHz clk_sys with the SDK default divider of 2 gives a 75 MHz PIO clock.
	const auto raw_div_x256 =
			(((u64)freq_khz * CPU_CYW43_PIO_CLKDIV_FRAC_SCALE) + (CPU_CYW43_TARGET_PIO_CLK_KHZ / 2u)) /
			CPU_CYW43_TARGET_PIO_CLK_KHZ;
	auto div_x256 = raw_div_x256;
	if (div_x256 < CPU_CYW43_PIO_CLKDIV_FRAC_SCALE) {
		div_x256 = CPU_CYW43_PIO_CLKDIV_FRAC_SCALE;
	}

	const auto div_int = (u16)(div_x256 / CPU_CYW43_PIO_CLKDIV_FRAC_SCALE);
	const auto div_frac8 = (u8)(div_x256 & CPU_CYW43_PIO_CLKDIV_FRAC_MASK);

	cyw43_set_pio_clock_divisor(div_int, div_frac8);
}
#endif

bool cpu_set_clock_khz(const u32 freq_khz, const bool required) {
	const bool result = set_sys_clock_khz(freq_khz, required);

#if defined(RASPBERRYPI_PICO2_W) && CYW43_PIO_CLOCK_DIV_DYNAMIC
	if (result) set_cyw43_pio_clock_divisor(freq_khz);
#endif

	return result;
}

void cpu_init() {
	adc_init();
	adc_set_temp_sensor_enabled(true);
	inited = true;
}

// ADC input selection and conversion are shared with ADC controls, so serialize them.
u16 cpu_adc_read(const u8 channel) {
	mutex_enter_blocking(&cpu_adc_mutex);
	adc_select_input(channel);
	const u16 value = adc_read();
	mutex_exit(&cpu_adc_mutex);
	return value;
}

float cpu_temp(const bool print_result) {
	if (unlikely(!inited)) {
		if (print_result) utils_printf("cpu_temp - call cpu_init first!");
		return -1;
	}
	constexpr float conversionFactor = 3.3f / (1 << 12);

	const float adc = (float)cpu_adc_read(4) * conversionFactor;
	const float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

	if (print_result) utils_printf("Onboard temperature = %.02f C\n", tempC);

	return tempC;
}

float cpu_speed(const bool print_result) {
	const auto freq_hz = clock_get_hz(clk_sys);

	const float freq_mhz = (float)freq_hz / 1'000'000.0f;
	if (print_result) utils_printf("System clock: %.2f MHz\n", freq_mhz);

	return freq_mhz;
}
