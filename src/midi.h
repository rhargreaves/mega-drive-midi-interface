#pragma once
#include "genesis.h"
#include "midi_fm.h"
#include "midi_psg.h"

#define MIDI_PROGRAMS 128
#define MAX_MIDI_VOLUME 127
#define DEFAULT_MIDI_PAN 64
#define MIDI_PITCH_BEND_MIN 0
#define MIDI_PITCH_BEND_CENTRE 0x2000
#define MIDI_PITCH_BEND_MAX 0x4000
#define DEFAULT_MIDI_PITCH_BEND MIDI_PITCH_BEND_CENTRE
#define GENERAL_MIDI_PITCH_BEND_SEMITONE_RANGE 2
#define UNASSIGNED_MIDI_CHANNEL 0x7F
#define NULL_RPN 0x7F7F

#define MIDI_CHANNELS 16
#define PSG_NOISE_CHAN 3
#define GENERAL_MIDI_PERCUSSION_CHANNEL 9

#define DEV_CHANS 13
#define DEV_PHYSICAL_CHANS 10
#define DEV_CHAN_MIN_FM 0
#define DEV_CHAN_MAX_FM 5
#define DEV_CHAN_MIN_PSG 6
#define DEV_CHAN_MAX_TONE_PSG 8
#define DEV_CHAN_PSG_NOISE 9
#define DEV_CHAN_MAX_PSG 9
#define DEV_CHAN_MIN_SPECIAL_MODE 10
#define DEV_CHAN_MAX_SPECIAL_MODE 12

#define RPN_PITCH_BEND_SENSITIVITY_MSB 0
#define RPN_PITCH_BEND_SENSITIVITY_LSB 0
#define RPN_PITCH_BEND_SENSITIVITY 0

#define CC_PORTAMENTO_TIME_MSB 5
#define CC_DATA_ENTRY_MSB 6
#define CC_VOLUME 7
#define CC_PAN 10
#define CC_EXPRESSION 11
#define CC_GENMDM_FM_ALGORITHM 14
#define CC_GENMDM_FM_FEEDBACK 15
#define CC_GENMDM_TOTAL_LEVEL_OP1 16
#define CC_GENMDM_TOTAL_LEVEL_OP2 17
#define CC_GENMDM_TOTAL_LEVEL_OP3 18
#define CC_GENMDM_TOTAL_LEVEL_OP4 19
#define CC_GENMDM_MULTIPLE_OP1 20
#define CC_GENMDM_MULTIPLE_OP2 21
#define CC_GENMDM_MULTIPLE_OP3 22
#define CC_GENMDM_MULTIPLE_OP4 23
#define CC_GENMDM_DETUNE_OP1 24
#define CC_GENMDM_DETUNE_OP2 25
#define CC_GENMDM_DETUNE_OP3 26
#define CC_GENMDM_DETUNE_OP4 27
#define CC_PORTAMENTO_TIME_LSB 37
#define CC_DATA_ENTRY_LSB 38
#define CC_GENMDM_RATE_SCALING_OP1 39
#define CC_GENMDM_RATE_SCALING_OP2 40
#define CC_GENMDM_RATE_SCALING_OP3 41
#define CC_GENMDM_RATE_SCALING_OP4 42
#define CC_GENMDM_ATTACK_RATE_OP1 43
#define CC_GENMDM_ATTACK_RATE_OP2 44
#define CC_GENMDM_ATTACK_RATE_OP3 45
#define CC_GENMDM_ATTACK_RATE_OP4 46
#define CC_GENMDM_DECAY_RATE_OP1 47
#define CC_GENMDM_DECAY_RATE_OP2 48
#define CC_GENMDM_DECAY_RATE_OP3 49
#define CC_GENMDM_DECAY_RATE_OP4 50
#define CC_GENMDM_SUSTAIN_RATE_OP1 51
#define CC_GENMDM_SUSTAIN_RATE_OP2 52
#define CC_GENMDM_SUSTAIN_RATE_OP3 53
#define CC_GENMDM_SUSTAIN_RATE_OP4 54
#define CC_GENMDM_SUSTAIN_LEVEL_OP1 55
#define CC_GENMDM_SUSTAIN_LEVEL_OP2 56
#define CC_GENMDM_SUSTAIN_LEVEL_OP3 57
#define CC_GENMDM_SUSTAIN_LEVEL_OP4 58
#define CC_GENMDM_RELEASE_RATE_OP1 59
#define CC_GENMDM_RELEASE_RATE_OP2 60
#define CC_GENMDM_RELEASE_RATE_OP3 61
#define CC_GENMDM_RELEASE_RATE_OP4 62
#define CC_SUSTAIN_PEDAL 64
#define CC_PORTAMENTO_ENABLE 65
#define CC_GENMDM_AMPLITUDE_MODULATION_OP1 70
#define CC_GENMDM_AMPLITUDE_MODULATION_OP2 71
#define CC_GENMDM_AMPLITUDE_MODULATION_OP3 72
#define CC_GENMDM_AMPLITUDE_MODULATION_OP4 73
#define CC_GENMDM_GLOBAL_LFO_ENABLE 74
#define CC_GENMDM_FMS 75
#define CC_GENMDM_AMS 76
#define CC_GENMDM_STEREO 77
#define CC_GENMDM_ENABLE_DAC 78
#define CC_GENMDM_CH3_SPECIAL_MODE 80
#define CC_GENMDM_POLYPHONIC_MODE 84
#define CC_FINE_TUNE 85
#define CC_DEVICE_SELECT 86
#define CC_STORE_PROGRAM 87
#define CC_SHOW_PARAMETERS_ON_UI 83
#define CC_GENMDM_SSG_EG_OP1 90
#define CC_GENMDM_SSG_EG_OP2 91
#define CC_GENMDM_SSG_EG_OP3 92
#define CC_GENMDM_SSG_EG_OP4 93
#define CC_NRPN_LSB 98
#define CC_NRPN_MSB 99
#define CC_RPN_LSB 100
#define CC_RPN_MSB 101
#define CC_GENMDM_GLOBAL_LFO_FREQUENCY 1
#define CC_RESET_ALL_CONTROLLERS 121
#define CC_ALL_SOUND_OFF 120
#define CC_ALL_NOTES_OFF 123

#define SYSEX_START 0xF0
#define SYSEX_END 0xF7

#define SYSEX_MANU_EXTENDED 0x00
#define SYSEX_MANU_REGION 0x22
#define SYSEX_MANU_ID 0x77

#define SYSEX_COMMAND_REMAP 0x00
#define SYSEX_COMMAND_PING 0x01
#define SYSEX_COMMAND_PONG 0x02
#define SYSEX_COMMAND_DYNAMIC 0x03
#define SYSEX_COMMAND_NON_GENERAL_MIDI_CCS 0x04
#define SYSEX_COMMAND_STICK_TO_DEVICE_TYPE 0x05
#define SYSEX_COMMAND_LOAD_PSG_ENVELOPE 0x06
#define SYSEX_COMMAND_INVERT_TOTAL_LEVEL 0x07
#define SYSEX_COMMAND_WRITE_YM2612_REG_PART_0 0x08
#define SYSEX_COMMAND_WRITE_YM2612_REG_PART_1 0x09
#define SYSEX_COMMAND_STORE_PROGRAM 0x0A
#define SYSEX_COMMAND_CLEAR_PROGRAM 0x0B
#define SYSEX_COMMAND_CLEAR_ALL_PROGRAMS 0x0C
#define SYSEX_COMMAND_DUMP_PRESET 0x0D
#define SYSEX_COMMAND_PRESET_DATA 0x0E
#define SYSEX_COMMAND_DUMP_CHANNEL 0x0F
#define SYSEX_COMMAND_CHANNEL_DATA 0x10

#define STORE_PROGRAM_TYPE_FM 0x00
#define STORE_PROGRAM_TYPE_PSG 0x01

typedef struct VTable {
    void (*noteOn)(u8 chan, u8 pitch, s8 cents, u8 velocity);
    void (*noteOff)(u8 chan, u8 pitch);
    void (*channelVolume)(u8 chan, u8 volume);
    void (*program)(u8 chan, u8 program);
    void (*allNotesOff)(u8 chan);
    void (*pan)(u8 chan, u8 pan);
    void (*pitch)(u8 chan, u8 pitch, s8 cents);
} VTable;

typedef struct DeviceChannel {
    u8 num;
    const VTable* ops;
    bool noteOn;
    u8 midiChannel;
    u8 program;
    u8 pitch;
    u8 volume;
    u8 pan;
    u16 pitchBend;
    s8 cents;
    u16 glideTargetPitch;
} DeviceChannel;

typedef enum MidiChangeType {
    MidiChangeType_None,
    MidiChangeType_Program,
    MidiChangeType_CC
} MidiChangeType;

typedef struct MidiChangeEvent {
    MidiChangeType type;
    u8 chan;
    u8 key;
    u8 value;
} MidiChangeEvent;

void midi_init(const FmPreset** defaultPresets, const PercussionPreset** defaultPercussionPresets,
    const u8** defaultEnvelopes);
void midi_note_on(u8 chan, u8 pitch, u8 velocity);
void midi_note_off(u8 chan, u8 pitch);
void midi_pitch_bend(u8 chan, u16 bend);
void midi_cc(u8 chan, u8 controller, u8 value);
void midi_program(u8 chan, u8 program);
void midi_sysex(const u8* data, u16 length);
bool midi_dynamic_mode(void);
DeviceChannel* midi_channel_mappings(void);
void midi_remap_channel(u8 midiChannel, u8 deviceChannel);
void midi_reset(void);
void midi_tick(u16 delta);
void midi_register_change_callback(void (*callback)(MidiChangeEvent));