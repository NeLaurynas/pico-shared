cmake_minimum_required(VERSION 3.30)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Standalone support: when configured directly, import the Pico SDK first so
# the GCC ARM toolchain and flags are set before project() runs.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif ()

project(pico-shared C CXX ASM)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	pico_sdk_init()
endif ()

# Match the base project flag sets when included as a subdirectory, and
# provide sensible defaults when building these libraries standalone.
if (NOT DEFINED BASE_CPU_FEATURE_FLAGS)
	set(BASE_CPU_FEATURE_FLAGS
			-mcpu=cortex-m33
			-mtune=cortex-m33
			-mfpu=fpv5-sp-d16
			-mthumb
	)
endif ()

if (NOT DEFINED BASE_SECTION_FLAGS)
	set(BASE_SECTION_FLAGS
			-ffunction-sections
			-fdata-sections
	)
endif ()

if (NOT DEFINED BASE_WARNING_FLAGS)
	set(BASE_WARNING_FLAGS
			-Wall
			-Wextra
			-Winline
			-Wno-error=old-style-declaration
	)
endif ()

if (NOT DEFINED BASE_DEBUG_FLAGS)
	set(BASE_DEBUG_FLAGS
			-Og
			-g3
	)
endif ()

if (NOT DEFINED BASE_RELEASE_FLAGS)
	set(BASE_RELEASE_FLAGS
			-O1
			-g0
	)
endif ()

add_library(pico_shared_options INTERFACE)
target_compile_options(pico_shared_options INTERFACE
		${BASE_CPU_FEATURE_FLAGS}
		${BASE_SECTION_FLAGS}
		${BASE_WARNING_FLAGS}
		$<$<CONFIG:Debug>:${BASE_DEBUG_FLAGS}>
		$<$<CONFIG:Release>:${BASE_RELEASE_FLAGS}>
		$<$<COMPILE_LANGUAGE:C>:-Wno-old-style-declaration>
)
target_link_options(pico_shared_options INTERFACE
		${BASE_CPU_FEATURE_FLAGS}
		$<$<CONFIG:Release>:-Wl,--strip-all>
)

add_library(pico_shared_config INTERFACE)
target_include_directories(pico_shared_config INTERFACE
		"${CMAKE_CURRENT_LIST_DIR}"
)

function(pico_shared_add_library target)
	add_library(${target} STATIC ${ARGN})
	target_link_libraries(${target}
			PUBLIC
				pico_shared_config
			PRIVATE
				pico_shared_options
	)
endfunction()

pico_shared_add_library(pico_shared_memory
		shared_modules/memory/memory.c
		shared_modules/memory/memory.h
)

pico_shared_add_library(pico_shared_utils
		utils.c
		utils.h
)
target_link_libraries(pico_shared_utils
		PUBLIC
			hardware_pwm
		PRIVATE
			hardware_adc
			pico_rand
			pico_shared_memory
			pico_status_led
			pico_stdlib
)

pico_shared_add_library(pico_shared_frtos
		frtos.c
		frtos.h
)
if (TARGET FreeRTOS-Kernel)
	target_compile_definitions(pico_shared_frtos PUBLIC PICO_SHARED_HAS_FREERTOS=1)
	target_compile_definitions(pico_shared_frtos PRIVATE
			FREE_RTOS_KERNEL_SMP=1
			LIB_FREERTOS_KERNEL=1
			PICO_CONFIG_RTOS_ADAPTER_HEADER=${FREERTOS_KERNEL_PATH}/${FREERTOS_KERNEL_PORT_PATH}/non_secure/freertos_sdk_config.h
	)
	target_include_directories(pico_shared_frtos PRIVATE
			${FREERTOS_CONFIG_FILE_DIRECTORY}
			${FREERTOS_KERNEL_PATH}/include
			${FREERTOS_KERNEL_PATH}/${FREERTOS_KERNEL_PORT_PATH}/non_secure
	)
else ()
	target_compile_definitions(pico_shared_frtos PUBLIC PICO_SHARED_HAS_FREERTOS=0)
endif ()
target_link_libraries(pico_shared_frtos PRIVATE pico_stdlib)

pico_shared_add_library(pico_shared_str
		str.c
		str.h
)

pico_shared_add_library(pico_shared_anim
		anim.c
		anim.h
)
target_link_libraries(pico_shared_anim PRIVATE
		m
		pico_shared_utils
)

pico_shared_add_library(pico_shared_mcp
		shared_modules/mcp/mcp.c
		shared_modules/mcp/mcp.h
		shared_modules/mcp/shared_config.h
)
target_link_libraries(pico_shared_mcp PRIVATE
		hardware_gpio
		hardware_i2c
		pico_shared_utils
)

pico_shared_add_library(pico_shared_storage
		shared_modules/storage/storage.c
		shared_modules/storage/storage.h
		shared_modules/storage/shared_config.h
)
target_link_libraries(pico_shared_storage PRIVATE
		hardware_flash
		pico_flash
		pico_multicore
		pico_shared_utils
)

pico_shared_add_library(pico_shared_v_monitor
		shared_modules/v_monitor/v_monitor.c
		shared_modules/v_monitor/v_monitor.h
		shared_modules/v_monitor/shared_config.h
)
target_link_libraries(pico_shared_v_monitor PRIVATE
		hardware_adc
		pico_shared_utils
)

pico_shared_add_library(pico_shared_wsleds
		shared_modules/wsleds/wsleds.c
		shared_modules/wsleds/wsleds.h
		shared_modules/wsleds/wsleds_data.h
		shared_modules/wsleds/shared_config.h
)
pico_generate_pio_header(pico_shared_wsleds ${CMAKE_CURRENT_LIST_DIR}/shared_modules/wsleds/pio_wsleds.pio)
target_link_libraries(pico_shared_wsleds PRIVATE
		hardware_dma
		hardware_pio
		pico_shared_anim
		pico_shared_utils
)

pico_shared_add_library(pico_shared_wsledswhite
		shared_modules/wsledswhite/wsledswhite.c
		shared_modules/wsledswhite/wsledswhite.h
		shared_modules/wsledswhite/wsledswhite_data.h
		shared_modules/wsledswhite/shared_config.h
)
pico_generate_pio_header(pico_shared_wsledswhite ${CMAKE_CURRENT_LIST_DIR}/shared_modules/wsledswhite/pio_wsledswhite.pio)
target_link_libraries(pico_shared_wsledswhite PRIVATE
		hardware_dma
		hardware_pio
		pico_shared_anim
		pico_shared_utils
)

pico_shared_add_library(pico_shared_cpu_cores
		shared_modules/cpu_cores/cpu_cores.c
		shared_modules/cpu_cores/cpu_cores.h
)
target_link_libraries(pico_shared_cpu_cores
		PUBLIC
			pico_util
		PRIVATE
			hardware_adc
			hardware_clocks
			hardware_dma
			hardware_i2c
			hardware_irq
			hardware_pio
			hardware_pwm
			hardware_uart
			hardware_xosc
			pico_multicore
			pico_shared_utils
)

pico_shared_add_library(pico_shared_mp3
		shared_modules/mp3/mp3.c
		shared_modules/mp3/mp3.h
		shared_modules/mp3/horn_mp3.h
		shared_modules/mp3/shared_config.h
)

add_library(pico_shared_app_settings INTERFACE)
target_link_libraries(pico_shared_app_settings INTERFACE
		pico_shared_config
		pico_shared_storage
)

add_library(pico-shared INTERFACE)
target_link_libraries(pico-shared INTERFACE
		pico_shared_anim
		pico_shared_app_settings
		pico_shared_cpu_cores
		pico_shared_frtos
		pico_shared_mcp
		pico_shared_memory
		pico_shared_mp3
		pico_shared_storage
		pico_shared_str
		pico_shared_utils
		pico_shared_v_monitor
		pico_shared_wsleds
		pico_shared_wsledswhite
)
