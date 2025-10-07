// Copyright (C) 2025 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef PICO_SHARED_CPU_CORES_H
#define PICO_SHARED_CPU_CORES_H
#include <pico/util/queue.h>

typedef enum {
	CPU_CORES_CMD_NOOP = 0, CPU_CORES_CMD_SHUTDOWN = 1369420
} mod_cores_cmd_t;

extern queue_t mod_cpu_core0_queue;

/**
 * @warning if btstack - setup btstack timers and call this there
 * @details
static btstack_timer_source_t btstack_qpoll;
static void qpoll_handler(btstack_timer_source_t *ts) {
	i32 command;
	while (queue_try_remove(&mod_cpu_core0_queue, &command)) if (command == CPU_CORES_CMD_SHUTDOWN) btstack_run_loop_trigger_exit();
	btstack_run_loop_set_timer(ts, 100);
	btstack_run_loop_add_timer(ts);
}
void btstack_handler_init() {
	cpu_cores_init_from_core0();
	btstack_run_loop_set_timer_handler(&btstack_qpoll, &qpoll_handler);
	btstack_run_loop_set_timer(&btstack_qpoll, 100);
	btstack_run_loop_add_timer(&btstack_qpoll);
}
// call btstack_handler_init(); before btstack loop
 */
void cpu_cores_init_from_core0();

/**
 * @brief Notifies core0 to quit loop or w/e and proceed to shutdown (call \c cpu_cores_shutdown_from_core0(); )
 */
[[noreturn]] void cpu_cores_send_shutdown_to_core0_from_core1();

/**
*	@warning Don't forget to turn off BT and WiFi stack before calling this (pico-shared doesn't include btstack, etc)
	@details
	utils_printf("wifi and bt shutdown\n");
	hci_power_control(HCI_POWER_OFF);
	cyw43_arch_deinit();
*/
void cpu_cores_shutdown_from_core0();

#endif //PICO_SHARED_CPU_CORES_H
