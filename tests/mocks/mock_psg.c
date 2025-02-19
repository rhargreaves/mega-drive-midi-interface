#include "cmocka_inc.h"
#include "mocks/mock_comm.h"
#include "mocks/mock_psg.h"

static bool disableChecks = false;

void mock_psg_disable_checks(void)
{
    disableChecks = true;
}

void mock_psg_enable_checks(void)
{
    disableChecks = false;
}

void __wrap_PSG_setEnvelope(u8 channel, u8 value)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(value);
}

void __wrap_PSG_setTone(u8 channel, u16 value)
{
    debug_message("call: PSG_setTone(%d, %d)\n", channel, value);
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(value);
}

void _expect_psg_tone(u8 channel, u16 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_PSG_setTone, channel, channel, file, line);
    expect_value_with_pos(__wrap_PSG_setTone, value, value, file, line);
}

void _expect_psg_attenuation(u8 channel, u8 value, const char* const file, const int line)
{
    expect_value_with_pos(__wrap_PSG_setEnvelope, channel, channel, file, line);
    expect_value_with_pos(__wrap_PSG_setEnvelope, value, value, file, line);
}

void __wrap_psg_note_on(u8 channel, u16 freq)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(freq);
}

void __wrap_psg_note_off(u8 channel)
{
    if (disableChecks)
        return;
    check_expected(channel);
}

void __wrap_psg_attenuation(u8 channel, u8 attenuation)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(attenuation);
}
