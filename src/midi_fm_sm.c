#include "midi_fm_sm.h"
#include "midi_fm.h"

static const u8 SEMITONES = 12;
static const u16 FREQS[] = {
    607, // B
    644, 681, 722, 765, 810, 858, 910, 964, 1021, 1081,
    1146 // A#
};

static u8 octave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 freqNumber(u8 pitch)
{
    return FREQS[((u8)(pitch - MIN_MIDI_PITCH)) % SEMITONES];
}

void midi_fm_sm_note_on(u8 chan, u8 pitch, u8 velocity)
{
    synth_specialModePitch(0, octave(pitch), freqNumber(pitch));
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