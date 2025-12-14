#include "mock_midi_tx.h"
#include "cmocka_inc.h"

void __wrap_midi_tx_send_sysex(const u8* data, u16 length)
{
    check_expected_ptr(data);
    check_expected(length);
}

void __wrap_midi_tx_send_cc(u8 channel, u8 controller, u8 value)
{
    print_message("expect_midi_tx_send_cc: %d, %d, %d\n", channel, controller, value);

    check_expected(channel);
    check_expected(controller);
    check_expected(value);
}

void _expect_midi_tx_send_cc(
    u8 channel, u8 controller, u8 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_midi_tx_send_cc, channel, channel, file, line);
    expect_value_with_pos(__wrap_midi_tx_send_cc, controller, controller, file, line);
    expect_value_with_pos(__wrap_midi_tx_send_cc, value, value, file, line);
}
