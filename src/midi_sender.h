#pragma once
#include <stdint.h>
#include <types.h>

void midi_sender_send_sysex(const u8* data, u16 length);
