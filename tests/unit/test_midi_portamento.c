#include "test_midi.h"

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
