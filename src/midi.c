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

static const u8 TOTAL_LEVELS[] = {
    126, 122, 117, 113, 108, 104, 100, 97, 93, 89, 86, 83,
    80, 77, 74, 71, 68, 66, 63, 61, 58, 56, 54, 52, 50, 48,
    46, 44, 43, 41, 40, 38, 37, 35, 34, 32, 31, 30, 29, 28,
    27, 26, 25, 24, 23, 22, 21, 20, 19, 19, 18, 17, 17, 16,
    15, 15, 14, 13, 13, 12, 12, 11, 11, 11, 10, 10, 9, 9, 9,
    8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
};

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    synth_pitch(chan,
        midi_getOctave(pitch),
        midi_getFreqNumber(pitch));
    synth_noteOn(chan);
}

void midi_noteOff(u8 chan)
{
    synth_noteOff(chan);
}

void midi_channelVolume(u8 chan, u8 volume)
{
    synth_totalLevel(chan, TOTAL_LEVELS[volume]);
}

static u8 midi_getOctave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 midi_getFreqNumber(u8 pitch)
{
    return FREQ_NUMBERS[((u8)(pitch - MIN_MIDI_PITCH)) % SEMITONES];
}
