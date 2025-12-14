#pragma once
#include <setjmp.h>
#include "genesis.h"
#include "utils.h"
#include "cmocka.h"

void __wrap_midi_tx_send_sysex(const u8* data, u16 length);
void __wrap_midi_tx_send_cc(u8 channel, u8 controller, u8 value);

extern void __real_midi_tx_send_sysex(const u8* data, u16 length);
extern void __real_midi_tx_send_cc(u8 channel, u8 controller, u8 value);

void _expect_midi_tx_send_cc(
    u8 channel, u8 controller, u8 value, const char* const file, const int line);

#define expect_midi_tx_send_cc(channel, controller, value)                                         \
    _expect_midi_tx_send_cc(channel, controller, value, __FILE__, __LINE__)
