#include "midi_dac.h"
#include "pcm_sample.h"
#include "snd/sound.h"
#include "snd/pcm/snd_pcm.h"

void midi_dac_note_on(u8 chan, u8 pitch, s8 cents, u8 velocity)
{
    const PcmSample* sample = percussionPcmSample[0];
    SND_PCM_startPlay(sample->data, sample->length, sample->rate, SOUND_PAN_CENTER, 0);
}

void midi_dac_reset(void)
{
}

void midi_dac_note_off(u8 chan, u8 pitch)
{
}

void midi_dac_channel_volume(u8 chan, u8 volume)
{
}

void midi_dac_pan(u8 chan, u8 pan)
{
}

void midi_dac_program(u8 chan, u8 program)
{
}

void midi_dac_all_notes_off(u8 chan)
{
}

void midi_dac_pitch(u8 chan, u8 pitch, s8 cents)
{
}
