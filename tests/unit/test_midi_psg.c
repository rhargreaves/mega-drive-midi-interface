#include "test_midi.h"

static void test_midi_triggers_psg_note_on(UNUSED void** state)
{
    const u8 midiKeys[] = { 45, 69, 108 };
    const u16 freqs[] = { 110, 440, 4186 };

    for (u8 i = 0; i < sizeof(midiKeys); i++) {
        for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

            u16 expectedFrequency = freqs[i];
            u8 expectedMidiKey = midiKeys[i];
            u8 expectedPsgChan = chan - MIN_PSG_CHAN;

            if (i == 0) {
                expect_psg_attenuation(expectedPsgChan, 0);
            }
            expect_psg_frequency(expectedPsgChan, expectedFrequency);

            __real_midi_noteOn(chan, expectedMidiKey, 127);
        }
    }
}

static void test_midi_triggers_psg_note_on_with_velocity(UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any_psg_frequency();
    expect_psg_attenuation(expectedPsgChan, 0x3);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_on_with_velocity_and_channel_volume(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);

    expect_any_psg_frequency();
    expect_psg_attenuation(expectedPsgChan, 0x7);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);
}

static void test_midi_changing_volume_during_psg_note_on_respects_velocity(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = chan - MIN_PSG_CHAN;

    expect_any_psg_frequency();
    expect_psg_attenuation(expectedPsgChan, 0x3);

    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME / 2);

    expect_psg_attenuation(expectedPsgChan, 0x7);

    __real_midi_cc(chan, CC_VOLUME, MAX_MIDI_VOLUME / 2);
}

static void test_midi_triggers_psg_note_off(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;
        u8 midiKey = 60;

        expect_any_psg_frequency();
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
    u8 midiKey = 60;
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        expect_any(__wrap_psg_frequency, channel);
        expect_any(__wrap_psg_frequency, freq);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_LOUDEST);

        __real_midi_noteOn(chan, midiKey, 127);

        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(
            __wrap_psg_attenuation, attenuation, PSG_ATTENUATION_SILENCE);

        __real_midi_noteOff(chan, midiKey);

        __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);
    }
}

static void test_midi_psg_note_off_only_triggered_if_specific_note_is_on(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;

    expect_psg_attenuation(0, PSG_ATTENUATION_LOUDEST);
    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 0x106);

    __real_midi_noteOn(chan, 60, 127);

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 0x115);

    __real_midi_noteOn(chan, 61, 127);

    __real_midi_noteOff(chan, 60);
}

static void test_midi_channel_volume_sets_psg_attenuation(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 96);

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 110);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, 1);

    __real_midi_noteOn(MIN_PSG_CHAN, 45, 127);
}

static void test_midi_channel_volume_sets_psg_attenuation_2(UNUSED void** state)
{
    __real_midi_cc(MIN_PSG_CHAN, CC_VOLUME, 127);

    expect_value(__wrap_psg_frequency, channel, 0);
    expect_value(__wrap_psg_frequency, freq, 110);
    expect_value(__wrap_psg_attenuation, channel, 0);
    expect_value(__wrap_psg_attenuation, attenuation, 0);

    __real_midi_noteOn(MIN_PSG_CHAN, 45, 127);
}

static void test_midi_sets_psg_pitch_bend(UNUSED void** state)
{
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
        expect_value(__wrap_psg_frequency, freq, 262);
        expect_value(__wrap_psg_attenuation, channel, expectedPsgChan);
        expect_value(__wrap_psg_attenuation, attenuation, 0);

        __real_midi_noteOn(chan, 60, 127);

        expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
        expect_value(__wrap_psg_frequency, freq, 191);

        __real_midi_pitchBend(chan, 1000);
    }
}

static void test_midi_plays_psg_envelope(UNUSED void** state)
{
    u8 pitch = 60;
    for (int chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {

        u8 expectedPsgChan = chan - MIN_PSG_CHAN;

        print_message("PSG Chan %d\n", expectedPsgChan);

        __real_midi_program(chan, 1);

        expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
        expect_value(__wrap_psg_frequency, freq, 262);
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

    expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
    expect_value(__wrap_psg_frequency, freq, 262);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME);

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

    expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
    expect_value(__wrap_psg_frequency, freq, 262);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME);

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
    u8 expectedPsgChan = 0;

    __real_midi_program(chan, 5);

    expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
    expect_value(__wrap_psg_frequency, freq, 262);
    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME);
    __real_midi_psg_tick();
}

static void test_midi_psg_envelope_with_end_flag_sends_note_off(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;

    __real_midi_program(chan, 5);

    expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
    expect_value(__wrap_psg_frequency, freq, 262);
    __real_midi_noteOn(chan, 60, MAX_MIDI_VOLUME);

    __real_midi_psg_tick();
    __real_midi_psg_tick();
}

static void
test_midi_psg_envelope_with_loop_end_continues_playing_after_note_off(
    UNUSED void** state)
{
    u8 chan = MIN_PSG_CHAN;
    u8 expectedPsgChan = 0;
    u8 pitch = 60;

    __real_midi_program(chan, 6);

    expect_value(__wrap_psg_frequency, channel, expectedPsgChan);
    expect_value(__wrap_psg_frequency, freq, 262);
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
    u8 pitch = 60;

    __real_midi_program(chan, 6);

    expect_psg_frequency(expectedPsgChan, 262);
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

    const u16 expectedInitialFreq = 0x749;
    const u16 expectedShiftedFreq[SHIFTS]
        = { /* Up */ 0x74e, 0x753, 0x75d, 0x77b, 0x7ad, 0x811, 0x93d,
              /* Down */ 0x744, 0x73f, 0x735, 0x717, 0x6e5, 0x681, 0x555 };

    for (u8 i = 0; i < SHIFTS; i++) {
        u8 envelopeStep = (i + 1) * 0x10;
        const u8 envelope[] = { EEF_LOOP_START, 0x00, envelopeStep, EEF_END };
        const u8* envelopes[] = { envelope };

        print_message("Shift: %d\n", i);
        midi_psg_init(envelopes);

        expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
        expect_psg_frequency(expectedPsgChan, expectedInitialFreq);
        __real_midi_noteOn(chan, A_SHARP, MAX_MIDI_VOLUME);

        expect_psg_frequency(expectedPsgChan, expectedShiftedFreq[i]);
        __real_midi_psg_tick();

        expect_psg_frequency(expectedPsgChan, expectedInitialFreq);
        __real_midi_psg_tick();
    }
}

static void test_midi_pitch_shift_handles_upper_limit_psg_envelope(
    UNUSED void** state)
{
    const u8 chan = MIN_PSG_CHAN;
    const u8 expectedPsgChan = 0;
    const u8 maxPitch = 127;
    const u16 expectedInitialFreq = 0x3100;
    const u8 envelope[] = { EEF_LOOP_START, 0x00, 0x10, EEF_END };
    const u8* envelopes[] = { envelope };
    midi_psg_init(envelopes);

    expect_psg_frequency(expectedPsgChan, expectedInitialFreq);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, maxPitch, MAX_MIDI_VOLUME);

    __real_midi_psg_tick();
}

static void test_midi_pitch_shift_handles_lower_limit_psg_envelope(
    UNUSED void** state)
{
    const u8 chan = MIN_PSG_CHAN;
    const u8 expectedPsgChan = 0;
    const u8 minPitch = 45;
    const u16 expectedInitialFreq = 0x6e;
    const u8 envelope[] = { EEF_LOOP_START, 0x00, 0x80, EEF_END };
    const u8* envelopes[] = { envelope };
    midi_psg_init(envelopes);

    expect_psg_frequency(expectedPsgChan, expectedInitialFreq);
    expect_psg_attenuation(expectedPsgChan, PSG_ATTENUATION_LOUDEST);
    __real_midi_noteOn(chan, minPitch, MAX_MIDI_VOLUME);

    __real_midi_psg_tick();
}
