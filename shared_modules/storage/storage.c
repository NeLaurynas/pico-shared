// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#include "storage.h"

#include <string.h>

#include "utils.h"
#include "hardware/flash.h"
#include "pico/flash.h"
#include "pico/multicore.h"

typedef struct {
	char type[4];
	bool has_records;
	u32 latest_version;
	u32 latest_offset;
} storage_type_state_t;

typedef struct {
	bool has_records;
	u32 latest_offset;
	storage_type_state_t types[MOD_STORAGE_DATA_TYPES];
} storage_state_t;

static storage_state_t storage_state = { };

#define STORAGE_WRITE_MAX_TRIES		25
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

static inline u32 entry_advance(const u32 offset) {
	const u32 next = offset + MOD_STORAGE_ENTRY_BYTES;
	return (next + MOD_STORAGE_ENTRY_BYTES > MOD_STORAGE_BYTES) ? 0u : next;
}

static bool entry_is_erased(const u8 *flash_location) {
	for (auto i = 0; i < 16; i++) if (flash_location[i] != 0xFF) return false; // quick check of entry header
	return true;
}

static bool record_valid(const storage_record_t *record) {
	storage_record_t tmp = *record; // copy
	tmp.crc32 = 0;
	const u32 c = utils_crc(&tmp, sizeof tmp);

	return record->crc32 == c;
}

static bool type_identifier_complete(const storage_type_state_t *state) {
	for (size_t i = 0; i < sizeof state->type; i++) if (state->type[i] == 0) return false;
	return true;
}

static i8 index_by_type(const char type[4]) {
	for (i8 i = 0; i < MOD_STORAGE_DATA_TYPES; i++) if (memcmp(type, storage_state.types[i].type, 4) == 0) return i;

	return -1;
}

static void full_rescan() {
	storage_state.latest_offset = 0;
	storage_state.has_records = false;
	for (auto i = 0; i < MOD_STORAGE_DATA_TYPES; i++) {
		storage_state.types[i].has_records = false;
		storage_state.types[i].latest_offset = 0;
		storage_state.types[i].latest_version = 0;
	}

	for (u32 offset = 0; offset + MOD_STORAGE_ENTRY_BYTES <= MOD_STORAGE_BYTES; offset += MOD_STORAGE_ENTRY_BYTES) {
		const u8 *flash_location = absolute_flash_location(offset);
		if (entry_is_erased(flash_location)) continue;

		const storage_record_t *record = (const storage_record_t*)flash_location; // read data as record

		if (!record_valid(record)) continue;

		const auto type_index = index_by_type(record->type);
		if (type_index < 0) continue;

		storage_type_state_t *state = &storage_state.types[type_index];
		if (!state->has_records || record->version > state->latest_version) {
			utils_printf("found data: %.*s -> %lu\n",
			             (int)sizeof state->type,
			             state->type,
			             (unsigned long)state->latest_version);
			state->has_records = true;
			state->latest_version = record->version;
			state->latest_offset = offset;
			// save global state
			storage_state.latest_offset = offset;
			storage_state.has_records = true;
		}
	}

	for (auto i = 0; i < MOD_STORAGE_DATA_TYPES; i++) {
			if (!storage_state.types[i].has_records)
				utils_printf("no data for type %.*s\n",
				             (int)sizeof storage_state.types[i].type,
				             storage_state.types[i].type);
	}
}

void storage_init(bool out[MOD_STORAGE_DATA_TYPES]) {
	utils_crc_init(); // just to be sure

	for (i8 i = 0; i < MOD_STORAGE_DATA_TYPES; i++) {
		if (!type_identifier_complete(&storage_state.types[i])) {
			utils_printf("storage_init: type index %d identifier missing\n", i);
			panic("about to fuck up storage mate");
		}
	}

	full_rescan();

	for (auto i = 0; i < MOD_STORAGE_DATA_TYPES; i++) out[i] = storage_state.types[i].has_records;
}

void storage_register_data_type(const u8 index, const char identifier[4]) {
	if (index >= MOD_STORAGE_DATA_TYPES) {
		utils_printf("!! index >= MOD_STORAGE_DATA_TYPES");
		return;
	}

	memcpy(storage_state.types[index].type, identifier, 4);
}

bool storage_load(const u8 index, void *out, const u32 len) {
	u8 *bytes = (u8*)out;
	if (index >= MOD_STORAGE_DATA_TYPES) return false;
	if (!storage_state.types[index].has_records || len > MOD_STORAGE_PAYLOAD_BYTES) return false;

	const storage_record_t *record = (const storage_record_t*)absolute_flash_location(storage_state.types[index].latest_offset);

	if (!record_valid(record)) {
		utils_printf("tried to load at %p - invalid record (try older version?)\n",
		             (const void*)absolute_flash_location(storage_state.latest_offset));
		return false;
	}

	memcpy(bytes, record->payload, len);
	return true;
}

// ReSharper disable once CppDFAConstantFunctionResult clion u dum dum
bool storage_save(const u8 index, const void *data, const u32 len) {
	if (index >= MOD_STORAGE_DATA_TYPES) return false;
	if (len > MOD_STORAGE_PAYLOAD_BYTES) return false;
	const u8 *bytes = (const u8*)data;

	u32 destination = storage_state.has_records ? entry_advance(storage_state.latest_offset) : 0u;

	u8 entry[MOD_STORAGE_ENTRY_BYTES];
	memset(entry, 0xFF, sizeof entry);

	storage_record_t *record = (storage_record_t*)entry;
	storage_type_state_t *state = &storage_state.types[index];

	memcpy(record->type, storage_state.types[index].type, 4);
	record->version = state->latest_version + 1u;
	memcpy(record->payload, bytes, len);

	record->crc32 = 0;
	record->crc32 = utils_crc(record, sizeof *record);

	int rc = 0;

	for (u32 attempt = 0; attempt < STORAGE_WRITE_MAX_TRIES; attempt++) {
		utils_printf("writing version %lu attempt %lu to %p\n",
		             (unsigned long)record->version,
		             (unsigned long)(attempt + 1u),
		             (const void*)absolute_flash_location(destination));

		u32 erased_sector_offset = MOD_STORAGE_BYTES;
		bool erase_failed = false;
		for (u32 page_index = 0; page_index < MOD_STORAGE_ENTRY_PAGES; page_index++) {
			const u32 page_offset = destination + (page_index * MOD_STORAGE_PAGE_SIZE);

			if ((page_offset % MOD_STORAGE_SECTOR_SIZE) == 0 && erased_sector_offset != page_offset) {
				utils_printf("erasing sector at offset 0x%08lX (XIP %p)\n",
				             (unsigned long)(MOD_STORAGE_OFFSET + page_offset),
				             (const void*)absolute_flash_location(page_offset));
				rc = flash_safe_execute(call_flash_range_erase,
				                        (void*)(uintptr_t)(MOD_STORAGE_OFFSET + page_offset),
				                        UINT32_MAX);
				if (rc != PICO_OK) {
					utils_printf("erase failed: %d (if -4 then forgot flash_safe_execute_core_init();)\n", rc);
					erase_failed = true;
					break;
				}
				erased_sector_offset = page_offset;
			}

			uintptr_t prog_params[] = {
				(uintptr_t)(MOD_STORAGE_OFFSET + page_offset),
				(uintptr_t)(entry + (page_index * MOD_STORAGE_PAGE_SIZE))
			};

			rc = flash_safe_execute(call_flash_range_program, prog_params, UINT32_MAX);
			if (rc != PICO_OK) break;
		}

		if (erase_failed) return false;

		if (rc == PICO_OK) {
			const storage_record_t *verify = (const storage_record_t*)absolute_flash_location(destination);

			if (record_valid(verify) && verify->version == record->version) {
				// ReSharper disable once CppDFAUnreachableCode - clion u dum dum
				storage_state.has_records = true;
				state->has_records = true;
				state->latest_version = record->version;
				storage_state.latest_offset = destination;
				state->latest_offset = destination;
				return true;
			}

			utils_printf("verify failed at %p, advancing to next entry\n",
			             (const void*)absolute_flash_location(destination));
		} else {
			utils_printf("program failed: %d (if -4 then forgot flash_safe_execute_core_init();)\n", rc);
		}

		destination = entry_advance(destination);
	}

	utils_printf("write failed after %lu attempts\n", (unsigned long)STORAGE_WRITE_MAX_TRIES);
	return false;
}

void storage_erase_all() {
	for (auto i = 0; i < MOD_STORAGE_SECTORS; i++) {
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
