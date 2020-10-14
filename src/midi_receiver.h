#pragma once
#include <stdint.h>
#include <types.h>

#include <stdbool.h>

void midi_receiver_perpectual_read(void);
void midi_receiver_read(void);
void midi_receiver_init(void);
u8 midi_receiver_lastUnknownStatus(void);
