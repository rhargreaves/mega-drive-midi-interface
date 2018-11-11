#include "midi.h"
#include "psg_chip.h"
#include "synth.h"

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

static const u16 FREQUENCIES[] = {
    8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 19, 21, 22, 23, 25,
    26, 28, 29, 31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, 65, 69, 73,
    78, 82, 87, 93, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175,
    185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392,
    415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880,
    932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760,
    1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520,
    3729, 3951, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040,
    7459, 7902, 8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544
};

static const u8 TOTAL_LEVELS[] = { 126, 122, 117, 113, 108, 104, 100, 97, 93,
    89, 86, 83, 80, 77, 74, 71, 68, 66, 63, 61, 58, 56, 54, 52, 50, 48, 46, 44,
    43, 41, 40, 38, 37, 35, 34, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21,
    20, 19, 19, 18, 17, 17, 16, 15, 15, 14, 13, 13, 12, 12, 11, 11, 11, 10, 10,
    9, 9, 9, 8, 8, 7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3,
    3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const u8 ATTENUATIONS[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8,
    8, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2,
    1, 1, 1, 0
};

void midi_noteOn(u8 chan, u8 pitch, u8 velocity)
{
    if (chan < MIN_PSG_CHAN) {
        synth_pitch(chan,
            midi_getOctave(pitch),
            midi_getFreqNumber(pitch));
        synth_noteOn(chan);
    } else {
        psg_noteOn(chan - MIN_PSG_CHAN, FREQUENCIES[pitch]);
    }
}

void midi_noteOff(u8 chan)
{
    if (chan < MIN_PSG_CHAN) {
        synth_noteOff(chan);
    } else {
        psg_noteOff(chan - MIN_PSG_CHAN);
    }
}

void midi_channelVolume(u8 chan, u8 volume)
{
    if (chan < MIN_PSG_CHAN) {
        synth_totalLevel(chan, TOTAL_LEVELS[volume]);
    } else {
        psg_attenuation(chan - MIN_PSG_CHAN, ATTENUATIONS[volume]);
    }
}

void midi_pan(u8 chan, u8 pan)
{
    if (pan > 96) {
        synth_stereo(chan, STEREO_MODE_RIGHT);
    } else if (pan > 31) {
        synth_stereo(chan, STEREO_MODE_CENTRE);
    } else {
        synth_stereo(chan, STEREO_MODE_LEFT);
    }
}

static u8 midi_getOctave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 midi_getFreqNumber(u8 pitch)
{
    return FREQ_NUMBERS[((u8)(pitch - MIN_MIDI_PITCH)) % SEMITONES];
}
