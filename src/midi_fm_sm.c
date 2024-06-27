#include "midi_fm_sm.h"
#include "midi_fm.h"
#include "midi.h"

static u8 currentPitch = 0;

void midi_fm_sm_note_on(u8 op, u8 pitch, u8 velocity)
{
    synth_specialModePitch(
        op, midi_fm_pitchToOctave(pitch), midi_fm_pitchToFreqNumber(pitch));
    synth_specialModeVolume(op, velocity);
    currentPitch = pitch;
}

void midi_fm_sm_pitch_bend(u8 op, u16 bend)
{
    u16 freq = midi_fm_pitchToFreqNumber(currentPitch);
    s16 bendRelative = bend - MIDI_PITCH_BEND_CENTRE;
    freq = freq + (bendRelative / 75);

    synth_specialModePitch(op, midi_fm_pitchToOctave(currentPitch), freq);
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