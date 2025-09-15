cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico2 CACHE STRING "Board type")

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico-shared C CXX ASM)

pico_sdk_init()

# Storage Flash layout. Set how many sectors to reserve; the rest is for code.
if (NOT DEFINED FLASH_TOTAL_BYTES)
	set(FLASH_TOTAL_BYTES 4194304 CACHE STRING "Total on-board flash size (bytes)") # Pico 2 flash size - 4 mB
endif ()

set(MOD_STORAGE_SECTOR_SIZE 4096 CACHE STRING "Flash sector size (bytes)") # Pico 2 sector size - 4 kB
set(MOD_STORAGE_SECTORS 128 CACHE STRING "Number of sectors to reserve for storage")
math(EXPR MOD_STORAGE_BYTES "${MOD_STORAGE_SECTORS} * ${MOD_STORAGE_SECTOR_SIZE}")
math(EXPR APP_FLASH_BYTES "${FLASH_TOTAL_BYTES} - ${MOD_STORAGE_BYTES}")

add_library(pico-shared STATIC
		utils.c
		utils.h
		shared_modules/mcp/mcp.c
		shared_modules/mcp/mcp.h
		shared_config.h
		shared_modules/mp3/mp3.c
		shared_modules/mp3/mp3.h
		shared_modules/mp3/horn_mp3.h
		shared_modules/wsleds/wsleds.c
		shared_modules/wsleds/wsleds.h
		shared_modules/wsleds/wsleds_data.h
		shared_modules/wsledswhite/wsledswhite.c
		shared_modules/wsledswhite/wsledswhite.h
		shared_modules/wsledswhite/wsledswhite_data.h
		anim.c
		anim.h
		shared_modules/v_monitor/v_monitor.c
		shared_modules/v_monitor/v_monitor.h
		shared_modules/cpu_cores/cpu_cores.c
		shared_modules/cpu_cores/cpu_cores.h
		shared_modules/memory/memory.c
		shared_modules/memory/memory.h
		shared_modules/storage/storage.c
		shared_modules/storage/storage.h
		str.c
		str.h
)

target_compile_definitions(pico-shared PRIVATE
        PICO_FLASH_SIZE_BYTES=${FLASH_TOTAL_BYTES}
        MOD_STORAGE_SECTORS=${MOD_STORAGE_SECTORS}
        MOD_STORAGE_SECTOR_SIZE=${MOD_STORAGE_SECTOR_SIZE}
        MOD_STORAGE_PAGE_SIZE=256 # Pico 2 page size in bytes
        MOD_STORAGE_BYTES=${MOD_STORAGE_BYTES}
        MOD_STORAGE_OFFSET=${APP_FLASH_BYTES}
)

get_target_property(PICO_STD_INCLUDE pico_stdlib INTERFACE_INCLUDE_DIRECTORIES)

target_compile_options(pico-shared PRIVATE
		$<$<CONFIG:Debug>:
		-Og # disable if you get <optimized out>
		-g3
		-Wall
		-Wextra
		-Winline
		#		-Werror
		-Wno-error=old-style-declaration
		-mcpu=cortex-m33
		-mtune=cortex-m33
		-mfpu=fpv5-sp-d16
		-mthumb
		-ffunction-sections
		-fdata-sections
		>

		$<$<CONFIG:Release>:
		-Wall
		-Wextra
		-Winline
		#-Werror
		-Wno-error=old-style-declaration
		-O3
		-s
		-g0
		-mcpu=cortex-m33
		-mtune=cortex-m33
		-mfpu=fpv5-sp-d16
		-mthumb
		-ffunction-sections
		-fdata-sections
		>
)

target_compile_options(pico-shared PRIVATE
		$<$<COMPILE_LANGUAGE:C>:-Wno-old-style-declaration>
)

target_link_options(pico-shared PRIVATE
		-mcpu=cortex-m33
		-mtune=cortex-m33
		-mfpu=fpv5-sp-d16
		-mthumb
)

# PIO
pico_generate_pio_header(pico-shared ${CMAKE_CURRENT_LIST_DIR}/shared_modules/wsleds/pio_wsleds.pio)
pico_generate_pio_header(pico-shared ${CMAKE_CURRENT_LIST_DIR}/shared_modules/wsledswhite/pio_wsledswhite.pio)

target_link_libraries(pico-shared
		pico_stdlib)

# Add the standard include files to the build
target_include_directories(pico-shared PRIVATE
		${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pico-shared
		hardware_i2c
		hardware_pio
		hardware_adc
		hardware_dma
		hardware_pwm
		pico_multicore
		hardware_irq
		hardware_flash
		pico_rand
		$<$<CONFIG:Debug>:hardware_uart>
)

configure_file(
		memmap_storage.ld.in
		${CMAKE_CURRENT_BINARY_DIR}/memmap_storage.ld
		@ONLY
)
pico_set_linker_script(pico-shared ${CMAKE_CURRENT_BINARY_DIR}/memmap_storage.ld)
