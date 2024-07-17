#include "midi_psg.h"
#include "bits.h"
#include "envelopes.h"
#include "midi.h"
#include "psg.h"
#include "region.h"
#include "scheduler.h"
#include <memory.h>
#include <psg.h>
#include <stdbool.h>
#include <types.h>

#define MIN_PSG_CHAN 6
#define MAX_PSG_CHAN 9
#define MIN_MIDI_KEY 45
#define MAX_MIDI_KEY 127
#define PITCH_SHIFTS 7
#define NUM_TONES (128 - MIN_MIDI_KEY)

static const u16 TONES_NTSC[NUM_TONES] = { 1016, 959, 905, 855, 807, 761, 719, 678, 640, 604, 570,
    538, 508, 479, 452, 427, 403, 380, 359, 339, 320, 302, 285, 269, 254, 239, 226, 213, 201, 190,
    179, 169, 160, 151, 142, 134, 127, 119, 113, 106, 100, 95, 89, 84, 80, 75, 71, 67, 63, 59, 56,
    53, 50, 47, 44, 42, 40, 37, 35, 33, 31, 29, 28, 26, 25, 23, 22, 21, 20, 18, 17, 16, 15, 14, 14,
    13, 12, 11, 11, 10, 10, 9, 8 };

static const u16 TONES_PAL[NUM_TONES] = { 1007, 951, 897, 847, 799, 754, 712, 672, 634, 599, 565,
    533, 503, 475, 448, 423, 399, 377, 356, 336, 317, 299, 282, 266, 251, 237, 224, 211, 199, 188,
    178, 168, 158, 149, 141, 133, 125, 118, 112, 105, 99, 94, 89, 84, 79, 74, 70, 66, 62, 59, 56,
    52, 49, 47, 44, 42, 39, 37, 35, 33, 31, 29, 28, 26, 24, 23, 22, 21, 19, 18, 17, 16, 15, 14, 14,
    13, 12, 11, 11, 10, 9, 9, 8 };

static const u8 ATTENUATIONS[] = { 15, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 11, 11, 11, 11, 10,
    10, 10, 10, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5,
    5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

typedef struct MidiPsgChannel MidiPsgChannel;

static u8 userDefinedEnvelope[256];
static u8* userDefinedEnvelopePtr;
static const u8** envelopes;

struct MidiPsgChannel {
    u8 chanNum;
    u8 key;
    u8 attenuation;
    u16 freq;
    bool noteOn;
    u8 volume;
    u8 velocity;
    u8 envelope;
    const u8* envelopeStep;
    const u8* envelopeLoopStart;
    bool noteReleased;
    u16 pitchBend;
};

static u8 audible;
static MidiPsgChannel psgChannels[MAX_PSG_CHANS];

static u16 toneForMidiKey(u8 midiKey);
static MidiPsgChannel* psgChannel(u8 psgChan);
static void initEnvelope(MidiPsgChannel* psgChan);
static void applyAttenuation(MidiPsgChannel* psgChan, u8 newAtt);
static u16 envelopeTone(MidiPsgChannel* psgChan);
static u16 effectiveTone(MidiPsgChannel* psgChan);
void midi_psg_reset(void);

void midi_psg_init(const u8** defaultEnvelopes)
{
    scheduler_addFrameHandler(midi_psg_tick);
    envelopes = defaultEnvelopes;
    midi_psg_reset();
}

void midi_psg_reset(void)
{
    userDefinedEnvelopePtr = NULL;
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        MidiPsgChannel* psgChan = psgChannel(chan);
        psgChan->attenuation = PSG_ATTENUATION_SILENCE;
        psgChan->chanNum = chan;
        psgChan->noteOn = false;
        psgChan->volume = MAX_MIDI_VOLUME;
        psgChan->velocity = MAX_MIDI_VOLUME;
        psgChan->envelope = 0;
        psgChan->noteReleased = false;
        psgChan->freq = 0;
        psgChan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
        initEnvelope(psgChan);
    }
}

static void initEnvelope(MidiPsgChannel* psgChan)
{
    psgChan->envelopeStep
        = userDefinedEnvelopePtr != NULL ? userDefinedEnvelopePtr : envelopes[psgChan->envelope];
    psgChan->envelopeLoopStart = NULL;
}

static u8 effectiveAttenuation(MidiPsgChannel* psgChan)
{
    u8 att = ATTENUATIONS[(psgChan->volume * psgChan->velocity) / MAX_MIDI_VOLUME];
    u8 invAtt = MAX_ATTENUATION - att;
    u8 envAtt = *psgChan->envelopeStep & 0x0F;
    u8 invEnvAtt = MAX_ATTENUATION - envAtt;
    u8 invEffectiveAtt = (invAtt * invEnvAtt) / MAX_ATTENUATION;
    u8 effectiveAtt = MAX_ATTENUATION - invEffectiveAtt;
    return effectiveAtt;
}

static void noteOff(MidiPsgChannel* psgChan)
{
    applyAttenuation(psgChan, PSG_ATTENUATION_SILENCE);
    psgChan->noteOn = false;
    psgChan->noteReleased = false;
}

static const u16 KEY_SHIFT_TABLE[PITCH_SHIFTS] = { 1, 1, 1, 1, 1, 2, 5 };
static const u16 PITCH_SHIFT_TABLE[PITCH_SHIFTS] = { 1, 2, 4, 10, 20, 40, 100 };

static u16 envelopeTone(MidiPsgChannel* psgChan)
{
    const u8 DIVISOR = 20;
    u8 shift = *psgChan->envelopeStep >> 4;
    u16 freq = toneForMidiKey(psgChan->key);
    if (shift == 0) {
        return freq;
    }
    u8 shiftIndex = shift - 1;
    if (shift < 0x5) {
        u16 nextTone = toneForMidiKey(psgChan->key + 1);
        return freq - ((freq - nextTone) * PITCH_SHIFT_TABLE[shiftIndex]) / DIVISOR;
    } else if (shift < 0x8) {
        return toneForMidiKey(psgChan->key + KEY_SHIFT_TABLE[shiftIndex]);
    } else if (shift < 0xC) {
        u16 prevFreq = toneForMidiKey(psgChan->key - 1);
        return freq + ((prevFreq - freq) * PITCH_SHIFT_TABLE[shiftIndex - 0x07]) / DIVISOR;
    } else {
        return toneForMidiKey(psgChan->key - KEY_SHIFT_TABLE[shiftIndex - 0x07]);
    }
}

static u16 pitchBentTone(MidiPsgChannel* psgChan, u16 baseTone)
{
    if (psgChan->pitchBend == DEFAULT_MIDI_PITCH_BEND) {
        return baseTone;
    } else if (psgChan->pitchBend < MIDI_PITCH_BEND_CENTRE) {
        // bending down
        u16 prevTone = toneForMidiKey(psgChan->key - GENERAL_MIDI_PITCH_BEND_SEMITONE_RANGE);
        // prev tone is higher
        u16 diff = prevTone - baseTone;
        u16 bend = MIDI_PITCH_BEND_CENTRE - psgChan->pitchBend;
        return baseTone + (u32)((diff * bend) / MIDI_PITCH_BEND_CENTRE);
    } else {
        // bending up
        u16 nextTone = toneForMidiKey(psgChan->key + GENERAL_MIDI_PITCH_BEND_SEMITONE_RANGE);
        // next tone is lower
        u16 diff = baseTone - nextTone;
        u16 bend = psgChan->pitchBend - MIDI_PITCH_BEND_CENTRE;
        return baseTone - (u32)((diff * bend) / MIDI_PITCH_BEND_CENTRE);
    }
}

static u16 effectiveTone(MidiPsgChannel* psgChan)
{
    u16 baseFreq = envelopeTone(psgChan);
    return pitchBentTone(psgChan, baseFreq);
}

static void applyTone(MidiPsgChannel* psgChan, u16 newFreq)
{
    if (newFreq != psgChan->freq) {
        PSG_setTone(psgChan->chanNum, newFreq);
        psgChan->freq = newFreq;
    }
}

static void applyAttenuation(MidiPsgChannel* psgChan, u8 newAtt)
{
    if (newAtt != psgChan->attenuation) {
        PSG_setEnvelope(psgChan->chanNum, newAtt);
        if (newAtt == PSG_ATTENUATION_SILENCE) {
            CLEAR_BIT(audible, psgChan->chanNum);
        } else {
            SET_BIT(audible, psgChan->chanNum);
        }
        psgChan->attenuation = newAtt;
    }
}

static void applyEnvelopeStep(MidiPsgChannel* psgChan)
{
    if (*psgChan->envelopeStep == EEF_LOOP_START) {
        psgChan->envelopeStep++;
        psgChan->envelopeLoopStart = psgChan->envelopeStep;
    }
    if (*psgChan->envelopeStep == EEF_LOOP_END && psgChan->envelopeLoopStart != NULL) {
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
    applyTone(psgChan, effectiveTone(psgChan));
    applyAttenuation(psgChan, effectiveAttenuation(psgChan));
}

void midi_psg_note_on(u8 chan, u8 key, u8 velocity)
{
    if (key < MIN_MIDI_KEY) {
        return;
    }
    MidiPsgChannel* psgChan = psgChannel(chan);
    psgChan->noteReleased = false;
    psgChan->key = key;
    psgChan->velocity = velocity;
    psgChan->noteOn = true;
    initEnvelope(psgChan);
    applyEnvelopeStep(psgChan);
}

void midi_psg_note_off(u8 chan, u8 pitch)
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

void midi_psg_all_notes_off(u8 chan)
{
    midi_psg_note_off(chan, psgChannel(chan)->key);
}

void midi_psg_channel_volume(u8 chan, u8 volume)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    psgChan->volume = volume;
    if (psgChan->noteOn) {
        applyAttenuation(psgChan, effectiveAttenuation(psgChan));
    }
}

void midi_psg_pitch_bend(u8 chan, u16 bend)
{
    MidiPsgChannel* psgChan = psgChannel(chan);
    psgChan->pitchBend = bend;
    applyTone(psgChan, effectiveTone(psgChan));
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

void midi_psg_load_envelope(const u8* eef)
{
    u16 i = 0;
    for (;;) {
        userDefinedEnvelope[i++] = *eef;
        if (*eef == EEF_END) {
            break;
        }
        eef++;
    };
    userDefinedEnvelopePtr = userDefinedEnvelope;
}

static u16 toneForMidiKey(u8 midiKey)
{
    const u16* tones = region_isPal() ? TONES_PAL : TONES_NTSC;
    if (midiKey < MIN_MIDI_KEY) {
        return tones[0];
    }
    if (midiKey > MAX_MIDI_KEY) {
        return tones[NUM_TONES - 1];
    }
    return tones[midiKey - MIN_MIDI_KEY];
}

static MidiPsgChannel* psgChannel(u8 chan)
{
    return &psgChannels[chan];
}

u8 midi_psg_busy(void)
{
    return audible;
}
