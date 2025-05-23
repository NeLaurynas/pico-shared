cmake_minimum_required(VERSION 3.30)

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
		shared_modules/mp3/mp3.c
		shared_modules/mp3/mp3.h
		shared_modules/mp3/horn_mp3.h
		shared_modules/wsleds/wsleds.c
		shared_modules/wsleds/wsleds.h
		shared_modules/wsleds/wsleds_data.h
		anim.c
		anim.h
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
		pico_rand
		$<$<CONFIG:Debug>:hardware_uart>
)
