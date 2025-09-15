// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "str.h"

#include <string.h>

void str_set(char dest[], const size_t cap, const char *src) {
	const size_t n = strnlen(src, cap ? cap - 1 : 0);
	memcpy(dest, src, n);
	memset(dest + n, 0, cap - n);
}
