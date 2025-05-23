#include "cmocka_inc.h"
#include "mocks/mock_comm.h"
#include "comm/comm.h"

void __wrap_comm_init(void)
{
}

void __wrap_comm_write(u8 data)
{
    check_expected(data);
}

bool __wrap_comm_read_ready(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_read(void)
{
    return mock_type(u8);
}

u16 __wrap_comm_idle_count(void)
{
    return mock_type(u16);
}

u16 __wrap_comm_busy_count(void)
{
    return mock_type(u16);
}

void __wrap_comm_reset_counts(void)
{
}

void __wrap_comm_megawifi_init(void)
{
}

void __wrap_comm_serial_init(void)
{
}

bool __wrap_comm_serial_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_serial_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_serial_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_serial_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_serial_write(u8 data)
{
    check_expected(data);
}

void __wrap_comm_everdrive_init(void)
{
}

bool __wrap_comm_everdrive_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_everdrive_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_everdrive_write(u8 data)
{
    check_expected(data);
}

void __wrap_comm_everdrive_pro_init(void)
{
}

bool __wrap_comm_everdrive_pro_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_everdrive_pro_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_pro_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_pro_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_everdrive_pro_write(u8 data)
{
    check_expected(data);
}

void __wrap_comm_demo_init(void)
{
}

bool __wrap_comm_demo_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_demo_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_demo_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_demo_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_demo_write(u8 data)
{
}

void __wrap_comm_demo_vsync(void)
{
    function_called();
}

bool __wrap_comm_megawifi_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_megawifi_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_megawifi_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_megawifi_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_megawifi_write(u8 data)
{
    check_expected(data);
}

void __wrap_comm_megawifi_tick(void)
{
    function_called();
}

void __wrap_comm_megawifi_send(u8 ch, char* data, u16 len)
{
    check_expected(ch);
    check_expected(data);
    check_expected(len);
}

void __wrap_comm_megawifi_midiEmitCallback(u8 midiByte)
{
    check_expected(midiByte);
}

bool __wrap_mw_uart_is_present(void)
{
    return mock_type(bool);
}

void _stub_megawifi_as_not_present(const char* file, const int line)
{
    will_return_with_pos(__wrap_mw_uart_is_present, false, file, line);
}

void _stub_everdrive_as_present(const char* file, const int line)
{
    will_return_with_pos(__wrap_comm_everdrive_is_present, true, file, line);
}

void _stub_usb_receive_nothing(const char* file, const int line)
{
    will_return_with_pos(__wrap_comm_everdrive_read_ready, 0, file, line);
}

void _stub_usb_receive_byte(u8 value, const char* file, const int line)
{
    will_return_with_pos(__wrap_comm_everdrive_read_ready, 1, file, line);
    will_return_with_pos(__wrap_comm_everdrive_read, value, file, line);
}

void _stub_usb_receive_program(u8 chan, u8 program, const char* file, const int line)
{
    _stub_usb_receive_byte(0xC0 + chan, file, line);
    _stub_usb_receive_byte(program, file, line);
}

void _stub_usb_receive_cc(u8 chan, u8 cc, u8 value, const char* file, const int line)
{
    _stub_usb_receive_byte(0xB0 + chan, file, line);
    _stub_usb_receive_byte(cc, file, line);
    _stub_usb_receive_byte(value, file, line);
}

void _stub_usb_receive_note_on(u8 chan, u8 key, u8 velocity, const char* file, const int line)
{
    _stub_usb_receive_byte(0x90 + chan, file, line);
    _stub_usb_receive_byte(key, file, line);
    _stub_usb_receive_byte(velocity, file, line);
}

void _stub_usb_receive_note_off(u8 chan, u8 key, const char* file, const int line)
{
    _stub_usb_receive_byte(0x80 + chan, file, line);
    _stub_usb_receive_byte(key, file, line);
    _stub_usb_receive_byte(0, file, line);
}

void _stub_usb_receive_pitch_bend(u8 chan, u16 bend, const char* file, const int line)
{
    u8 lower = bend & 0x007F;
    u8 upper = bend >> 7;

    _stub_usb_receive_byte(0xE0 + chan, file, line);
    _stub_usb_receive_byte(lower, file, line);
    _stub_usb_receive_byte(upper, file, line);
}

void _expect_usb_sent_byte(u8 value, const char* file, const int line)
{
    will_return_with_pos(__wrap_comm_everdrive_write_ready, 1, file, line);
    expect_value_with_pos(__wrap_comm_everdrive_write, data, value, file, line);
}

void _stub_comm_read_returns_midi_event(
    u8 status, u8 data, u8 data2, const char* file, const int line)
{
    will_return_with_pos(__wrap_comm_read, status, file, line);
    will_return_with_pos(__wrap_comm_read, data, file, line);
    will_return_with_pos(__wrap_comm_read, data2, file, line);
}

void _stub_comm_read_returns_cc(u8 chan, u8 cc, u8 value, const char* file, const int line)
{
    will_return_with_pos(__wrap_comm_read, 0xB0 + chan, file, line);
    will_return_with_pos(__wrap_comm_read, cc, file, line);
    will_return_with_pos(__wrap_comm_read, value, file, line);
}
