# pico-shared

Internal static library shared across Vesta Pico projects (currently used by Phobos).

This is the only library under `projects/phobos/lib/` that Vesta treats as “owned” code; other libraries are vendored/external.

## What’s inside
- Small utilities: `utils.[ch]`, `str.[ch]`, `anim.[ch]`
- Hardware helpers under `shared_modules/` (e.g. storage layout, voltage monitor, WS LED drivers)
- Flash layout helpers: `memmap_storage.ld.in` and related build plumbing

## How it’s used
Phobos pulls this library in via CMake (`projects/phobos/src/CMakeLists.txt`) using `add_subdirectory(...)` and links it into the firmware image.

The library can also be configured standalone (it will import the Pico SDK when configured directly), but the normal workflow is to build it as part of the Phobos build.
