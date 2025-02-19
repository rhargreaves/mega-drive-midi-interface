#pragma once
#include "genesis.h"
#include "cmocka.h"

void stub_megawifi_as_not_present(void);
void stub_everdrive_as_present(void);
void expect_usb_sent_byte(u8 value);
void stub_usb_receive_nothing(void);
void stub_usb_receive_byte(u8 value);
void stub_usb_receive_program(u8 chan, u8 program);
void stub_usb_receive_cc(u8 chan, u8 cc, u8 value);
void stub_usb_receive_note_on(u8 chan, u8 key, u8 velocity);
void stub_usb_receive_note_off(u8 chan, u8 key);
void stub_usb_receive_pitch_bend(u8 chan, u16 bend);
void stub_comm_read_returns_midi_event(u8 status, u8 data, u8 data2);
