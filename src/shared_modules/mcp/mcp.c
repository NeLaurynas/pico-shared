// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "mcp.h"

#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include "shared_config.h"
#include "utils.h"

// MCP23017 registers (Bank Mode 1)
#define C_IODIRA	0x00 // I/O Direction Register A
#define C_IODIRB	0x01 // I/O Direction Register B
#define C_GPIOA		0x12 // GPIO Register A
#define C_GPIOB		0x13 // GPIO Register B
#define C_GPPUA		0x0C // PULL UP A
#define C_GPPUB		0x0D // PULL UP B

#define C_IOCONA			0x0A  //IO Configuration Register A - BANK/MIRROR/SLEW/INTPOL
#define C_IOCONB			0x0B  //IO Configuration Register B - BANK/MIRROR/SLEW/INTPOL
#define C_IOCON_BANK_BIT	7
#define C_IOCON_MIRROR_BIT	6
#define C_IOCON_SEQOP_BIT	5
#define C_IOCON_DISSLW_BIT	4
#define C_IOCON_HAEN_BIT	3
#define C_IOCON_ODR_BIT		2
#define C_IOCON_INTPOL_BIT	1

// cache variables
static u8 cache_mcp1_gpioa = 0;
static u8 cache_mcp1_gpiob = 0;
static u8 cache_mcp2_gpioa = 0;
static u8 cache_mcp2_gpiob = 0;
static u32 cache_last_mcp1_gpio = 0;
static u32 cache_last_mcp2_gpio = 0;

// TODO: hardcoded for TWO MCPs, refactor hard
void write_register(const u8 address, const u8 regist, const u8 value) {
	const u8 data[2] = { regist, value };
	auto const result = i2c_write_blocking(MOD_MCP_I2C_PORT, address, data, 2, false);
	if (result == PICO_ERROR_GENERIC) utils_error_mode(address == MOD_MCP_ADDR1 ? 11 : 12); // mode(11) (mode12)
}

u8 read_register(const u8 address, const u8 regist) {
	u8 value;
	auto result = i2c_write_blocking(MOD_MCP_I2C_PORT, address, &regist, 1, true);
	if (result == PICO_ERROR_GENERIC) utils_error_mode(address == MOD_MCP_ADDR1 ? 13 : 14); // mode(13) (mode14)
	result = i2c_read_blocking(MOD_MCP_I2C_PORT, address, &value, 1, false);
	if (result == PICO_ERROR_GENERIC) utils_error_mode(address == MOD_MCP_ADDR1 ? 15 : 16); // mode(15) mode(16)
	return value;
}

u16 read_dual_registers(const u8 address, const u8 regist) {
	u8 value[2] = { 0 };
	auto result = i2c_write_blocking(MOD_MCP_I2C_PORT, address, &regist, 1, true);
	if (result == PICO_ERROR_GENERIC) utils_error_mode(address == MOD_MCP_ADDR1 ? 17 : 18); // mode(17) (mode18)
	result = i2c_read_blocking(MOD_MCP_I2C_PORT, address, value, 2, false);
	if (result == PICO_ERROR_GENERIC) utils_error_mode(address == MOD_MCP_ADDR1 ? 19 : 20); // mode(19) (mode20)
	return (value[1] << 8) | value[0];
}

inline bool is_bit_set(const u8 value, const u8 bit) {
	return 0b1 & (value >> bit);
}

inline u8 cfg_address(const u8 data) {
	return is_bit_set(data, 7) ? MOD_MCP_ADDR2 : MOD_MCP_ADDR1;
}

inline u8 cfg_gpio_bank(const u8 data) {
	return is_bit_set(data, 6) ? C_GPIOB : C_GPIOA;
}

inline u8 cfg_iodir_bank(const u8 data) {
	return is_bit_set(data, 6) ? C_IODIRB : C_IODIRA;
}

inline u8 cfg_gppu_bank(const u8 data) {
	return is_bit_set(data, 6) ? C_GPPUB : C_GPPUA;
}

inline void set_bit(u8 *value, const u8 bit, const bool set) {
	if (set) {
		*value |= (1 << bit);
	} else {
		*value &= ~(1 << bit);
	}
}

inline u8 cfg_get_number(const u8 data) {
	return data & 0b00111111; // last 6 bits
}

void mcp_cfg_set_pin_out_mode(const u8 data, const bool is_out) {
	const auto address = cfg_address(data);
	const auto bank = cfg_iodir_bank(data);
	auto options = read_register(address, bank);
	set_bit(&options, cfg_get_number(data), !is_out);
	write_register(address, bank, options);
}

void mcp_cfg_set_pull_up(u8 pinData, bool pull_up) {
	const auto address = cfg_address(pinData);
	const auto bank = cfg_gppu_bank(pinData);
	auto options = read_register(address, bank);
	set_bit(&options, cfg_get_number(pinData), pull_up);
	write_register(address, bank, options);
}

void setup_bank_configuration(const u8 address, const u8 regist) {
	u8 iocon_data = 0;
	set_bit(&iocon_data, C_IOCON_BANK_BIT, false); // set to Bank Mode 0
	set_bit(&iocon_data, C_IOCON_MIRROR_BIT, false);
	set_bit(&iocon_data, C_IOCON_SEQOP_BIT, false);
	set_bit(&iocon_data, C_IOCON_DISSLW_BIT, false);
	set_bit(&iocon_data, C_IOCON_HAEN_BIT, false);
	set_bit(&iocon_data, C_IOCON_ODR_BIT, false);
	set_bit(&iocon_data, C_IOCON_INTPOL_BIT, false);
	write_register(address, regist, iocon_data);
}

void mcp_init() {
	const auto rate = i2c_init(MOD_MCP_I2C_PORT, 400'000); // 400 khz
	if (rate != 400'000) utils_error_mode(10);
	gpio_set_function(MOD_MCP_PIN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(MOD_MCP_PIN_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(MOD_MCP_PIN_SDA);
	gpio_pull_up(MOD_MCP_PIN_SCL);
	sleep_ms(1);

	setup_bank_configuration(MOD_MCP_ADDR1, C_IOCONA);
	setup_bank_configuration(MOD_MCP_ADDR1, C_IOCONB);
	setup_bank_configuration(MOD_MCP_ADDR2, C_IOCONA);
	setup_bank_configuration(MOD_MCP_ADDR2, C_IOCONB);
	sleep_ms(1);
}

// TODO: cache like is_pin_low and then don't forget to flush? // takes 0.2-0.3 ms per call at 12mhz....
void mcp_set_out(const u8 pinData, const bool out) {
	const auto address = cfg_address(pinData);
	const auto bank = cfg_gpio_bank(pinData);
	auto data = read_register(address, bank);
	set_bit(&data, cfg_get_number(pinData), out);
	write_register(address, bank, data);
}

bool mcp_is_pin_low(const u8 pinData) {
	const auto address = cfg_address(pinData);
	const auto bank = cfg_gpio_bank(pinData);
	u8 data;
	const bool first_mcp = address == MOD_MCP_ADDR1;
	u32 *cache_time = first_mcp ? &cache_last_mcp1_gpio : &cache_last_mcp2_gpio;
	const bool cache_possible = utils_time_diff_ms(*cache_time, time_us_32()) < MOD_MCP_GPIO_CACHE_MS;
	const bool first_bank = bank == C_GPIOA;

	if (first_mcp && cache_possible) {
		data = (first_bank ? cache_mcp1_gpioa : cache_mcp1_gpiob);
	} else if (!first_mcp && cache_possible) {
		data = (first_bank ? cache_mcp2_gpioa : cache_mcp2_gpiob);
	} else {
		const auto new_data = read_dual_registers(address, C_GPIOA); // read both A and B registers
		const u8 bank_a = new_data & 0b11111111;
		const u8 bank_b = (new_data >> 8) & 0b11111111;
		u8 *cache_a = first_mcp ? &cache_mcp1_gpioa : &cache_mcp2_gpioa;
		u8 *cache_b = first_mcp ? &cache_mcp1_gpiob : &cache_mcp2_gpiob;

		*cache_a = bank_a;
		*cache_b = bank_b;
		*cache_time = time_us_32();

		data = first_bank ? bank_a : bank_b;
	}
	return !is_bit_set(data, cfg_get_number(pinData));
}
