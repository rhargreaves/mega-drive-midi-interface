#include "test_midi.h"

static void test_midi_portamento_glides_note_up(UNUSED void** state)
{
    __real_midi_cc(0, CC_PORTAMENTO_ENABLE, 127);

    expect_synth_pitch(0, 2, 0x439);
    expect_synth_volume_any();
    expect_synth_noteOn(0);
    __real_midi_note_on(0, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    __real_midi_note_on(0, MIDI_PITCH_C4, MAX_MIDI_VOLUME);

    expect_synth_pitch(0, 2, 0x43f);
    midi_tick();
    for (u16 i = 0; i < 148; i++) {
        print_message("%d\n", i);
        expect_synth_pitch_any();
        midi_tick();
    }

    expect_synth_pitch(0, 4, 644);
    midi_tick();

    midi_tick();
    midi_tick();
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
        print_message("%d\n", i);
        expect_synth_pitch_any();
        midi_tick();
    }

    expect_synth_pitch(0, 2, 1081);
    midi_tick();

    midi_tick();
    midi_tick();
}
