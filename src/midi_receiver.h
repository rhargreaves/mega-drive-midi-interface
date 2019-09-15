#pragma once
#include <stdbool.h>
#include <types.h>

void midi_receiver_perpectual_read(void);
void midi_receiver_read(void);
void midi_receiver_init(void);
u8 midi_receiver_lastUnknownStatus(void);
