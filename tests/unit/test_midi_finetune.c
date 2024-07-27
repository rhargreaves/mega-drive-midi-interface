#include "test_midi.h"

static void test_midi_finetune_max(UNUSED void** state)
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

static void test_midi_finetune_min(UNUSED void** state)
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
