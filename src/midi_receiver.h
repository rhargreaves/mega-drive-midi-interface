#pragma once
#include <stdint.h>
#include <types.h>
#include <stdbool.h>

void midi_receiver_readIfCommReady(void);
void midi_receiver_read(void);
void midi_receiver_init(void);
