#include "midi_fm.h"
#include "midi.h"
#include "region.h"
#include "synth.h"

static const u8 SEMITONES = 12;
static const u16 FREQS[] = {
    // Lower Pitch Bends (-2 st)
    541, 573,
    // Normal Range
    607, // B
    644, 681, 722, 765, 810, 858, 910, 964, 1021, 1081,
    1146, // A#
    // Upper Pitch Bends (+2 st)
    1214, 1286
};
static const u8 FREQ_NORMAL_RANGE_OFFSET = 2;

typedef struct MidiFmChannel MidiFmChannel;

struct MidiFmChannel {
    u8 pitch;
    u8 volume;
    u8 velocity;
    u8 pan;
    bool percussive;
    u16 pitchBend;
};

static MidiFmChannel fmChannels[MAX_FM_CHANS];

static u8 pitchIsOutOfRange(u8 pitch);
static u8 effectiveVolume(MidiFmChannel* channelState);
static void updatePan(u8 chan);
static u16 lookupFreqNum(u8 pitch, s16 offset);
void midi_fm_reset(void);

static const FmChannel** presets;
static const PercussionPreset** percussionPresets;

void midi_fm_init(const FmChannel** defaultPresets,
    const PercussionPreset** defaultPercussionPresets)
{
    presets = defaultPresets;
    percussionPresets = defaultPercussionPresets;
    midi_fm_reset();
}

void midi_fm_reset(void)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        MidiFmChannel* fmChan = &fmChannels[chan];
        fmChan->volume = MAX_MIDI_VOLUME;
        fmChan->velocity = MAX_MIDI_VOLUME;
        fmChan->pan = 0;
        fmChan->percussive = false;
        fmChan->pitchBend = DEFAULT_MIDI_PITCH_BEND;
    }
    synth_init(presets[0]);
}

void midi_fm_note_on(u8 chan, u8 pitch, u8 velocity)
{
    if (pitchIsOutOfRange(pitch)) {
        return;
    }
    MidiFmChannel* fmChan = &fmChannels[chan];
    if (fmChan->percussive) {
        const PercussionPreset* percussionPreset = percussionPresets[pitch];
        synth_preset(chan, &percussionPreset->channel);
        pitch = percussionPreset->key;
    }
    fmChan->velocity = velocity;
    synth_volume(chan, effectiveVolume(fmChan));
    fmChan->pitch = pitch;
    synth_pitch(chan, midi_fm_pitchToOctave(fmChan->pitch),
        midi_fm_pitchToFreqNum(fmChan->pitch, fmChan->pitchBend));
    synth_noteOn(chan);
}

void midi_fm_note_off(u8 chan, u8 pitch)
{
    (void)pitch;
    synth_noteOff(chan);
}

void midi_fm_channel_volume(u8 chan, u8 volume)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->volume = volume;
    synth_volume(chan, effectiveVolume(fmChan));
}

u16 midi_fm_pitchToFreqNum(u8 pitch, u16 pitchBend)
{
    u16 freq = lookupFreqNum(pitch, 0);
    if (pitchBend == MIDI_PITCH_BEND_CENTRE) {
        return freq;
    }
    s16 bendRelative = pitchBend - MIDI_PITCH_BEND_CENTRE;
    s16 approxFreqDelta;
    if (bendRelative < 0) {
        // bend down
        u16 boundFreq
            = lookupFreqNum(pitch, -GENERAL_MIDI_PITCH_BEND_SEMITONE_RANGE);
        approxFreqDelta
            = (((freq - boundFreq) * bendRelative) / DEFAULT_MIDI_PITCH_BEND);
    } else {
        // bend up
        u16 boundFreq
            = lookupFreqNum(pitch, GENERAL_MIDI_PITCH_BEND_SEMITONE_RANGE);
        approxFreqDelta
            = (((boundFreq - freq) * bendRelative) / DEFAULT_MIDI_PITCH_BEND);
    }
    return freq + approxFreqDelta;
}
void midi_fm_pitch_bend(u8 chan, u16 bend)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->pitchBend = bend;
    synth_pitch(chan, midi_fm_pitchToOctave(fmChan->pitch),
        midi_fm_pitchToFreqNum(fmChan->pitch, fmChan->pitchBend));
}

void midi_fm_program(u8 chan, u8 program)
{
    const FmChannel* data = presets[program];
    synth_preset(chan, data);
    updatePan(chan);
}

void midi_fm_all_notes_off(u8 chan)
{
    midi_fm_note_off(chan, 0);
}

void midi_fm_percussive(u8 chan, bool enabled)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->percussive = enabled;
}

void midi_fm_pan(u8 chan, u8 pan)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    fmChan->pan = pan;
    updatePan(chan);
}

static void updatePan(u8 chan)
{
    MidiFmChannel* fmChan = &fmChannels[chan];
    if (fmChan->pan > 95) {
        synth_stereo(chan, STEREO_MODE_RIGHT);
    } else if (fmChan->pan > 31) {
        synth_stereo(chan, STEREO_MODE_CENTRE);
    } else {
        synth_stereo(chan, STEREO_MODE_LEFT);
    }
}

u8 midi_fm_pitchToOctave(u8 pitch)
{
    return (pitch - MIN_MIDI_PITCH) / SEMITONES;
}

static u16 lookupFreqNum(u8 pitch, s16 offset)
{
    u16 index = FREQ_NORMAL_RANGE_OFFSET
        + ((u8)(pitch - MIN_MIDI_PITCH) % SEMITONES) + offset;
    u16 freq = FREQS[index];
    return freq;
}

static u8 pitchIsOutOfRange(u8 pitch)
{
    return pitch < MIN_MIDI_PITCH || pitch > MAX_MIDI_PITCH;
}

static u8 effectiveVolume(MidiFmChannel* fmChan)
{
    return (fmChan->volume * fmChan->velocity) / 0x7F;
}
