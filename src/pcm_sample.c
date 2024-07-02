#include "pcm_sample.h"
#include "sound.h"

static const PcmSample clap = { Clap, sizeof(Clap), SOUND_RATE_22050 };

const PcmSample* percussionPcmSample[128] = { &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap, &clap,
    &clap, &clap };
