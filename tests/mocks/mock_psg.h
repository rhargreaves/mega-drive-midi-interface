#pragma once
#include "genesis.h"
#include "debug.h"
#include "cmocka.h"

void _expect_psg_tone(u8 channel, u16 value, const char* const file, const int line);
void _expect_psg_attenuation(u8 channel, u8 value, const char* const file, const int line);

#define expect_psg_tone(channel, value) _expect_psg_tone(channel, value, __FILE__, __LINE__)
#define expect_psg_attenuation(channel, value)                                                     \
    _expect_psg_attenuation(channel, value, __FILE__, __LINE__)
#define expect_any_psg_tone()                                                                      \
    {                                                                                              \
        debug_message("expect: PSG_setTone(channel=*, value=*)\n");                                \
        expect_any(__wrap_PSG_setTone, channel);                                                   \
        expect_any(__wrap_PSG_setTone, value);                                                     \
    }
#define expect_any_psg_tone_on_channel(c)                                                          \
    {                                                                                              \
        expect_value(__wrap_PSG_setTone, channel, c);                                              \
        expect_any(__wrap_PSG_setTone, value);                                                     \
    }
#define expect_any_psg_attenuation()                                                               \
    {                                                                                              \
        expect_any(__wrap_PSG_setEnvelope, channel);                                               \
        expect_any(__wrap_PSG_setEnvelope, value);                                                 \
    }

void __wrap_psg_note_on(u8 channel, u16 freq);
void __wrap_psg_note_off(u8 channel);
void __wrap_psg_attenuation(u8 channel, u8 attenuation);
