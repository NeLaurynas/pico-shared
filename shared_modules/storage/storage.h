// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PICO_SHARED_STORAGE_H
#define PICO_SHARED_STORAGE_H

#include "shared_config.h"

#define MOD_STORAGE_ENTRY_BYTES      (MOD_STORAGE_PAGE_SIZE * MOD_STORAGE_ENTRY_PAGES)
#define MOD_STORAGE_PAYLOAD_BYTES    (MOD_STORAGE_ENTRY_BYTES - 12u) // version(4) + crc32(4) + type(4)

typedef struct __attribute__((packed)) {
	char type[4];
	u32 version;
	u32 crc32;
	u8 payload[MOD_STORAGE_PAYLOAD_BYTES];
} storage_record_t;

static_assert(sizeof(storage_record_t) == MOD_STORAGE_ENTRY_BYTES, "record size mismatch");
static_assert((MOD_STORAGE_SECTOR_SIZE % MOD_STORAGE_PAGE_SIZE) == 0, "sector must be multiple of page");
static_assert((MOD_STORAGE_BYTES % MOD_STORAGE_SECTOR_SIZE) == 0, "reserved bytes must be multiple of sector");
static_assert(MOD_STORAGE_SECTORS >= 2, "reserve at least 2 sectors to avoid self-erasing latest");
static_assert((MOD_STORAGE_OFFSET % MOD_STORAGE_SECTOR_SIZE) == 0, "offset must be sector aligned");
static_assert((MOD_STORAGE_BYTES % MOD_STORAGE_SECTOR_SIZE) == 0, "size must be sector multiple");
static_assert(MOD_STORAGE_OFFSET + MOD_STORAGE_BYTES <= PICO_FLASH_SIZE_BYTES, "storage exceeds flash");
static_assert(MOD_STORAGE_ENTRY_BYTES <= MOD_STORAGE_BYTES, "entry larger than reserved storage");

/**
 * @warning if only core0 is running - PICO_FLASH_ASSUME_CORE1_SAFE=1
 * @warning Use \c memmap_storage.ld.in to reserve flash for storage!
 * @warning if multicore setup - \c flash_safe_execute_core_init() on coreA if calling from coreB.
 * @details
configure_file(
		${PICO_SHARED_COPY_PARENT}/pico-shared/memmap_storage.ld.in
		${CMAKE_CURRENT_BINARY_DIR}/memmap_storage.ld
		@ONLY
)
pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/memmap_storage.ld)
 * @param out \c true if all good; \c false if no records - \b first \b boot?
 */
void storage_init(bool out[MOD_STORAGE_DATA_TYPES]);

void storage_register_data_type(const u8 index, const char identifier[4]);

[[nodiscard]] bool storage_load(const u8 index, void *out, const u32 len);

[[nodiscard]] bool storage_save(const u8 index, const void *data, const u32 len);

void storage_erase_all();

#endif //PICO_SHARED_STORAGE_H
