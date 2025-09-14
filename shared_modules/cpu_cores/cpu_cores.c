// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "cpu_cores.h"

#include "shared_config.h"
#include "utils.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "pico/sleep.h"

static void (*shutdown_callback)(void) = nullptr;

static inline void pwm_off_all(void) {
	for (u16 s = 0; s < NUM_PWM_SLICES; s++) pwm_set_enabled(s, false);
#ifdef RESETS_RESET_PWM_BITS
	reset_block(RESETS_RESET_PWM_BITS);
#endif
}

static inline void pio_off_all(void) {
#if NUM_PIOS >= 1
	pio_set_sm_mask_enabled(pio0, 0xF, false);
	pio_clear_instruction_memory(pio0);
#endif
#if NUM_PIOS >= 2
	pio_set_sm_mask_enabled(pio1, 0xF, false);
	pio_clear_instruction_memory(pio1);
#endif
#if NUM_PIOS >= 3
	pio_set_sm_mask_enabled(pio2, 0xF, false);
	pio_clear_instruction_memory(pio2);
#endif
#if NUM_PIOS >= 4
	pio_set_sm_mask_enabled(pio3, 0xF, false);
	pio_clear_instruction_memory(pio3);
#endif

#ifdef RESETS_RESET_PIO0_BITS
	reset_block(RESETS_RESET_PIO0_BITS);
#endif
#ifdef RESETS_RESET_PIO1_BITS
	reset_block(RESETS_RESET_PIO1_BITS);
#endif
#ifdef RESETS_RESET_PIO2_BITS
	reset_block(RESETS_RESET_PIO2_BITS);
#endif
#ifdef RESETS_RESET_PIO3_BITS
	reset_block(RESETS_RESET_PIO3_BITS);
#endif
}

static inline void dma_off_all(void) {
	for (u16 ch = 0; ch < NUM_DMA_CHANNELS; ch++) {
		dma_channel_abort(ch);
	}
#ifdef DMA_IRQ_0
	irq_set_enabled(DMA_IRQ_0, false);
#endif
#ifdef DMA_IRQ_1
	irq_set_enabled(DMA_IRQ_1, false);
#endif
	if (dma_hw) {
		dma_hw->inte0 = 0;
		dma_hw->inte1 = 0;
	}

#ifdef RESETS_RESET_DMA_BITS
	reset_block(RESETS_RESET_DMA_BITS); // gate DMA engine
#endif
}

static inline void adc_off_all(void) {
	adc_set_temp_sensor_enabled(false);
	adc_run(false);
#ifdef ADC_IRQ_FIFO
	irq_set_enabled(ADC_IRQ_FIFO, false);
#endif
#ifdef RESETS_RESET_ADC_BITS
	reset_block(RESETS_RESET_ADC_BITS);
#endif
}

static inline void i2c_off_all(void) {
#ifdef i2c0
	i2c_deinit(i2c0);
#endif
#ifdef i2c1
	i2c_deinit(i2c1);
#endif
#ifdef i2c2
	i2c_deinit(i2c2);
#endif
#ifdef i2c3
	i2c_deinit(i2c3);
#endif
#ifdef RESETS_RESET_I2C0_BITS
	reset_block(RESETS_RESET_I2C0_BITS);
#endif
#ifdef RESETS_RESET_I2C1_BITS
	reset_block(RESETS_RESET_I2C1_BITS);
#endif
#ifdef RESETS_RESET_I2C2_BITS
	reset_block(RESETS_RESET_I2C2_BITS);
#endif
#ifdef RESETS_RESET_I2C3_BITS
	reset_block(RESETS_RESET_I2C3_BITS);
#endif
}

static inline void uart_off_all(void) {
#ifdef uart0
	uart_deinit(uart0);
#endif
#ifdef uart1
	uart_deinit(uart1);
#endif
#ifdef uart2
	uart_deinit(uart2);
#endif
#ifdef uart3
	uart_deinit(uart3);
#endif
#ifdef RESETS_RESET_UART0_BITS
	reset_block(RESETS_RESET_UART0_BITS);
#endif
#ifdef RESETS_RESET_UART1_BITS
	reset_block(RESETS_RESET_UART1_BITS);
#endif
#ifdef RESETS_RESET_UART2_BITS
	reset_block(RESETS_RESET_UART2_BITS);
#endif
#ifdef RESETS_RESET_UART3_BITS
	reset_block(RESETS_RESET_UART3_BITS);
#endif
}

static void sio_fifo_irq_handler(void) {
	while (multicore_fifo_rvalid()) {
		const u32 cmd = multicore_fifo_pop_blocking();
		if (cmd == CPU_CORES_CMD_SHUTDOWN) {
			if (shutdown_callback != NULL) shutdown_callback();
			else
				utils_printf("!!! SHUTDOWN_CALLBACK WAS NULL");
		}
	}

	multicore_fifo_clear_irq();
}

void cpu_cores_init_from_core0(void (*callback)(void)) {
	shutdown_callback = callback;

	multicore_fifo_drain();
	multicore_fifo_clear_irq();

	irq_set_exclusive_handler(SIO_FIFO_IRQ_NUM(0), sio_fifo_irq_handler);
	irq_set_enabled(SIO_FIFO_IRQ_NUM(0), true);
}

[[noreturn]]
void cpu_cores_send_shutdown_to_core0_from_core1(void) {
	utils_printf("set shutdown flag for core0 from core 1\n");
	multicore_fifo_push_blocking(CPU_CORES_CMD_SHUTDOWN);

	utils_printf("core1 entering loop to prevent instruction execution; core0 will later shut it down\n");
	for (;;) tight_loop_contents();
}

void cpu_cores_shutdown_from_core0() {
	// utils_printf("wifi and bt shutdown\n");
	// hci_power_control(HCI_POWER_OFF);
	// cyw43_arch_deinit();

	utils_printf("core1 shutdown\n");
	multicore_reset_core1();

	utils_printf("pwm shutdown\n");
	pwm_off_all();
	utils_printf("pio shutdown\n");
	pio_off_all();
	utils_printf("dma shutdown\n");
	dma_off_all();
	utils_printf("adc shutdown\n");
	adc_off_all();
	utils_printf("i2c shutdown\n");
	i2c_off_all();
	utils_printf("uart shutdown\n");
	uart_off_all();

	utils_printf("gpio shutdown\n");
	for (u16 i = 0; i < 30; ++i) {
		gpio_set_function(i, GPIO_FUNC_NULL);
		gpio_disable_pulls(i);
		gpio_set_dir(i, false);
	}

	utils_printf("going to sleep\n");
	sleep_goto_dormant_until_edge_high(1);
}
