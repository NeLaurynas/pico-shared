// Copyright (C) 2026 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "../../shared_config.h"

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES       4194304u
#endif

#ifndef MOD_STORAGE_SECTOR_SIZE
#define MOD_STORAGE_SECTOR_SIZE     4096u
#endif

#ifndef MOD_STORAGE_SECTORS
#define MOD_STORAGE_SECTORS         128u
#endif

#ifndef MOD_STORAGE_PAGE_SIZE
#define MOD_STORAGE_PAGE_SIZE       256u
#endif

#ifndef MOD_STORAGE_BYTES
#define MOD_STORAGE_BYTES           (MOD_STORAGE_SECTORS * MOD_STORAGE_SECTOR_SIZE)
#endif

#ifndef MOD_STORAGE_OFFSET
#define MOD_STORAGE_OFFSET          (PICO_FLASH_SIZE_BYTES - MOD_STORAGE_BYTES)
#endif

#ifndef MOD_STORAGE_ENTRY_PAGES
#define MOD_STORAGE_ENTRY_PAGES     2u
#endif

#ifndef MOD_STORAGE_DATA_TYPES
#define MOD_STORAGE_DATA_TYPES      2u
#endif
