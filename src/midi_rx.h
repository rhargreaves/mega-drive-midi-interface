#pragma once
#include "genesis.h"

void midi_rx_read_if_comm_ready(void);
void midi_rx_read(void);
void midi_rx_init(void);
void midi_rx_run_startup_sequence(void);