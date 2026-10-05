# pico-shared

Shared Pico C library: utilities, hardware helpers, flash layout.

Phobos adds it through CMake. `PICO_SHARED_PROJECT_CONFIG_DIR` selects `shared_config.h`. `PICO_SHARED_CPU_EXTRAS` defaults ON; Phobos uses OFF, keeping clock/ADC/temperature helpers. Standalone build imports Pico SDK.
