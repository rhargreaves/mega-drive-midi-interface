#include "test_midi.h"

static const u16 TONE_C4 = 427;
static const u16 TONE_CSHARP4 = 403;
static const u16 TONE_A2 = 1016;

static void test_midi_triggers_psg_note_on(UNUSED void** state)
{
    const u8 midiKeys[] = { 45, 69, 108 };
    const u16 tones[] = { 1016, 254, 26 };

    for (u8 i = 0; i < sizeof(midiKeys); i++) {
        for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

            u16 expectedFrequency = tones[i];
            u8 expectedMidiKey = midiKeys[i];
            u8 expectedPsgChan = chan - MIN_PSG_CHAN;

            if (i == 0) {
                expect_psg_attenuation(expectedPsgChan, 0);
            }
            expect_psg_tone(expectedPsgChan, expectedFrequency);

            __real_midi_noteOn(chan, expectedMidiKey, 127);
        }
    }
}

static void test_midi_uses_PAL_tones_if_system_is_in_that_region(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    const u16 TONE_PAL_C4 = 423;

    wraps_region_setIsPal(true);

    expect_psg_tone(expectedPsgChan, TONE_PAL_C4);
    expect_psg_attenuation(expectedPsgChan, 0);

    __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME);
}

static void test_midi_triggers_psg_note_on_with_velocity(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any_psg_tone();
    expect_psg_attenuation(expectedPsgChan, 0x3);

    __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_on_with_velocity_and_channel_volume(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);

    expect_any_psg_tone();
    expect_psg_attenuation(expectedPsgChan, 0x7);

    __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME / 2);
}

static void test_midi_changing_volume_during_psg_note_on_respects_velocity(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any_psg_tone();
    expect_psg_attenuation(expectedPsgChan, 0x3);

    __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME / 2);

    expect_psg_attenuation(expectedPsgChan, 0x7);

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_off(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        u8 midiKey = MIDI_PITCH_C4_;

        expect_any_psg_tone();
        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);

        __real_midi_noteOn(chan, midiKey, 127);

        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, midiKey);
    }
}

static void test_midi_drops_psg_key_below_45(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedMidiKey = 44;
        __real_midi_noteOn(chan, expectedMidiKey, 127);
    }
}

static void
test_midi_triggers_psg_note_off_and_volume_change_does_not_cause_psg_channel_to_play(
    UNUSED void** state)
{
    u8 midiKey = MIDI_PITCH_C4_;
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        expect_any_psg_tone();
        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
        __real_midi_noteOn(chan, midiKey, 127);

        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_SILENCE);
        __real_midi_noteOff(chan, midiKey);

        __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);
    }
}

static void test_midi_psg_note_off_only_triggered_if_specific_note_is_on(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;

    expect_psg_attenuation(0, PSG_ATTENUATION_LOUDEST);
    expect_psg_tone(0, TONE_C4);
    __real_midi_noteOn(chan, MIDI_PITCH_C4_, 127);

    expect_psg_tone(0, TONE_CSHARP4);
    __real_midi_noteOn(chan, MIDI_PITCH_CSHARP4_, 127);

    __real_midi_noteOff(chan, MIDI_PITCH_C4_);
}

static void test_midi_channel_volume_sets_psg_attenuation(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);

    expect_psg_tone(0, 1016);
    expect_psg_attenuation(0, 1);

    __real_midi_noteOn(MIN_PSG_CHAN, MIDI_PITCH_A2_, 127);
}

static void test_midi_channel_volume_sets_psg_attenuation_2(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 127);

    expect_psg_tone(0, 1016);
    expect_psg_attenuation(0, 0);

    __real_midi_noteOn(MIN_PSG_CHAN, MIDI_PITCH_A2_, 127);
}

static void test_midi_sets_psg_pitch_bend_down(UNUSED void** state)
{
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        print_message("PSG Chan %d\n", expectedPsgChan);

        expect_psg_tone(expectedPsgChan, TONE_C4);
        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
        __real_midi_noteOn(chan, MIDI_PITCH_C4_, 127);

        expect_psg_tone(expectedPsgChan, 0x1df);
        __real_midi_pitchBend(chan, 0);
    }
}

static void test_midi_sets_psg_pitch_bend_up(UNUSED void** state)
{
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        print_message("PSG Chan %d\n", expectedPsgChan);

        expect_psg_tone(expectedPsgChan, TONE_C4);
        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
        __real_midi_noteOn(chan, MIDI_PITCH_C4_, 127);

        expect_psg_tone(expectedPsgChan, 0x17c);
        __real_midi_pitchBend(chan, 0x4000);
    }
}

static void test_midi_psg_pitch_bend_persists_after_tick(UNUSED void** state)
{
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        print_message("PSG Chan %d\n", expectedPsgChan);

        expect_psg_tone(expectedPsgChan, TONE_C4);
        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
        __real_midi_noteOn(chan, MIDI_PITCH_C4_, 127);

        expect_psg_tone(expectedPsgChan, 0x1d8);
        __real_midi_pitchBend(chan, 1000);

        __real_midi_psg_tick();
    }
}

static void test_midi_plays_psg_envelope(UNUSED void** state)
{
    u8 pitch = MIDI_PITCH_C4_;
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        print_message("PSG Chan %d\n", expectedPsgChan);

        __real_midi_program(chan, 1);

        expect_psg_tone(expectedPsgChan, TONE_C4);
        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
        __real_midi_noteOn(chan, pitch, MAX_MIDI_VOLUME);

        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_SILENCE);
        __real_midi_psg_tick();

        __real_midi_noteOff(chan, pitch);
    }
}

static void test_midi_plays_advanced_psg_envelope(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;

    __real_midi_program(chan, 2);

    expect_psg_tone(expectedPsgChan, TONE_C4);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME);

    expect_psg_attenuation(expectedPsgChan, 7);
    __real_midi_psg_tick();

    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_SILENCE);
    __real_midi_psg_tick();
}

static void test_midi_loops_psg_envelope(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;

    __real_midi_program(chan, 4);

    expect_psg_tone(expectedPsgChan, TONE_C4);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME);

    expect_psg_attenuation(expectedPsgChan, 7);
    __real_midi_psg_tick();

    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_psg_tick();

    expect_psg_attenuation(expectedPsgChan, 7);
    __real_midi_psg_tick();
}

static void test_midi_psg_envelope_with_only_end_flag_is_silent(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;

    __real_midi_program(chan, 5);
    __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME);
    __real_midi_psg_tick();
    __real_midi_psg_tick();
}

static void
test_midi_psg_envelope_with_loop_end_continues_playing_after_note_off(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;
    u8 pitch = MIDI_PITCH_C4_;

    __real_midi_program(chan, 6);

    expect_psg_tone(expectedPsgChan, TONE_C4);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, pitch, MAX_MIDI_VOLUME);

    __real_midi_noteOff(chan, pitch);

    expect_psg_attenuation(expectedPsgChan, 0x05);
    __real_midi_psg_tick();

    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_SILENCE);
    __real_midi_psg_tick();
}

static void
test_midi_psg_envelope_with_loop_end_resets_release_note_after_note_silenced(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;
    u8 pitch = MIDI_PITCH_C4_;

    __real_midi_program(chan, 6);

    expect_psg_tone(expectedPsgChan, TONE_C4);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, pitch, MAX_MIDI_VOLUME);

    __real_midi_noteOff(chan, pitch);

    expect_psg_attenuation(expectedPsgChan, 0x05);
    __real_midi_psg_tick();

    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_SILENCE);
    __real_midi_psg_tick();

    // Play again
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, pitch, MAX_MIDI_VOLUME);

    __real_midi_psg_tick();
}

#define SHIFTS 14

static void test_midi_shifts_semitone_in_psg_envelope(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;

    const u16 expectedInitialTone = TONE_C4;
    const u16 expectedShiftedTone[SHIFTS]
        = { /* Up */ 0x1aa, 0x1a9, 0x1a7, 0x19f, 0x193, 0x17c, 0x140,
              /* Down */ 0x1ac, 0x1ad, 0x1b0, 0x1b7, 0x1c4, 0x1df, 0x23a };

    for (u8 i = 0; i < SHIFTS; i++) {
        u8 envelopeStep = (i + 1) * 0x10;
        const u8 envelope[] = { EEF_LOOP_START, 0x00, envelopeStep, EEF_END };
        const u8* envelopes[] = { envelope };

        print_message("Shift: %d %d\n", i, envelopeStep);
        midi_psg_init(envelopes);

        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
        expect_psg_tone(expectedPsgChan, expectedInitialTone);
        __real_midi_noteOn(chan, MIDI_PITCH_C4_, MAX_MIDI_VOLUME);

        expect_psg_tone(expectedPsgChan, expectedShiftedTone[i]);
        __real_midi_psg_tick();

        expect_psg_tone(expectedPsgChan, expectedInitialTone);
        __real_midi_psg_tick();
    }
}

static void test_midi_pitch_shift_handles_upper_limit_psg_envelope(
    UNUSED void** state)
{
    const u8 chan = MIN_PSG_CHAN;
    const u8 expectedPsgChan = 0;
    const u8 maxPitch = 127;
    const u16 expectedInitialTone = 8;
    const u8 envelope[] = { EEF_LOOP_START, 0x00, 0x10, EEF_END };
    const u8* envelopes[] = { envelope };
    midi_psg_init(envelopes);

    expect_psg_tone(expectedPsgChan, expectedInitialTone);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, maxPitch, MAX_MIDI_VOLUME);

    __real_midi_psg_tick();
}

static void test_midi_pitch_shift_handles_lower_limit_psg_envelope(
    UNUSED void** state)
{
    const u8 chan = MIN_PSG_CHAN;
    const u8 expectedPsgChan = 0;
    const u8 minPitch = MIDI_PITCH_A2_;
    const u16 expectedInitialTone = TONE_A2;
    const u8 envelope[] = { EEF_LOOP_START, 0x00, 0x80, EEF_END };
    const u8* envelopes[] = { envelope };
    midi_psg_init(envelopes);

    expect_psg_tone(expectedPsgChan, expectedInitialTone);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, minPitch, MAX_MIDI_VOLUME);

    __real_midi_psg_tick();
}

static void test_midi_loads_psg_envelope(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;

    const u8 eef[] = { 0x66, EEF_END };

    __real_midi_psg_loadEnvelope(eef);

    expect_psg_tone(0, 0x17c);
    expect_psg_attenuation(0, 6);

    __real_midi_noteOn(chan, MIDI_PITCH_C4_, 127);

    expect_psg_attenuation(0, PSG_ATTENUATION_SILENCE);

    __real_midi_psg_tick();
}

static void test_psg_sets_busy_indicators(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan++) {
        print_message("Chan %d\n", chan);
        expect_psg_tone(chan, TONE_C4);
        expect_psg_attenuation(chan, PSG_ATTENUATION_LOUDEST);

        __real_midi_noteOn(
            chan + MIN_PSG_CHAN, MIDI_PITCH_C4_, MAX_MIDI_VOLUME);
    }

    for (u8 chan = 0; chan < MAX_PSG_CHANS; chan += 2) {
        expect_psg_attenuation(chan, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan + MIN_PSG_CHAN, MIDI_PITCH_C4_);
    }

    u8 busy = psg_busy();
    assert_int_equal(busy, 0b1010);
}
