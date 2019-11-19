#include "test_midi.h"

static int test_dynamic_midi_setup(UNUSED void** state)
{
    test_midi_setup(state);

    const u8 SYSEX_EXTENDED_MANU_ID_SECTION = 0x00;
    const u8 SYSEX_UNUSED_EUROPEAN_SECTION = 0x22;
    const u8 SYSEX_UNUSED_MANU_ID = 0x77;
    const u8 SYSEX_DYNAMIC_COMMAND_ID = 0x03;
    const u8 SYSEX_DYNAMIC_ENABLED = 0x01;

    const u8 sequence[] = {
        SYSEX_EXTENDED_MANU_ID_SECTION,
        SYSEX_UNUSED_EUROPEAN_SECTION,
        SYSEX_UNUSED_MANU_ID,
        SYSEX_DYNAMIC_COMMAND_ID,
        SYSEX_DYNAMIC_ENABLED,
    };

    __real_midi_sysex(sequence, sizeof(sequence));

    return 0;
}

static void test_midi_dynamic_uses_all_fm_channels(UNUSED void** state)
{
    const u8 octave = 4;
    const u16 freq = 0x28d;
    const u16 pitch = 60;

    for (u16 i = DEV_CHAN_MIN_FM; i < DEV_CHAN_MAX_FM; i++) {
        expect_synth_pitch(i, octave, freq);
        expect_synth_volume_any();
        expect_value(__wrap_synth_noteOn, channel, i);

        __real_midi_noteOn(0, pitch, 127);
    }
}
