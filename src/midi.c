#include <midi.h>
#include <synth.h>

static u8 midi_getOctave(u8 pitch);
static u16 midi_getFreqNumber(u8 pitch);
static u16 midi_modulo(u16 a, u16 b);

void midi_process(Message* message)
{
    if ((message->status & 0b10010000) == 0b10010000) {
        synth_noteOn(0);
        synth_pitch(
            midi_getOctave(message->data),
            midi_getFreqNumber(message->data));
    } else if ((message->status & 0b10000000) == 0b10000000) {
        synth_noteOff(0);
    }
}

static u8 midi_getOctave(u8 pitch)
{
    return ((pitch - 21) / 12);
}

static u16 midi_getFreqNumber(u8 pitch)
{
    const u16 freqNumbers[] = {
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
    return freqNumbers[midi_modulo(pitch - 23, 12)];
}

static u16 midi_modulo(u16 a, u16 b)
{
    u16 c = a - b;
    while (c >= b) {
        c = c - b;
    }
    return c;
}
