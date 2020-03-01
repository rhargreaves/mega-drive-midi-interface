#include "midi_psg.h"
#include "midi.h"
#include "psg_chip.h"
#include <stdbool.h>

#define MIN_PSG_CHAN 6
#define MAX_PSG_CHAN 9
#define MIN_MIDI_KEY 45

#define EEF_END 0xFF

static const u16 FREQUENCIES[] = { 110, 117, 123, 131, 139, 147, 156, 165, 175,
    185, 196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415,
    440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
    1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
    2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
    4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902,
    8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544 };

static const u8 ATTENUATIONS[] = { 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12,
    11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7,
    7, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const u8 ENVELOPE_0[] = { 0x00, EEF_END };
static const u8 ENVELOPE_1[] = { 0x00, 0x0F, EEF_END };
static const u8 ENVELOPE_2[] = { 0x00, 0x07, 0x0F, EEF_END };
static const u8 ENVELOPE_3[] = { 0x00, 0x01, 0x02, 0x01, 0x01, 0x02, 0x02, 0x03,
    0x03, 0x04, 0x04, 0x08, 0x08, 0x08, 0x08, 0x0A, 0x0A, 0x0C, 0x0C, EEF_END };

static const u8* ENVELOPES[MIDI_PROGRAMS] = { ENVELOPE_0, ENVELOPE_1,
    ENVELOPE_2, ENVELOPE_3, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0,
    ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0, ENVELOPE_0 };

typedef struct MidiPsgChannel MidiPsgChannel;

struct MidiPsgChannel {
    u8 chanNum;
    u8 key;
    u8 attenuation;
    bool noteOn;
    u8 volume;
    u8 velocity;
    u8 envelope;
    const u8* envelopeStep;
};

static MidiPsgChannel psgChannels[MAX_PSG_CHANS];
static u16 freqForMidiKey(u8 midiKey);
static MidiPsgChannel* psgChannel(u8 psgChan);

void midi_psg_init(void)
{
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        MidiPsgChannel* psgChan = psgChannel(chan);
        psgChan->chanNum = chan;
        psgChan->noteOn = false;
        psgChan->volume = MAX_MIDI_VOLUME;
        psgChan->velocity = MAX_MIDI_VOLUME;
        psgChan->envelope = 0;
        psgChan->envelopeStep = NULL;
    }
}

static u8 effectiveAttenuation(MidiPsgChannel* psgChan)
{
    u8 att = ATTENUATIONS[(psgChan->volume * psgChan->velocity) / 0x7F];
    u8 invAtt = PSG_ATTENUATION_SILENCE - att;
    u8 envAtt = *psgChan->envelopeStep;
    u8 invEnvAtt = PSG_ATTENUATION_SILENCE - envAtt;
    u8 invEffectiveAtt = (invAtt * invEnvAtt) / PSG_ATTENUATION_SILENCE;
    u8 effectiveAtt = PSG_ATTENUATION_SILENCE - invEffectiveAtt;
    return effectiveAtt;
}

void midi_psg_noteOn(u8 chan, u8 key, u8 velocity)
{
    if (key < MIN_MIDI_KEY) {
        return;
    }
    MidiPsgChannel* psgChan = psgChannel(chan);
    psg_frequency(chan, freqForMidiKey(key));

    psgChan->velocity = velocity;
    psgChan->envelopeStep = ENVELOPES[psgChan->envelope];
    psg_attenuation(chan, effectiveAttenuation(psgChan));
    psgChan->key = key;
    psgChan->noteOn = true;
}

void midi_psg_noteOff(u8 chan, u8 pitch)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    if (psgChan->noteOn && psgChan->key == pitch) {
        psg_attenuation(chan, PSG_ATTENUATION_SILENCE);
        psgChan->noteOn = false;
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
    psg_frequency(chan, freq);
}

void midi_psg_program(u8 chan, u8 program)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    psgChan->envelope = program;
}

void midi_psg_pan(u8 chan, u8 pan)
{
}

static void incrementEnvelopeStep(MidiPsgChannel* chan)
{
    if (*chan->envelopeStep == EEF_END) {
        return;
    }
    chan->envelopeStep++;
    if (*chan->envelopeStep == EEF_END) {
        return;
    }
    u8 attenuation = *chan->envelopeStep;
    psg_attenuation(chan->chanNum, attenuation);
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
    return FREQUENCIES[midiKey - MIN_MIDI_KEY];
}

static MidiPsgChannel* psgChannel(u8 chan)
{
    return &psgChannels[chan];
}
