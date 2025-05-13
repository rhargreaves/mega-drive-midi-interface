#pragma once
#include "genesis.h"

void midi_receiver_read_if_comm_ready(void);
void midi_receiver_read_once();
void midi_receiver_init(void);
void midi_receiver_run_startup_sequence(void);