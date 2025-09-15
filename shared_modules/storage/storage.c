// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "storage.h"

#include <string.h>

#include "utils.h"
#include "hardware/flash.h"
#include "pico/multicore.h"

typedef struct {
	bool has_records;
	u32 latest_version;
	u32 latest_offset;
} storage_state_t;

static storage_state_t storage_state = { };

#define STORAGE_BASE_XIP   ((uintptr_t)(XIP_BASE + (u32)MOD_STORAGE_OFFSET))

static inline const u8 *absolute_flash_location(const u32 offset) {
	return (const u8*)(STORAGE_BASE_XIP + (uintptr_t)offset);
}

static inline u32 sector_start(const u32 sector_index) {
	return sector_index * MOD_STORAGE_SECTOR_SIZE;
}

static inline u32 page_advance(const u32 offset) {
	const u32 n = offset + MOD_STORAGE_PAGE_SIZE;
	return (n >= MOD_STORAGE_BYTES) ? 0u : n;
}

static bool page_is_erased(const u8 *flash_location) {
	for (u32 i = 0; i < 5; i++)
		if (flash_location[i] != 0xFF) {
			utils_printf("page_is_erased: %p: false\n", (const void*)flash_location);
			return false; // yea check only first page locations
		}
	utils_printf("page_is_erased: %p: true\n", (const void*)flash_location);
	return true;
}

static bool record_valid(const settings_record_t *record) {
	settings_record_t tmp = *record; // copy
	tmp.crc32 = 0;
	const u32 c = utils_crc(&tmp, sizeof tmp);

	const bool result = record->crc32 == c;
	utils_printf("record valid: %s\n", result ? "true" : "false");
	return result;
}

static void full_rescan() {
	storage_state.has_records = false;
	storage_state.latest_offset = 0;
	storage_state.latest_version = 0;

	for (u32 offset = 0; offset < MOD_STORAGE_BYTES; offset += MOD_STORAGE_PAGE_SIZE) {
		const u8 *flash_location = absolute_flash_location(offset);
		if (page_is_erased(flash_location)) continue;

		const settings_record_t *record = (const settings_record_t*)flash_location; // read data as record
		if (!record_valid(record)) continue;

		if (!storage_state.has_records || record->version > storage_state.latest_version) {
			utils_printf("latest version: %lu\n", (unsigned long)record->version);
			storage_state.has_records = true;
			storage_state.latest_version = record->version;
			storage_state.latest_offset = offset;
		}
	}

	if (!storage_state.has_records)
		utils_printf("nothing found :(\n");
}

bool storage_init() {
	utils_crc_init(); // just to be sure

	full_rescan();

	return storage_state.has_records;
}

bool storage_load(void *out, const u32 len) {
	u8 *bytes = (u8*)out;
	if (!storage_state.has_records || len > MOD_STORAGE_PAYLOAD_BYTES) return false;

	const settings_record_t *record = (const settings_record_t*)absolute_flash_location(storage_state.latest_offset);

	if (!record_valid(record)) {
		utils_printf("tried to load at %p - invalid record (try older version?)\n",
		             (const void*)absolute_flash_location(storage_state.latest_offset));
		return false;
	}

	memcpy(bytes, record->payload, len);
	utils_printf("payload loaded, version: %lu\n", (unsigned long)record->version);
	return true;
}

bool storage_save(const void *data, const u32 len) {
	if (len > MOD_STORAGE_PAYLOAD_BYTES) return false;
	const u8 *bytes = (const u8*)data;

	const u32 destination = storage_state.has_records ? page_advance(storage_state.latest_offset) : 0u;

	u8 page[MOD_STORAGE_PAGE_SIZE];
	for (u32 i = 0; i < MOD_STORAGE_PAGE_SIZE; i++) page[i] = 0xFF;

	settings_record_t *record = (settings_record_t*)page;
	record->version = storage_state.latest_version + 1u;
	memcpy(record->payload, bytes, len);

	record->crc32 = 0;
	record->crc32 = utils_crc(record, sizeof *record);

	multicore_lockout_start_blocking();

	if ((destination % MOD_STORAGE_SECTOR_SIZE) == 0) {
		utils_printf("erasing sector at offset 0x%08lX (XIP %p)\n",
		             (unsigned long)(MOD_STORAGE_OFFSET + destination),
		             (const void*)absolute_flash_location(destination));
		flash_range_erase(MOD_STORAGE_OFFSET + destination, MOD_STORAGE_SECTOR_SIZE);
	}

	utils_printf("writing version %lu to %p\n",
	             (unsigned long)record->version,
	             (const void*)absolute_flash_location(destination));
	flash_range_program(MOD_STORAGE_OFFSET + destination, page, MOD_STORAGE_PAGE_SIZE);

	multicore_lockout_end_blocking();

	// TODO: read back and try again with next page (limit try count to 100?)

	storage_state.has_records = true;
	storage_state.latest_version = record->version;
	storage_state.latest_offset = destination;

	return true;
}

void storage_erase_all() {
	for (u32 i = 0; i < MOD_STORAGE_SECTORS; i++) {
		const u32 sector_offset = sector_start(i);
		utils_printf("erasing sector %u\n", (unsigned)i);
		flash_range_erase(MOD_STORAGE_OFFSET + sector_offset, MOD_STORAGE_SECTOR_SIZE);
	}

	storage_state = (storage_state_t) { 0 };
}
