# pico-shared
Shared stuff for Pico projects

## Setup
1. Add git submodule to `src/lib/pico-shared`
1. Include `shared_config.h` in `config.h`- or probably copy paste shared_config.h and edit there
	1. To override settings - in `config.h`:
		```c
		#undef MOD_MCP_PIN_SDA
		#define MOD_MCP_PIN_SDA		16
1. Update CMakeLists.txt:
	1. Add directories:
		```cmake
		add_subdirectory(lib/pico-shared/src)
		target_link_libraries(${PROJECT_NAME} pico-shared)
	1. Update `target_include_directories`:
		```cmake
		${CMAKE_CURRENT_LIST_DIR}/lib/pico-shared/src
1. Profit probably
