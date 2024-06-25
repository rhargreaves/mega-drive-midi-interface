#include "midi_fm_sm.h"
#include "midi_fm.h"

void midi_fm_sm_note_on(u8 op, u8 pitch, u8 velocity)
{
    synth_specialModePitch(
        op, midi_fm_pitchToOctave(pitch), midi_fm_pitchToFreqNumber(pitch));
}

void midi_fm_sm_pitch_bend(u8 chan, u16 bend)
{
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