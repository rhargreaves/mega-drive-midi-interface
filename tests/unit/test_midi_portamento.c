#include "test_midi.h"

static int test_midi_portamento_setup(UNUSED void** state)
{
    test_midi_setup(state);

    for (u8 chan = 0; chan < MIDI_CHANNELS; chan++) {
        __real_midi_cc(chan, CC_PORTAMENTO_TIME_MSB, 126);
    }
    return 0;
}

static void test_midi_portamento_glides_note_up(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        debug_message("channel %d\n", chan);
        __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

        expect_synth_pitch(chan, 2, 0x439);
        expect_synth_volume_any();
        expect_synth_noteOn(chan);
        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 2, 0x43f);
        midi_tick();
        for (u16 i = 0; i < 148; i++) {
            expect_synth_pitch_any();
            midi_tick();
        }

        expect_synth_pitch(chan, 4, 644);
        midi_tick();

        midi_tick();
        midi_tick();
    }
}

static void test_midi_portamento_glides_note_down(UNUSED void** state)
{
    __real_midi_cc(0, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(0, 4, 644);
    expect_synth_volume_any();
    expect_synth_noteOn(0);

    __real_midi_note_on(0, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
    __real_midi_note_on(0, MIDI_PITCH_A2, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 4, 640);
    midi_tick();
    for (u16 i = 0; i < 148; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    expect_synth_pitch(0, 2, 1081);
    midi_tick();

    midi_tick();
    midi_tick();
}

static void test_midi_portamento_glides_note_up_and_down_on_early_release(UNUSED void** state)
{
    u8 chan = 0;
    debug_message("channel %d\n", chan);
    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 2, 0x43f);
    midi_tick();
    for (u16 i = 0; i < 74; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    debug_message("note off\n");
    __real_midi_note_off(chan, MIDI_PITCH_C4);
    for (u16 i = 0; i < 74; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    expect_synth_pitch(chan, 2, 0x439);
    midi_tick();

    midi_tick();
    midi_tick();
}

static void test_midi_portamento_glide_ignores_unassigned_channels(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        debug_message("channel %d\n", chan);
        __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

        expect_synth_pitch(chan, 2, 0x439);
        expect_synth_volume_any();
        expect_synth_noteOn(chan);
        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        midi_remap_channel(chan, 0x7F);
        midi_tick();
    }
}

static void test_midi_portamento_glides_note_up_down_and_back_up(UNUSED void** state)
{
    u8 chan = 0;
    debug_message("channel %d\n", chan);
    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 2, 0x43f);
    midi_tick();
    for (u16 i = 0; i < 74; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    debug_message("note off\n");
    __real_midi_note_off(chan, MIDI_PITCH_C4);
    for (u16 i = 0; i < 37; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    debug_message("note back on\n");
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
    for (u16 i = 0; i < 111; i++) {
        debug_message("%d\n", i);
        expect_synth_pitch_any();
        midi_tick();
    }

    expect_synth_pitch(chan, 4, 644);
    midi_tick();

    midi_tick();
    midi_tick();
}

static void test_midi_portamento_glides_only_if_target_set(UNUSED void** state)
{
    u8 chan = 0;
    debug_message("channel %d\n", chan);
    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);

    midi_tick();
    midi_tick();
}

static void test_midi_portamento_glide_ends_after_both_notes_off(UNUSED void** state)
{
    u8 chan = 0;
    debug_message("channel %d\n", chan);
    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 2, 0x43f);
    midi_tick();
    for (u16 i = 0; i < 74; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    debug_message("note off\n");
    __real_midi_note_off(chan, MIDI_PITCH_C4);
    for (u16 i = 0; i < 37; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    debug_message("note off\n");
    expect_value(__wrap_synth_noteOff, channel, 0);
    __real_midi_note_off(chan, MIDI_PITCH_A2);

    midi_tick();

    expect_synth_pitch(chan, 3, 0x284);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_C3, MAX_MIDI_VOLUME);

    midi_tick();
    midi_tick();
}

static void test_midi_portamento_glides_fully_up_and_down(UNUSED void** state)
{
    u8 chan = 0;
    debug_message("channel %d\n", chan);
    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 2, 0x43f);
    midi_tick();
    for (u16 i = 0; i < 148; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }
    expect_synth_pitch(chan, 4, 644);
    midi_tick();

    debug_message("note off\n");
    __real_midi_note_off(chan, MIDI_PITCH_C4);
    for (u16 i = 0; i < 148; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }
    expect_synth_pitch(chan, 2, 0x43f);
    midi_tick();
}

static void test_midi_portamento_synth_note_off_triggered(UNUSED void** state)
{
    u8 chan = 0;
    debug_message("channel %d\n", chan);
    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);

    midi_tick();

    debug_message("note off\n");
    expect_value(__wrap_synth_noteOff, channel, chan);
    __real_midi_note_off(chan, MIDI_PITCH_A2);
    midi_tick();
}

static void test_midi_portamento_zeros_any_residual_cents(UNUSED void** state)
{
    const u8 chan = 0;
    debug_message("channel %d\n", chan);
    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 2, 0x43f);
    midi_tick();
    for (u16 i = 0; i < 37; i++) {
        expect_synth_pitch_any();
        midi_tick();
    }

    expect_value(__wrap_synth_noteOff, channel, chan);
    __real_midi_note_off(chan, MIDI_PITCH_C4);
    __real_midi_note_off(chan, MIDI_PITCH_A2);

    midi_tick();

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 2, 0x43f);
    midi_tick();
}

static void test_midi_portamento_glides_note_up_for_psg(UNUSED void** state)
{
    for (u8 chan = MIN_PSG_CHAN; chan <= MAX_PSG_CHAN; chan++) {
        debug_message("channel %d\n", chan);
        u8 psgChan = chan - MIN_PSG_CHAN;
        __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

        expect_psg_attenuation(psgChan, 0);
        expect_psg_tone(psgChan, 0x3f8);
        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        expect_psg_tone(psgChan, 0x3f3);
        midi_tick();
        for (u16 i = 0; i < 148; i++) {
            expect_any_psg_tone();
            midi_tick();
        }

        expect_psg_tone(psgChan, 0x1ab);
        midi_tick();

        midi_tick();
        midi_tick();
    }
}

static void test_midi_portamento_glides_note_up_with_pitch_bend(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        debug_message("channel %d\n", chan);
        __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

        expect_synth_pitch(chan, 2, 0x439);
        expect_synth_volume_any();
        expect_synth_noteOn(chan);
        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 2, 0x47a);
        __real_midi_pitch_bend(chan, 0x3000); // +1st

        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);
        expect_synth_pitch(chan, 2, 0x480);
        midi_tick();

        for (u16 i = 0; i < 148; i++) {
            expect_synth_pitch_any();
            midi_tick();
        }

        expect_synth_pitch(chan, 4, 681);
        midi_tick();

        midi_tick();
        midi_tick();
    }
}

static void test_midi_portamento_glides_note_down_with_pitch_bend(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        debug_message("channel %d\n", chan);
        __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);

        expect_synth_pitch(chan, 4, 0x284);
        expect_synth_volume_any();
        expect_synth_noteOn(chan);
        __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

        expect_synth_pitch(chan, 4, 0x25f);
        __real_midi_pitch_bend(chan, 0x1000); // -1st

        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
        expect_synth_pitch(chan, 3, 0x4b7);
        midi_tick();

        for (u16 i = 0; i < 148; i++) {
            expect_synth_pitch_any();
            midi_tick();
        }

        expect_synth_pitch(chan, 2, 0x3fd);
        midi_tick();

        midi_tick();
        midi_tick();
    }
}

static void test_midi_portamento_sets_portamento_time_to_minimum(UNUSED void** state)
{
    const u8 chan = 0;

    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);
    __real_midi_cc(chan, CC_PORTAMENTO_TIME_MSB, 0);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 3, 0x466);
    midi_tick();

    expect_synth_pitch(chan, 4, 0x495);
    midi_tick();
}

static void test_midi_portamento_sets_portamento_time_to_maximum(UNUSED void** state)
{
    const u8 chan = 0;

    __real_midi_cc(chan, CC_PORTAMENTO_ENABLE, 127);
    __real_midi_cc(chan, CC_PORTAMENTO_TIME_MSB, 127);

    expect_synth_pitch(chan, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(chan);
    __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(chan, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(chan, 2, 0x439);
    midi_tick();

    expect_synth_pitch(chan, 2, 0x439);
    midi_tick();
}
