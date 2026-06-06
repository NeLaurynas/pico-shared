// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <hardware/i2c.h>

#include "../../shared_config.h"

#ifndef MOD_MCP_PIN_SDA
#define MOD_MCP_PIN_SDA             16
#endif

#ifndef MOD_MCP_PIN_SCL
#define MOD_MCP_PIN_SCL             17
#endif

#ifndef MOD_MCP_I2C_PORT
#define MOD_MCP_I2C_PORT            (&i2c0_inst)
#endif

#ifndef MOD_MCP_ADDR1
#define MOD_MCP_ADDR1               0b00100000
#endif

#ifndef MOD_MCP_ADDR2
#define MOD_MCP_ADDR2               0b00100001
#endif

#ifndef MOD_MCP_GPIO_CACHE_MS
#define MOD_MCP_GPIO_CACHE_MS       6
#endif

#ifndef MOD_MCP_WRITE_RETRY_COUNT
#define MOD_MCP_WRITE_RETRY_COUNT   2
#endif
