#include "midi_psg.h"
#include "envelopes.h"
#include "midi.h"
#include "psg_chip.h"
#include <stdbool.h>

#define MIN_PSG_CHAN 6
#define MAX_PSG_CHAN 9
#define MIN_MIDI_KEY 45
#define MAX_MIDI_KEY 127
#define PITCH_SHIFTS 7
#define NUM_FREQUENCIES (128 - MIN_MIDI_KEY)

static const u16 FREQUENCIES[NUM_FREQUENCIES] = { 110, 117, 123, 131, 139, 147,
    156, 165, 175, 185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349,
    370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831,
    880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760,
    1865, 1976, 2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520,
    3729, 3951, 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040,
    7459, 7902, 8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544 };

static const u8 ATTENUATIONS[] = { 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12,
    11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7,
    7, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

typedef struct MidiPsgChannel MidiPsgChannel;

static const u8** envelopes;

struct MidiPsgChannel {
    u8 chanNum;
    u8 key;
    u8 attenuation;
    bool noteOn;
    u8 volume;
    u8 velocity;
    u8 envelope;
    const u8* envelopeStep;
    const u8* envelopeLoopStart;
    bool noteReleased;
    u16 freq;
};

static MidiPsgChannel psgChannels[MAX_PSG_CHANS];

static u16 freqForMidiKey(u8 midiKey);
static MidiPsgChannel* psgChannel(u8 psgChan);
static void initEnvelope(MidiPsgChannel* psgChan);

void midi_psg_init(const u8** defaultEnvelopes)
{
    envelopes = defaultEnvelopes;
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        MidiPsgChannel* psgChan = psgChannel(chan);
        psgChan->chanNum = chan;
        psgChan->noteOn = false;
        psgChan->volume = MAX_MIDI_VOLUME;
        psgChan->velocity = MAX_MIDI_VOLUME;
        psgChan->envelope = 0;
        psgChan->noteReleased = false;
        psgChan->freq = 0;
        initEnvelope(psgChan);
    }
}

static void initEnvelope(MidiPsgChannel* psgChan)
{
    psgChan->envelopeStep = envelopes[psgChan->envelope];
    psgChan->envelopeLoopStart = NULL;
}

static u8 effectiveAttenuation(MidiPsgChannel* psgChan)
{
    u8 att
        = ATTENUATIONS[(psgChan->volume * psgChan->velocity) / MAX_MIDI_VOLUME];
    u8 invAtt = MAX_ATTENUATION - att;
    u8 envAtt = *psgChan->envelopeStep & 0x0F;
    u8 invEnvAtt = MAX_ATTENUATION - envAtt;
    u8 invEffectiveAtt = (invAtt * invEnvAtt) / MAX_ATTENUATION;
    u8 effectiveAtt = MAX_ATTENUATION - invEffectiveAtt;
    return effectiveAtt;
}

static void noteOff(MidiPsgChannel* psgChan)
{
    psg_attenuation(psgChan->chanNum, PSG_ATTENUATION_SILENCE);
    psgChan->noteOn = false;
    psgChan->noteReleased = false;
}

static const u16 PITCH_SHIFT_TABLE[PITCH_SHIFTS] = {
    1,
    2,
    4,
    10,
    25,
    50,
    120,
};

static u16 effectiveFrequency(MidiPsgChannel* psgChan)
{
    const u8 DIVISOR = 20;
    u8 shift = *psgChan->envelopeStep >> 4;
    u16 freq = freqForMidiKey(psgChan->key);
    if (shift > 0 && shift < 0x08) {
        u16 increments = PITCH_SHIFT_TABLE[shift - 1];
        u16 nextFreq = freqForMidiKey(psgChan->key + 1);
        freq = freq + ((nextFreq - freq) / DIVISOR) * increments;
    } else if (shift >= 0x08) {
        u16 decrements = PITCH_SHIFT_TABLE[shift - 0x07 - 1];
        u16 prevFreq = freqForMidiKey(psgChan->key - 1);
        freq = freq - ((freq - prevFreq) / DIVISOR) * decrements;
    }
    return freq;
}

static void applyFrequency(MidiPsgChannel* psgChan, u16 newFreq)
{
    if (newFreq != psgChan->freq) {
        psg_frequency(psgChan->chanNum, newFreq);
        psgChan->freq = newFreq;
    }
}

static void applyEnvelopeStep(MidiPsgChannel* psgChan)
{
    if (*psgChan->envelopeStep == EEF_LOOP_START) {
        psgChan->envelopeStep++;
        psgChan->envelopeLoopStart = psgChan->envelopeStep;
    }
    if (*psgChan->envelopeStep == EEF_LOOP_END
        && psgChan->envelopeLoopStart != NULL) {
        psgChan->envelopeStep++;
        if (!psgChan->noteReleased) {
            psgChan->envelopeStep = psgChan->envelopeLoopStart;
        }
    }
    if (*psgChan->envelopeStep == EEF_END) {
        if (psgChan->envelopeLoopStart != NULL && !psgChan->noteReleased) {
            psgChan->envelopeStep = psgChan->envelopeLoopStart;
        } else {
            noteOff(psgChan);
            return;
        }
    }
    applyFrequency(psgChan, effectiveFrequency(psgChan));
    psg_attenuation(psgChan->chanNum, effectiveAttenuation(psgChan));
}

void midi_psg_noteOn(u8 chan, u8 key, u8 velocity)
{
    if (key < MIN_MIDI_KEY) {
        return;
    }
    MidiPsgChannel* psgChan = psgChannel(chan);
    psgChan->noteReleased = false;
    psgChan->key = key;
    applyFrequency(psgChan, freqForMidiKey(key));
    psgChan->velocity = velocity;
    initEnvelope(psgChan);
    applyEnvelopeStep(psgChan);
    psgChan->noteOn = true;
}

void midi_psg_noteOff(u8 chan, u8 pitch)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    if (psgChan->noteOn && psgChan->key == pitch) {
        if (psgChan->envelopeLoopStart != NULL) {
            psgChan->noteReleased = true;
        } else {
            noteOff(psgChan);
        }
    }
}

void midi_psg_allNotesOff(u8 chan)
{
    midi_psg_noteOff(chan, psgChannel(chan)->key);
}

void midi_psg_channelVolume(u8 chan, u8 volume)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    psgChan->volume = volume;
    if (psgChan->noteOn) {
        psg_attenuation(chan, effectiveAttenuation(psgChan));
    }
}

void midi_psg_pitchBend(u8 chan, u16 bend)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    u16 freq = freqForMidiKey(psgChan->key);
    s16 bendRelative = bend - 0x2000;
    freq = freq + (bendRelative / 100);
    applyFrequency(psgChan, freq);
}

void midi_psg_program(u8 chan, u8 program)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    psgChan->envelope = program;
}

void midi_psg_pan(u8 chan, u8 pan)
{
    (void)chan;
    (void)pan;
}

static void incrementEnvelopeStep(MidiPsgChannel* chan)
{
    if (*chan->envelopeStep != EEF_END) {
        chan->envelopeStep++;
    }
    applyEnvelopeStep(chan);
}

void midi_psg_tick(void)
{
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        MidiPsgChannel* psgChan = psgChannel(chan);
        if (psgChan->noteOn) {
            incrementEnvelopeStep(psgChan);
        }
    }
}

static u16 freqForMidiKey(u8 midiKey)
{
    if (midiKey < MIN_MIDI_KEY) {
        return FREQUENCIES[0];
    }
    if (midiKey > MAX_MIDI_KEY) {
        return FREQUENCIES[NUM_FREQUENCIES - 1];
    }
    return FREQUENCIES[midiKey - MIN_MIDI_KEY];
}

static MidiPsgChannel* psgChannel(u8 chan)
{
    return &psgChannels[chan];
}
