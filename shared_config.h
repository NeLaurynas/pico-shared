// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stdint.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef int64_t i64;
typedef uint64_t u64;

#define US_IN_MS      1000ULL
#define US_IN_SECOND  (1000ULL * US_IN_MS)
#define US_IN_MINUTE  (60ULL * US_IN_SECOND)
#define US_IN_HOUR    (60ULL * US_IN_MINUTE)

#ifndef DBG
#define DBG 1
#endif
