// Copyright (C) 2024 Laurynas 'Deviltry' Ekekeke
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MPC_H
#define MPC_H

#include "shared_config.h"

void mcp_init();

void mcp_cfg_set_pin_out_mode(u8 data, bool is_out);

void mcp_cfg_set_pull_up(u8 pinData, bool pull_up);

void mcp_set_out(u8 pinData, bool out);

bool mcp_is_pin_low(u8 pinData);

#endif //MPC_H
