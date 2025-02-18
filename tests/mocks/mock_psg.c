#include "cmocka_inc.h"
#include "asserts.h"
#include "mocks/mock_psg.h"

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
