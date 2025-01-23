# pico-shared
Shared stuff for Pico projects

## Setup
1. Add git submodule to `<root>/lib/pico-shared`
1. Copy to `src` and modify `shared_config.h`
1. Update CMakeLists.txt:
	1. Update `target_include_directories`:
		```cmake
		${CMAKE_CURRENT_LIST_DIR}/../lib/pico-shared
	1. Add directories:
	   ```cmake
       add_subdirectory(../lib/pico-shared ${CMAKE_BINARY_DIR}/pico-shared)
       target_link_libraries(${PROJECT_NAME} pico-shared)
1. Profit probably
