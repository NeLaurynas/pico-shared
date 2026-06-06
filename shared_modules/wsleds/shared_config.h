// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <hardware/pio.h>

#include "../../shared_config.h"

#ifndef MOD_WSLEDS_LED_COUNT
#define MOD_WSLEDS_LED_COUNT        64
#endif

#ifndef MOD_WSLEDS_PIO
#define MOD_WSLEDS_PIO              pio0
#endif

#ifndef MOD_WSLEDS_SM
#define MOD_WSLEDS_SM               1
#endif

#ifndef MOD_WSLEDS_DMA_CH
#define MOD_WSLEDS_DMA_CH           1
#endif

#ifndef MOD_WSLEDS_PIN
#define MOD_WSLEDS_PIN              18
#endif
