#ifndef SHARED_CONFIG_H
#define SHARED_CONFIG_H

#include <pico/types.h>

// TYPEDEFS
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

#define DBG true

// MCP
/* MCP Layout (one byte) for controlled pins:
 *   bit 7: MCP 1 or MCP 2
 *   bit 6: Bank A or Bank B
 *   rest: 6-bit number (max val 63)
 *    7   6   5   4   3   2   1   0
 *  | M | B |       number          |
 *  ie: #define MOD_NUM_LEDG (u8)((0 << 7) | (1 << 6) | 1) // MCP - first bit - MCP 1, second bit - Bank B - pin 1
 */
#define MOD_MCP_PIN_SDA		16
#define MOD_MCP_PIN_SCL		17
#define MOD_MCP_I2C_PORT	(&i2c0_inst)
#define MOD_MCP_ADDR1		0x20
#define MOD_MCP_ADDR2		0x21
#define MOD_MCP_GPIO_CACHE_MS 6

// MISC
#define INTERNAL_LED 25

#endif //SHARED_CONFIG_H
