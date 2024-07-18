#include "pcm_sample.h"
#include "snd/sound.h"
#include "snd/pcm/snd_pcm.h"

static const PcmSample clap = { Clap, sizeof(Clap), SOUND_PCM_RATE_22050 };

const PcmSample* percussionPcmSample[128]
    = { &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
          &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap };
