// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PICO_SHARED_CPU_CORES_H
#define PICO_SHARED_CPU_CORES_H

enum {
	CPU_CORES_CMD_NOOP = 0, CPU_CORES_CMD_SHUTDOWN = 1
};

void cpu_cores_init_from_core0(void (*callback)());

void cpu_cores_send_shutdown_to_core0_from_core1();

/**
*	!!! Don't forget to turn off BT and WiFi stack before calling this (pico-shared doesn't include btstack, etc)
	utils_printf("wifi and bt shutdown\n");
	hci_power_control(HCI_POWER_OFF);
	cyw43_arch_deinit();
*/
void cpu_cores_shutdown_from_core0();

#endif //PICO_SHARED_CPU_CORES_H
