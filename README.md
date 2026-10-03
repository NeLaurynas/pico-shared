# pico-shared

Static library shared across Vesta Pico projects (currently Phobos); the only Vesta-owned code under `projects/phobos/lib/`.

- Utilities: `utils.[ch]`, `str.[ch]`, `anim.[ch]`.
- Hardware helpers in `shared_modules/` (storage layout, voltage monitor, WS LED drivers).
- Flash layout: `memmap_storage.ld.in` and its build plumbing.

Phobos adds it with `add_subdirectory(...)` in `projects/phobos/src/CMakeLists.txt`. `PICO_SHARED_PROJECT_CONFIG_DIR` selects the project's `shared_config.h` (else library defaults). `PICO_SHARED_CPU_EXTRAS` (default `ON`) adds the CPU shutdown and calculation APIs; Phobos sets it `OFF`, keeping clock, ADC and temperature helpers. Configured standalone, the library imports the Pico SDK itself.
