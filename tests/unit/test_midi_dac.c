#include "test_midi.h"
#include "snd/sound.h"
#include "snd/pcm/snd_pcm.h"

static int test_midi_dac_setup(UNUSED void** state)
{
    test_midi_setup(state);

    expect_value(__wrap_synth_enableDac, enable, true);
    __real_midi_cc(0, 78, 0x7F);

    return 0;
}

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

static void test_midi_dac_plays_note(UNUSED void** state)
{
    expect_any(__wrap_SND_PCM_startPlay, sample);
    expect_any(__wrap_SND_PCM_startPlay, len);
    expect_any(__wrap_SND_PCM_startPlay, rate);
    expect_value(__wrap_SND_PCM_startPlay, pan, SOUND_PAN_CENTER);
    expect_value(__wrap_SND_PCM_startPlay, loop, 0);

    __real_midi_note_on(5, 100, 100);
}

static void test_midi_dac_stops_note(UNUSED void** state)
{
    expect_any(__wrap_SND_PCM_startPlay, sample);
    expect_any(__wrap_SND_PCM_startPlay, len);
    expect_any(__wrap_SND_PCM_startPlay, rate);
    expect_value(__wrap_SND_PCM_startPlay, pan, SOUND_PAN_CENTER);
    expect_value(__wrap_SND_PCM_startPlay, loop, 0);

    __real_midi_note_on(5, 100, 100);

    expect_function_call(__wrap_SND_PCM_stopPlay);

    __real_midi_note_off(5, 100);
}
