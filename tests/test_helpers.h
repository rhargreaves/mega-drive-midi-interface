#pragma once

// --- MIDI ---
#define POLYPHONIC_ON 0x7F
#define SPECIAL_MODE_ON 64
#define DEVICE_SELECT_PSG 64

#define MIDI_CHANNEL_PSG_1 6
#define MIDI_CHANNEL_1 0
#define MIDI_CHANNEL_3 2
#define MIDI_CHANNEL_6 5
#define MIDI_CHANNEL_7 6
#define MIDI_CHANNEL_11 10

#define MIDI_VOLUME_MAX 127
#define MIDI_VELOCITY_MAX 127

// increments on C
#define MIDI_PITCH_A2 45
#define MIDI_PITCH_C3 48
#define MIDI_PITCH_CS3 49
#define MIDI_PITCH_D3 50
#define MIDI_PITCH_AS3 58
#define MIDI_PITCH_B3 59
#define MIDI_PITCH_C4 60
#define MIDI_PITCH_CS4 61
#define MIDI_PITCH_AS4 70
#define MIDI_PITCH_B4 71
#define MIDI_PITCH_C5 72
#define MIDI_PITCH_CS5 73
#define MIDI_PITCH_AS6 94
#define MIDI_PITCH_B6 95

#define FREQBLK_A2 0x1439
#define FREQBLK_B2 0x1A5F
#define FREQBLK_C3 0x1A84
#define FREQBLK_CS3 0x1AA9
#define FREQBLK_D3 0x1AD2

#define SYSEX_DYNAMIC_AUTO 0x02
#define SYSEX_DYNAMIC_ENABLED 0x01
#define SYSEX_DYNAMIC_DISABLED 0x00
#define SYSEX_NON_GENERAL_MIDI_CCS_ENABLED 0x01
#define SYSEX_NON_GENERAL_MIDI_CCS_DISABLED 0x00

#define UNASSIGNED_MIDI_CHANNEL 0x7F
#define NULL_RPN_MSB 0x7F
#define NULL_RPN_LSB 0x7F

// --- YM2612 ---
#define YM_OP_REG_INDEX(op) ((op) == 1 ? 2 : ((op) == 2 ? 1 : (op)))
#define YM_REG(baseReg, channel) (baseReg + (channel % 3))
#define YM_REG2(baseReg, channel, op) (baseReg + (channel % 3) + (YM_OP_REG_INDEX(op) * 4))
#define YM_REG3(baseReg, op) (baseReg + (YM_OP_REG_INDEX(op) * 4))
#define YM_REG_CH3SM(baseReg, op) (baseReg + ((op + 1) % 3))

#define YM_CH3SM_OP1_FREQ_LSB 0xA9
#define YM_CH3SM_OP1_FREQ_MSB_BLK 0xAD
#define YM_CH3SM_OP2_FREQ_LSB 0xA8
#define YM_CH3SM_OP2_FREQ_MSB_BLK 0xAC
#define YM_CH3SM_OP3_FREQ_LSB 0xAA
#define YM_CH3SM_OP3_FREQ_MSB_BLK 0xAE
#define YM_CH3SM_OP4_FREQ_LSB 0xA2
#define YM_CH3SM_OP4_FREQ_MSB_BLK 0xA6

#define YM_TOTAL_LEVEL_LOUDEST 0
#define YM_TOTAL_LEVEL_SILENCE 0x7F

#define YM_CH1 0
#define YM_CH2 1
#define YM_CH3 2

#define YM_OP1 0
#define YM_OP2 1
#define YM_OP3 2
#define YM_OP4 3

// --- Synth ---
#define FM_CH1 0
#define FM_CH2 1
#define FM_CH3 2
#define FM_CH4 3
#define FM_CH5 4
#define FM_CH6 5

#define SYNTH_NTSC_B 607 // 0x25F
#define SYNTH_NTSC_C 644 // 0x284
#define SYNTH_NTSC_CS 681 // 0x2A9
#define SYNTH_NTSC_D 722 // 0x2D2
#define SYNTH_NTSC_DS 765 // 0x2FD
#define SYNTH_NTSC_E 810 // 0x32A
#define SYNTH_NTSC_F 858 // 0x35A
#define SYNTH_NTSC_FS 910 // 0x38A
#define SYNTH_NTSC_G 964 // 0x3C4
#define SYNTH_NTSC_GS 1021 // 0x3FD
#define SYNTH_NTSC_A 1081 // 0x439
#define SYNTH_NTSC_AS 1146 // 0x47A

// --- PSG ---
#define PSG_CH1 0
#define PSG_CH2 1
#define PSG_CH3 2
#define PSG_NOISE_CH4 3

#define TONE_NTSC_AS3 479
#define TONE_NTSC_C4 427
#define TONE_NTSC_CS4 403
#define TONE_NTSC_DS4 380
#define TONE_NTSC_A2 1016