// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "memory.h"

#include <malloc.h>
#include <stdlib.h>
#include <utils.h>

extern char __StackLimit, __bss_end__;

static inline size_t total_heap() {
	return (size_t)(&__StackLimit - &__bss_end__);
}

static inline size_t used_heap() {
	const struct mallinfo mi = mallinfo();
	return (size_t)mi.uordblks;
}

static inline size_t free_heap() {
	return total_heap() - used_heap();
}

size_t memory_remaining_heap() {
	size_t lo = 0, hi = free_heap();

	while (lo < hi) {
		const size_t mid = (lo + hi + 1) / 2;
		void *p = malloc(mid);
		if (p) {
			free(p);
			lo = mid;
		} else hi = mid - 1;
	}

	utils_printf("Free memory: %u kB\n", lo / 1024);

	return lo;
}
