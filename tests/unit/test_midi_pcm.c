#include "test_midi.h"

static void test_midi_enables_dac(UNUSED void** state)
{
    expect_value(__wrap_synth_enableDac, enable, true);

    __real_midi_cc(0, 78, 0x7F);
}

static void test_midi_disables_dac(UNUSED void** state)
{
    expect_value(__wrap_synth_enableDac, enable, false);

    __real_midi_cc(0, 78, 0);
}
