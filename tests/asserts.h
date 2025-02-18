#pragma once
#include "genesis.h"
#include "debug.h"
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

void expect_memory_with_pos(const char* const function, const char* const parameter,
    const void* const value, const size_t size, const char* const file, const int line);

#define expect_value_with_pos(function, parameter, value, file, line)                              \
    _expect_value(#function, #parameter, file, line, cast_to_largest_integral_type(value), 1)
#define expect_any_with_pos(function, parameter, file, line)                                       \
    _expect_any(#function, #parameter, file, line, 1)
#define will_return_with_pos(function, value, file, line)                                          \
    _will_return(#function, file, line, cast_to_largest_integral_type(value), 1)

// ---- synth ----

// ---- log ----
#define expect_log_info(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_info, fmt, f, sizeof(f));                                         \
    }
#define expect_log_warn(f)                                                                         \
    {                                                                                              \
        expect_memory(__wrap_log_warn, fmt, f, sizeof(f));                                         \
    }
