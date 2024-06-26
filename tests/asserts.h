#pragma once
#include <stdint.h>
#include <types.h>

#include <cmocka.h>

void expect_usb_sent_byte(u8 value);
void stub_usb_receive_byte(u8 value);
void stub_comm_read_returns_midi_event(u8 status, u8 data, u8 data2);
void expect_ym2612_write_reg_any_data(u8 part, u8 reg);
void expect_ym2612_write_operator_any_data(u8 chan, u8 op, u8 baseReg);
void expect_ym2612_write_channel_any_data(u8 chan, u8 baseReg);
void expect_synth_pitch_any(void);
void expect_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void expect_synth_volume_any(void);
void expect_synth_volume(u8 channel, u8 volume);
u8 regOpIndex(u8 op);

void _expect_ym2612_write_reg(
    u8 part, u8 reg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_channel(
    u8 chan, u8 baseReg, u8 data, const char* const file, const int line);
void _expect_ym2612_write_operator(u8 chan, u8 op, u8 baseReg, u8 data,
    const char* const file, const int line);

#define REG_PART(chan) (chan < 3 ? 0 : 1)
#define REG_OFFSET(chan) (chan % 3)

#define expect_log_info(f)                                                     \
    {                                                                          \
        expect_memory(__wrap_log_info, fmt, f, sizeof(f));                     \
    }

#define expect_log_warn(f)                                                     \
    {                                                                          \
        expect_memory(__wrap_log_warn, fmt, f, sizeof(f));                     \
    }

#define expect_psg_attenuation(chan, v)                                        \
    {                                                                          \
        expect_value(__wrap_PSG_setEnvelope, channel, chan);                   \
        expect_value(__wrap_PSG_setEnvelope, value, v);                        \
    }

#define expect_psg_tone(c, t)                                                  \
    {                                                                          \
        expect_value(__wrap_PSG_setTone, channel, c);                          \
        expect_value(__wrap_PSG_setTone, value, t);                            \
    }

#define expect_any_psg_attenuation()                                           \
    {                                                                          \
        expect_any(__wrap_PSG_setEnvelope, channel);                           \
        expect_any(__wrap_PSG_setEnvelope, value);                             \
    }

#define expect_any_psg_tone()                                                  \
    {                                                                          \
        expect_any(__wrap_PSG_setTone, channel);                               \
        expect_any(__wrap_PSG_setTone, value);                                 \
    }

#define expect_any_psg_tone_on_channel(c)                                      \
    {                                                                          \
        expect_value(__wrap_PSG_setTone, channel, c);                          \
        expect_any(__wrap_PSG_setTone, value);                                 \
    }

#define expect_midi_emit(mb)                                                   \
    {                                                                          \
        expect_value(__wrap_comm_megawifi_midiEmitCallback, midiByte, mb);     \
    }

#define expect_midi_emit_trio(mb1, mb2, mb3)                                   \
    {                                                                          \
        expect_value(__wrap_comm_megawifi_midiEmitCallback, midiByte, mb1);    \
        expect_value(__wrap_comm_megawifi_midiEmitCallback, midiByte, mb2);    \
        expect_value(__wrap_comm_megawifi_midiEmitCallback, midiByte, mb3);    \
    }

#define expect_midi_emit_duo(mb1, mb2)                                         \
    {                                                                          \
        expect_value(__wrap_comm_megawifi_midiEmitCallback, midiByte, mb1);    \
        expect_value(__wrap_comm_megawifi_midiEmitCallback, midiByte, mb2);    \
    }

#define expect_ym2612_write_operator(chan, op, baseReg, data)                  \
    _expect_ym2612_write_operator(chan, op, baseReg, data, __FILE__, __LINE__)

#define expect_ym2612_write_reg(part, reg, data)                               \
    _expect_ym2612_write_reg(part, reg, data, __FILE__, __LINE__)

#define expect_ym2612_write_channel(chan, baseReg, data)                       \
    _expect_ym2612_write_channel(chan, baseReg, data, __FILE__, __LINE__)
