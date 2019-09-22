#pragma once
#include "midi_fm.h"
#include "midi_psg.h"
#include <stdbool.h>
#include <types.h>

#define MAX_MIDI_VOLUME 127
#define MAX_MIDI_CHANS 9
#define MIDI_CHANNELS 16

#define CC_VOLUME 7
#define CC_PAN 10
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
#define CC_GENMDM_RATE_SCALING_OP1 39
#define CC_GENMDM_RATE_SCALING_OP2 40
#define CC_GENMDM_RATE_SCALING_OP3 41
#define CC_GENMDM_RATE_SCALING_OP4 42
#define CC_GENMDM_ATTACK_RATE_OP1 43
#define CC_GENMDM_ATTACK_RATE_OP2 44
#define CC_GENMDM_ATTACK_RATE_OP3 45
#define CC_GENMDM_ATTACK_RATE_OP4 46
#define CC_GENMDM_FIRST_DECAY_RATE_OP1 47
#define CC_GENMDM_FIRST_DECAY_RATE_OP2 48
#define CC_GENMDM_FIRST_DECAY_RATE_OP3 49
#define CC_GENMDM_FIRST_DECAY_RATE_OP4 50
#define CC_GENMDM_SECOND_DECAY_RATE_OP1 51
#define CC_GENMDM_SECOND_DECAY_RATE_OP2 52
#define CC_GENMDM_SECOND_DECAY_RATE_OP3 53
#define CC_GENMDM_SECOND_DECAY_RATE_OP4 54
#define CC_GENMDM_SECOND_AMPLITUDE_OP1 55
#define CC_GENMDM_SECOND_AMPLITUDE_OP2 56
#define CC_GENMDM_SECOND_AMPLITUDE_OP3 57
#define CC_GENMDM_SECOND_AMPLITUDE_OP4 58
#define CC_GENMDM_RELEASE_RATE_OP1 59
#define CC_GENMDM_RELEASE_RATE_OP2 60
#define CC_GENMDM_RELEASE_RATE_OP3 61
#define CC_GENMDM_RELEASE_RATE_OP4 62
#define CC_GENMDM_AMPLITUDE_MODULATION_OP1 70
#define CC_GENMDM_AMPLITUDE_MODULATION_OP2 71
#define CC_GENMDM_AMPLITUDE_MODULATION_OP3 72
#define CC_GENMDM_AMPLITUDE_MODULATION_OP4 73
#define CC_GENMDM_GLOBAL_LFO_ENABLE 74
#define CC_GENMDM_FMS 75
#define CC_GENMDM_AMS 76
#define CC_GENMDM_STEREO 77
#define CC_POLYPHONIC_MODE 80
#define CC_GENMDM_SSG_EG_OP1 90
#define CC_GENMDM_SSG_EG_OP2 91
#define CC_GENMDM_SSG_EG_OP3 92
#define CC_GENMDM_SSG_EG_OP4 93
#define CC_GENMDM_GLOBAL_LFO_FREQUENCY 1
#define CC_ALL_NOTES_OFF 123

typedef struct ControlChange ControlChange;

struct ControlChange {
    u8 controller;
    u8 value;
};

typedef struct Timing Timing;

struct Timing {
    u8 clock;
    u16 bar;
    u8 barBeat;
    u8 sixteenth;
    u16 clocks;
};

void midi_init(
    Channel** defaultPresets, PercussionPreset** defaultPercussionPresets);
void midi_noteOn(u8 chan, u8 pitch, u8 velocity);
void midi_noteOff(u8 chan, u8 pitch);
void midi_pitchBend(u8 chan, u16 bend);
bool midi_getPolyphonic(void);
ControlChange* midi_lastUnknownCC(void);
void midi_cc(u8 chan, u8 controller, u8 value);
bool midi_overflow(void);
void midi_clock(void);
void midi_start(void);
void midi_position(u16 beat);
void midi_program(u8 chan, u8 program);
Timing* midi_timing(void);
void midi_sysex(const u8* data, u16 length);
