#include "midi_fm.h"
#include "midi.h"
#include "synth.h"

static const u8 MIN_MIDI_PITCH = 11;
static const u8 MAX_MIDI_PITCH = 106;
static const u8 SEMITONES = 12;
static const u16 FREQ_NUMBERS[] = {
    617, // B
    653, 692, 733, 777, 823, 872, 924, 979, 1037, 1099,
    1164 // A#
};

typedef struct FmChannelState FmChannelState;

struct FmChannelState {
    u8 pitch;
    u8 volume;
    u8 velocity;
};

static FmChannelState FmChannelStates[MAX_FM_CHANS];

static u8 octave(u8 pitch);
static u16 freqNumber(u8 pitch);
static u8 pitchIsOutOfRange(u8 pitch);
static u8 effectiveVolume(FmChannelState* channelState);

static Channel** presets;

void midi_fm_init(Channel** defaultPresets)
{
    presets = defaultPresets;
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        FmChannelState* state = &FmChannelStates[chan];
        state->volume = MAX_MIDI_VOLUME;
        state->velocity = MAX_MIDI_VOLUME;
    }
    synth_init(presets[0]);
}

void midi_fm_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (pitchIsOutOfRange(pitch)) {
        return;
    }
    FmChannelState* state = &FmChannelStates[chan];
    state->velocity = velocity;
    synth_volume(chan, effectiveVolume(state));
    state->pitch = pitch;
    synth_pitch(chan, octave(pitch), freqNumber(pitch));
    synth_noteOn(chan);
}

void midi_fm_noteOff(u8 chan, u8 pitch)
{
    synth_noteOff(chan);
}

void midi_fm_channelVolume(u8 chan, u8 volume)
{
    FmChannelState* state = &FmChannelStates[chan];
    state->volume = volume;
    synth_volume(chan, effectiveVolume(state));
}

void midi_fm_pitchBend(u8 chan, u16 bend)
{
    FmChannelState* state = &FmChannelStates[chan];
    u16 freq = freqNumber(state->pitch);
    s16 bendRelative = bend - 0x2000;
    freq = freq + (bendRelative / 100);
    synth_pitch(chan, octave(state->pitch), freq);
}

void midi_fm_program(u8 chan, u8 program)
{
    const Channel* data = presets[program];
    synth_preset(chan, data);
}

void midi_fm_allNotesOff(u8 chan)
{
    midi_fm_noteOff(chan, 0);
}

static u8 octave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 freqNumber(u8 pitch)
{
    return FREQ_NUMBERS[((u8)(pitch - MIN_MIDI_PITCH)) % SEMITONES];
}

static u8 pitchIsOutOfRange(u8 pitch)
{
    return pitch < MIN_MIDI_PITCH || pitch > MAX_MIDI_PITCH;
}

static u8 effectiveVolume(FmChannelState* channelState)
{
    return (channelState->volume * channelState->velocity) / 0x7F;
}
