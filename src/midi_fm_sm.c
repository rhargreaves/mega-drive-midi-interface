#include "midi_fm_sm.h"
#include "midi_fm.h"
#include "midi.h"

static u8 currentPitch = 0;
static u16 currentPitchBend = DEFAULT_MIDI_PITCH_BEND;

static u16 effectiveFreq(u8 pitch, u16 pitchBend)
{
    u16 freq = midi_fm_pitchToFreqNumber(pitch);
    s16 bendRelative = pitchBend - MIDI_PITCH_BEND_CENTRE;
    return freq + (bendRelative / 75);
}

void midi_fm_sm_note_on(u8 op, u8 pitch, u8 velocity)
{
    currentPitch = pitch;
    synth_specialModePitch(op, midi_fm_pitchToOctave(currentPitch),
        effectiveFreq(currentPitch, currentPitchBend));
    synth_specialModeVolume(op, velocity);
}

void midi_fm_sm_pitch_bend(u8 op, u16 bend)
{
    currentPitchBend = bend;
    synth_specialModePitch(op, midi_fm_pitchToOctave(currentPitch),
        effectiveFreq(currentPitch, currentPitchBend));
}

void midi_fm_sm_reset(void)
{
    currentPitch = 0;
    currentPitchBend = DEFAULT_MIDI_PITCH_BEND;
}

// no-ops
void midi_fm_sm_note_off(u8 chan, u8 pitch)
{
}

void midi_fm_sm_channel_volume(u8 chan, u8 volume)
{
}

void midi_fm_sm_pan(u8 chan, u8 pan)
{
}

void midi_fm_sm_program(u8 chan, u8 program)
{
}

void midi_fm_sm_all_notes_off(u8 chan)
{
}