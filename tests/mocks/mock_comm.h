#pragma once
#include "genesis.h"
#include "cmocka.h"

void __wrap_comm_init(void);
bool __wrap_comm_read_ready(void);
u8 __wrap_comm_read(void);
void __wrap_comm_write(u8 data);
void __wrap_comm_serial_init(void);
bool __wrap_comm_serial_is_present(void);
u8 __wrap_comm_serial_read_ready(void);
u8 __wrap_comm_serial_read(void);
u8 __wrap_comm_serial_write_ready(void);
void __wrap_comm_serial_write(u8 data);
void __wrap_comm_everdrive_init(void);
bool __wrap_comm_everdrive_is_present(void);
u8 __wrap_comm_everdrive_read_ready(void);
u8 __wrap_comm_everdrive_read(void);
u8 __wrap_comm_everdrive_write_ready(void);
void __wrap_comm_everdrive_write(u8 data);
void __wrap_comm_everdrive_pro_init(void);
bool __wrap_comm_everdrive_pro_is_present(void);
u8 __wrap_comm_everdrive_pro_read_ready(void);
u8 __wrap_comm_everdrive_pro_read(void);
u8 __wrap_comm_everdrive_pro_write_ready(void);
void __wrap_comm_everdrive_pro_write(u8 data);
void __wrap_comm_demo_init(void);
bool __wrap_comm_demo_is_present(void);
u8 __wrap_comm_demo_read_ready(void);
u8 __wrap_comm_demo_read(void);
u8 __wrap_comm_demo_write_ready(void);
void __wrap_comm_demo_write(u8 data);
void __wrap_comm_demo_vsync(void);
void __wrap_comm_megawifi_init(void);
void __wrap_comm_megawifi_midiEmitCallback(u8 midiByte);
void __wrap_comm_megawifi_tick(void);
void __wrap_comm_megawifi_send(u8 ch, char* data, u16 len);
bool __wrap_comm_megawifi_is_present(void);
u8 __wrap_comm_megawifi_read_ready(void);
u8 __wrap_comm_megawifi_read(void);
u8 __wrap_comm_megawifi_write_ready(void);
void __wrap_comm_megawifi_write(u8 data);
bool __wrap_mw_uart_is_present(void);
extern void __real_comm_megawifi_midiEmitCallback(u8 midiByte);
extern void __real_comm_megawifi_init(void);
extern void __real_comm_megawifi_tick(void);
extern bool __real_comm_read_ready(void);
extern void __real_comm_demo_init(void);
extern u8 __real_comm_demo_read_ready(void);
extern u8 __real_comm_demo_read(void);
extern u8 __real_comm_demo_write_ready(void);
extern void __real_comm_demo_write(u8 data);
extern void __real_comm_demo_vsync(void);
extern bool __real_comm_megawifi_is_present(void);
extern u8 __real_comm_megawifi_read_ready(void);
extern u8 __real_comm_megawifi_read(void);
extern u8 __real_comm_megawifi_write_ready(void);
extern void __real_comm_megawifi_write(u8 data);
extern void __real_comm_init(void);
extern void __real_comm_write(u8 data);
extern u8 __real_comm_read(void);
extern u16 __real_comm_idle_count(void);
extern u16 __real_comm_busy_count(void);
extern void __real_comm_reset_counts(void);

#define stub_megawifi_as_not_present() _stub_megawifi_as_not_present(__FILE__, __LINE__)
#define stub_everdrive_as_present() _stub_everdrive_as_present(__FILE__, __LINE__)
#define expect_usb_sent_byte(value) _expect_usb_sent_byte(value, __FILE__, __LINE__)
#define stub_usb_receive_nothing() _stub_usb_receive_nothing(__FILE__, __LINE__)
#define stub_usb_receive_byte(value) _stub_usb_receive_byte(value, __FILE__, __LINE__)
#define stub_usb_receive_program(chan, program)                                                    \
    _stub_usb_receive_program(chan, program, __FILE__, __LINE__)
#define stub_usb_receive_cc(chan, cc, value)                                                       \
    _stub_usb_receive_cc(chan, cc, value, __FILE__, __LINE__)
#define stub_usb_receive_note_on(chan, key, velocity)                                              \
    _stub_usb_receive_note_on(chan, key, velocity, __FILE__, __LINE__)
#define stub_usb_receive_note_off(chan, key)                                                       \
    _stub_usb_receive_note_off(chan, key, __FILE__, __LINE__)
#define stub_usb_receive_pitch_bend(chan, bend)                                                    \
    _stub_usb_receive_pitch_bend(chan, bend, __FILE__, __LINE__)
#define stub_comm_read_returns_midi_event(status, data, data2)                                     \
    _stub_comm_read_returns_midi_event(status, data, data2, __FILE__, __LINE__)
#define stub_comm_read_returns_cc(chan, cc, value)                                                 \
    _stub_comm_read_returns_cc(chan, cc, value, __FILE__, __LINE__)

void _stub_megawifi_as_not_present(const char* file, const int line);
void _stub_everdrive_as_present(const char* file, const int line);
void _expect_usb_sent_byte(u8 value, const char* file, const int line);
void _stub_usb_receive_nothing(const char* file, const int line);
void _stub_usb_receive_byte(u8 value, const char* file, const int line);
void _stub_usb_receive_program(u8 chan, u8 program, const char* file, const int line);
void _stub_usb_receive_cc(u8 chan, u8 cc, u8 value, const char* file, const int line);
void _stub_usb_receive_note_on(u8 chan, u8 key, u8 velocity, const char* file, const int line);
void _stub_usb_receive_note_off(u8 chan, u8 key, const char* file, const int line);
void _stub_usb_receive_pitch_bend(u8 chan, u16 bend, const char* file, const int line);
void _stub_comm_read_returns_midi_event(
    u8 status, u8 data, u8 data2, const char* file, const int line);
void _stub_comm_read_returns_cc(u8 chan, u8 cc, u8 value, const char* file, const int line);