#include <midi.h>
#include <synth.h>

static u8 midi_getOctave(u8 pitch);
static u16 midi_getFreqNumber(u8 pitch);

static const u8 MIN_MIDI_PITCH = 23;
static const u8 SEMITONES = 12;
static const u16 FREQ_NUMBERS[] = {
    617, // B
    653,
    692,
    733,
    777,
    823,
    872,
    924,
    979,
    1037,
    1099,
    1164 // A#
};

void midi_noteOn(u8 pitch, u8 velocity)
{
    synth_noteOn(0);
    synth_pitch(
        midi_getOctave(pitch),
        midi_getFreqNumber(pitch));
}

void midi_noteOff(void)
{
    synth_noteOff(0);
}

static u8 midi_getOctave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 midi_getFreqNumber(u8 pitch)
{
    return FREQ_NUMBERS[((u8)(pitch - MIN_MIDI_PITCH)) % SEMITONES];
}
