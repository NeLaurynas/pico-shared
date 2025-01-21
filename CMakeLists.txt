cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico2 CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(pico-shared C CXX ASM)

pico_sdk_init()

add_library(pico-shared STATIC
		utils.c
		utils.h
		shared_modules/mcp/mcp.c
		shared_modules/mcp/mcp.h
		shared_config.h
)

get_target_property(PICO_STD_INCLUDE pico_stdlib INTERFACE_INCLUDE_DIRECTORIES)

target_compile_options(pico-shared PRIVATE
		$<$<CONFIG:Debug>:
		-Og
		-g
		#-Wall
		#-Wextra
		#-Winline
		#-Werror
		-Wno-error=old-style-declaration
		-march=armv8-m.main
		-mtune=cortex-m33
		-mfpu=fpv5-sp-d16
		-mthumb
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
		-march=armv8-m.main
		-mtune=cortex-m33
		-mfpu=fpv5-sp-d16
		-mthumb
		>
)

target_compile_options(pico-shared PRIVATE
		$<$<COMPILE_LANGUAGE:C>:-Wno-old-style-declaration>
)

target_link_options(pico-shared PRIVATE
		-march=armv8-m.main
		-mtune=cortex-m33
		-mfpu=fpv5-sp-d16
		-mthumb
)

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
		pico_rand
		$<$<CONFIG:Debug>:hardware_uart>
)
