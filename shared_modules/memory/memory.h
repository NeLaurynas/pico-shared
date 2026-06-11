// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <stddef.h>

/**
 * @brief Real-ish free heap
 * @warning Gets info by allocating \b ALL the heap, so if you want to allocate stuff while this is running - \e gg
 * @return Size in bytes
 */
size_t memory_remaining_heap(bool print_result);
