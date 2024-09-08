#include "test_midi_finetune.h"
#include "test_midi.h"

void test_midi_finetune_max(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        debug_message("channel %d\n", chan);
        __real_midi_cc(chan, CC_FINE_TUNE, 127);

        expect_synth_pitch(chan, 2, 0x461);
        expect_synth_volume_any();
        expect_synth_noteOn(chan);
        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    }
}

void test_midi_finetune_min(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        debug_message("channel %d\n", chan);
        __real_midi_cc(chan, CC_FINE_TUNE, 0);

        expect_synth_pitch(chan, 2, 0x412);
        expect_synth_volume_any();
        expect_synth_noteOn(chan);
        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    }
}

void test_midi_finetune_with_pitchbend(UNUSED void** state)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        debug_message("channel %d\n", chan);
        __real_midi_cc(chan, CC_FINE_TUNE, 0);

        expect_synth_pitch(chan, 2, 0x492);
        __real_midi_pitch_bend(chan, MIDI_PITCH_BEND_MAX);

        expect_synth_pitch(chan, 2, 0x492);
        expect_synth_volume_any();
        expect_synth_noteOn(chan);
        __real_midi_note_on(chan, MIDI_PITCH_A2, MAX_MIDI_VOLUME);
    }
}
