#include "midi_psg.h"
#include "utils.h"
#include "envelopes.h"
#include "midi.h"
#include "psg.h"
#include "scheduler.h"
#include "pitchcents.h"

#define MIN_PSG_CHAN 6
#define MAX_PSG_CHAN 9
#define MIN_MIDI_KEY 45
#define MAX_MIDI_KEY 127
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

typedef struct MidiPsgChannel {
    u8 chanNum;
    u8 pitch;
    s8 cents;
    u8 attenuation;
    u16 freq;
    bool noteOn;
    u8 volume;
    u8 velocity;
    u8 envelope;
    const u8* envelopeStep;
    const u8* envelopeLoopStart;
    bool noteReleased;
} MidiPsgChannel;

static u8 userDefinedEnvelope[256];
static u8* userDefinedEnvelopePtr;
static const u8** envelopes;
static MidiPsgChannel psgChannels[MAX_PSG_CHANS];

static u16 tone_for_midi_key(u8 midiKey);
static MidiPsgChannel* psg_channel(u8 psgChan);
static void init_envelope(MidiPsgChannel* psgChan);
static void apply_attenuation(MidiPsgChannel* psgChan, u8 newAtt);
static u16 envelope_tone(MidiPsgChannel* psgChan);
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
        MidiPsgChannel* psgChan = psg_channel(chan);
        psgChan->attenuation = PSG_ATTENUATION_SILENCE;
        psgChan->chanNum = chan;
        psgChan->noteOn = false;
        psgChan->volume = MAX_MIDI_VOLUME;
        psgChan->velocity = MAX_MIDI_VOLUME;
        psgChan->envelope = 0;
        psgChan->noteReleased = false;
        psgChan->freq = 0;
        psgChan->pitch = 0;
        psgChan->cents = 0;
        init_envelope(psgChan);
    }
}

static void init_envelope(MidiPsgChannel* psgChan)
{
    psgChan->envelopeStep
        = userDefinedEnvelopePtr != NULL ? userDefinedEnvelopePtr : envelopes[psgChan->envelope];
    psgChan->envelopeLoopStart = NULL;
}

static u8 effective_attenuation(MidiPsgChannel* psgChan)
{
    u8 att = ATTENUATIONS[(psgChan->volume * psgChan->velocity) / MAX_MIDI_VOLUME];
    u8 invAtt = MAX_ATTENUATION - att;
    u8 envAtt = *psgChan->envelopeStep & 0x0F;
    u8 invEnvAtt = MAX_ATTENUATION - envAtt;
    u8 invEffectiveAtt = (invAtt * invEnvAtt) / MAX_ATTENUATION;
    return MAX_ATTENUATION - invEffectiveAtt;
}

static void note_off(MidiPsgChannel* psgChan)
{
    apply_attenuation(psgChan, PSG_ATTENUATION_SILENCE);
    psgChan->noteOn = false;
    psgChan->noteReleased = false;
}

static u16 tone_from_pitch_cents(PitchCents pc)
{
    u16 tone = tone_for_midi_key(pc.pitch);
    if (pc.cents == 0) {
        return tone;
    }
    return tone + (((tone_for_midi_key(pc.pitch + 1) - tone) * pc.cents) / 100);
}

static u16 envelope_tone(MidiPsgChannel* psgChan)
{
    PitchCents pc = { .pitch = psgChan->pitch, .cents = psgChan->cents };
    u8 shift = *psgChan->envelopeStep >> 4;
    switch (shift) {
    case 0:
        break;
    case 0x1:
        pc = pc_shift(pc, 5);
        break;
    case 0x2:
        pc = pc_shift(pc, 10);
        break;
    case 0x3:
        pc = pc_shift(pc, 20);
        break;
    case 0x4:
        pc = pc_shift(pc, 50);
        break;
    case 0x5:
        pc.pitch += 1;
        break;
    case 0x6:
        pc.pitch += 2;
        break;
    case 0x7:
        pc.pitch += 5;
        break;
    case 0x8:
        pc = pc_shift(pc, -5);
        break;
    case 0x9:
        pc = pc_shift(pc, -10);
        break;
    case 0xA:
        pc = pc_shift(pc, -20);
        break;
    case 0xB:
        pc = pc_shift(pc, -50);
        break;
    case 0xC:
        pc.pitch -= 1;
        break;
    case 0xD:
        pc.pitch -= 2;
        break;
    case 0xE:
        pc.pitch -= 5;
        break;
    }
    return tone_from_pitch_cents(pc);
}

static void apply_tone(MidiPsgChannel* psgChan, u16 newFreq)
{
    if (newFreq != psgChan->freq) {
        PSG_setTone(psgChan->chanNum, newFreq);
        psgChan->freq = newFreq;
    }
}

static void apply_attenuation(MidiPsgChannel* psgChan, u8 newAtt)
{
    if (newAtt != psgChan->attenuation) {
        PSG_setEnvelope(psgChan->chanNum, newAtt);
        psgChan->attenuation = newAtt;
    }
}

static void apply_envelope_step(MidiPsgChannel* psgChan)
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
            note_off(psgChan);
            return;
        }
    }
    apply_tone(psgChan, envelope_tone(psgChan));
    apply_attenuation(psgChan, effective_attenuation(psgChan));
}

void midi_psg_note_on(u8 chan, u8 pitch, s8 cents, u8 velocity)
{
    if (pitch < MIN_MIDI_KEY) {
        return;
    }
    MidiPsgChannel* psgChan = psg_channel(chan);
    psgChan->noteReleased = false;
    psgChan->pitch = pitch;
    psgChan->cents = cents;
    psgChan->velocity = velocity;
    psgChan->noteOn = true;
    init_envelope(psgChan);
    apply_envelope_step(psgChan);
}

void midi_psg_note_off(u8 chan, u8 pitch)
{
    MidiPsgChannel* psgChan = psg_channel(chan);
    if (psgChan->noteOn) {
        if (psgChan->envelopeLoopStart != NULL) {
            psgChan->noteReleased = true;
        } else {
            note_off(psgChan);
        }
    }
}

void midi_psg_all_notes_off(u8 chan)
{
    midi_psg_note_off(chan, psg_channel(chan)->pitch);
}

void midi_psg_channel_volume(u8 chan, u8 volume)
{
    MidiPsgChannel* psgChan = psg_channel(chan);
    psgChan->volume = volume;
    if (psgChan->noteOn) {
        apply_attenuation(psgChan, effective_attenuation(psgChan));
    }
}

void midi_psg_program(u8 chan, u8 program)
{
    MidiPsgChannel* psgChan = psg_channel(chan);
    psgChan->envelope = program;
}

void midi_psg_pan(u8 chan, u8 pan)
{
    (void)chan;
    (void)pan;
}

static void increment_envelope_step(MidiPsgChannel* chan)
{
    if (*chan->envelopeStep != EEF_END) {
        chan->envelopeStep++;
    }
    apply_envelope_step(chan);
}

void midi_psg_tick(u16 delta)
{
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        MidiPsgChannel* psgChan = psg_channel(chan);
        if (psgChan->noteOn) {
            increment_envelope_step(psgChan);
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

static u16 tone_for_midi_key(u8 midiKey)
{
    const u16* tones = SYS_isPAL() ? TONES_PAL : TONES_NTSC;
    if (midiKey < MIN_MIDI_KEY) {
        return tones[0];
    }
    if (midiKey > MAX_MIDI_KEY) {
        return tones[NUM_TONES - 1];
    }
    return tones[midiKey - MIN_MIDI_KEY];
}

static MidiPsgChannel* psg_channel(u8 chan)
{
    return &psgChannels[chan];
}

void midi_psg_pitch(u8 chan, u8 pitch, s8 cents)
{
    MidiPsgChannel* psgChan = psg_channel(chan);
    psgChan->pitch = pitch;
    psgChan->cents = cents;
    apply_tone(psgChan, envelope_tone(psgChan));
}
