// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "storage.h"

#include <string.h>

#include "utils.h"
#include "hardware/flash.h"
#include "pico/flash.h"
#include "pico/multicore.h"

typedef struct {
	bool has_records;
	u32 latest_version;
	u32 latest_offset;
} storage_state_t;

static storage_state_t storage_state = { };

#define STORAGE_WRITE_MAX_TRIES		100u
#define STORAGE_BASE_XIP			((uintptr_t)(XIP_BASE + (u32)MOD_STORAGE_OFFSET))

// --- helpers from pico examples
static void call_flash_range_erase(void *param) {
	const u32 abs_off = (uintptr_t)param;
	flash_range_erase(abs_off, MOD_STORAGE_SECTOR_SIZE);
}

static void call_flash_range_program(void *param) {
	const uintptr_t *p = (uintptr_t*)param;
	const u32 abs_off = p[0];
	const u8 *data = (const u8*)p[1];
	flash_range_program(abs_off, data, MOD_STORAGE_PAGE_SIZE);
}

// --- /helpers from pico examples

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
	for (u32 i = 0; i < 5; i++) if (flash_location[i] != 0xFF) return false; // yea check only first page locations
	return true;
}

static bool record_valid(const settings_record_t *record) {
	settings_record_t tmp = *record; // copy
	tmp.crc32 = 0;
	const u32 c = utils_crc(&tmp, sizeof tmp);

	return record->crc32 == c;
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
	return true;
}

// ReSharper disable once CppDFAConstantFunctionResult clion u dum dum
bool storage_save(const void *data, const u32 len) {
	if (len > MOD_STORAGE_PAYLOAD_BYTES) return false;
	const u8 *bytes = (const u8*)data;

	u32 destination = storage_state.has_records ? page_advance(storage_state.latest_offset) : 0u;

	u8 page[MOD_STORAGE_PAGE_SIZE];
	for (u32 i = 0; i < MOD_STORAGE_PAGE_SIZE; i++) page[i] = 0xFF;

	settings_record_t *record = (settings_record_t*)page;
	record->version = storage_state.latest_version + 1u;
	memcpy(record->payload, bytes, len);

	record->crc32 = 0;
	record->crc32 = utils_crc(record, sizeof *record);

	int rc;

	for (u32 attempt = 0; attempt < STORAGE_WRITE_MAX_TRIES; attempt++) {
		if ((destination % MOD_STORAGE_SECTOR_SIZE) == 0) {
			utils_printf("erasing sector at offset 0x%08lX (XIP %p)\n",
			             (unsigned long)(MOD_STORAGE_OFFSET + destination),
			             (const void*)absolute_flash_location(destination));
			rc = flash_safe_execute(call_flash_range_erase,
			                        (void*)(uintptr_t)(MOD_STORAGE_OFFSET + destination),
			                        UINT32_MAX);
			if (rc != PICO_OK) {
				utils_printf("erase failed: %d (if -4 then forgot flash_safe_execute_core_init();)\n", rc);
				destination = page_advance(destination);
				continue;
			}
		}

		utils_printf("writing version %lu attempt %lu to %p\n",
		             (unsigned long)record->version,
		             (unsigned long)(attempt + 1u),
		             (const void*)absolute_flash_location(destination));
		uintptr_t prog_params[] = { (uintptr_t)(MOD_STORAGE_OFFSET + destination), (uintptr_t)page };

		rc = flash_safe_execute(call_flash_range_program, prog_params, UINT32_MAX);
		if (rc == PICO_OK) {
			const settings_record_t *verify = (const settings_record_t*)absolute_flash_location(destination);

			if (record_valid(verify) && verify->version == record->version) {
				// ReSharper disable once CppDFAUnreachableCode - clion u dum dum
				storage_state.has_records = true;
				storage_state.latest_version = record->version;
				storage_state.latest_offset = destination;
				return true;
			}

			utils_printf("verify failed at %p, advancing to next page\n",
			             (const void*)absolute_flash_location(destination));
		} else {
			utils_printf("program failed: %d (if -4 then forgot flash_safe_execute_core_init();)\n", rc);
		}

		destination = page_advance(destination);
	}

	utils_printf("write failed after %lu attempts\n", (unsigned long)STORAGE_WRITE_MAX_TRIES);
	return false;
}

void storage_erase_all() {
	for (u32 i = 0; i < MOD_STORAGE_SECTORS; i++) {
		const u32 sector_offset = sector_start(i);
		const u32 off = MOD_STORAGE_OFFSET + sector_offset;

		utils_printf("erasing sector %u at 0x%08lX\n", (unsigned)i, (unsigned long)off);

		const int rc = flash_safe_execute(call_flash_range_erase, (void*)(uintptr_t)off, UINT32_MAX);
		if (rc != PICO_OK) {
			utils_printf("erase failed: %d\n", rc);
		}
	}

	storage_state = (storage_state_t) { 0 };
}
