#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <midi.h>

static void test_midi_triggers_synth_note_on(void** state)
{
    expect_value(__wrap_synth_noteOn, channel, 0);

    Message noteOn = { 0b10010000, 0x40, 127 };

    __real_midi_process(&noteOn);
}

static void test_midi_triggers_synth_note_off(void** state)
{
    expect_value(__wrap_synth_noteOff, channel, 0);

    Message noteOn = { 0b10000000, 0x40, 127 };

    __real_midi_process(&noteOn);
}
