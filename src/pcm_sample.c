#include "pcm_sample.h"
#include "samples.h"
#include "snd/sound.h"
#include "snd/pcm/snd_pcm.h"

// clang-format off
static const PcmSample bellride = { pcm_BellRide, sizeof(pcm_BellRide), SOUND_PCM_RATE_22050 };
static const PcmSample belltree = { pcm_BellTree, sizeof(pcm_BellTree), SOUND_PCM_RATE_22050 };
static const PcmSample bongorim = { pcm_BongoRim, sizeof(pcm_BongoRim), SOUND_PCM_RATE_22050 };
static const PcmSample cabasa = { pcm_Cabasa, sizeof(pcm_Cabasa), SOUND_PCM_RATE_22050 };
static const PcmSample castanet = { pcm_Castanet, sizeof(pcm_Castanet), SOUND_PCM_RATE_22050 };
static const PcmSample chinacrash = { pcm_ChinaCrash, sizeof(pcm_ChinaCrash), SOUND_PCM_RATE_22050 };
static const PcmSample clap = { pcm_Clap, sizeof(pcm_Clap), SOUND_PCM_RATE_22050 };
static const PcmSample clave = { pcm_Clave, sizeof(pcm_Clave), SOUND_PCM_RATE_22050 };
static const PcmSample closedslap = { pcm_ClosedSlap, sizeof(pcm_ClosedSlap), SOUND_PCM_RATE_22050 };
static const PcmSample cowbell = { pcm_CowBell, sizeof(pcm_CowBell), SOUND_PCM_RATE_22050 };
static const PcmSample crsh1 = { pcm_Crsh1, sizeof(pcm_Crsh1), SOUND_PCM_RATE_22050 };
static const PcmSample crsh2 = { pcm_Crsh2, sizeof(pcm_Crsh2), SOUND_PCM_RATE_22050 };
static const PcmSample flrstudio = { pcm_FlrStudio, sizeof(pcm_FlrStudio), SOUND_PCM_RATE_22050 };
static const PcmSample guirodown = { pcm_GuiroDown, sizeof(pcm_GuiroDown), SOUND_PCM_RATE_22050 };
static const PcmSample guiroup = { pcm_GuiroUp, sizeof(pcm_GuiroUp), SOUND_PCM_RATE_22050 };
static const PcmSample histudio = { pcm_HiStudio, sizeof(pcm_HiStudio), SOUND_PCM_RATE_22050 };
static const PcmSample hi_hatclosed = { pcm_Hi_HatClosed, sizeof(pcm_Hi_HatClosed), SOUND_PCM_RATE_22050 };
static const PcmSample hi_hatfoot = { pcm_Hi_HatFoot, sizeof(pcm_Hi_HatFoot), SOUND_PCM_RATE_22050 };
static const PcmSample hi_hathalf_open = { pcm_Hi_HatHalf_Open, sizeof(pcm_Hi_HatHalf_Open), SOUND_PCM_RATE_22050 };
static const PcmSample highagogo = { pcm_HighAgogo, sizeof(pcm_HighAgogo), SOUND_PCM_RATE_22050 };
static const PcmSample highconga = { pcm_HighConga, sizeof(pcm_HighConga), SOUND_PCM_RATE_22050 };
static const PcmSample highq = { pcm_HighQ, sizeof(pcm_HighQ), SOUND_PCM_RATE_22050 };
static const PcmSample hightimbale = { pcm_HighTimbale, sizeof(pcm_HighTimbale), SOUND_PCM_RATE_22050 };
static const PcmSample highwoodblock = { pcm_HighWoodblock, sizeof(pcm_HighWoodblock), SOUND_PCM_RATE_22050 };
static const PcmSample longtaikohit = { pcm_LongTaikoHit, sizeof(pcm_LongTaikoHit), SOUND_PCM_RATE_22050 };
static const PcmSample longwhistle = { pcm_LongWhistle, sizeof(pcm_LongWhistle), SOUND_PCM_RATE_22050 };
static const PcmSample lowagogo = { pcm_LowAgogo, sizeof(pcm_LowAgogo), SOUND_PCM_RATE_22050 };
static const PcmSample lowbongo = { pcm_LowBongo, sizeof(pcm_LowBongo), SOUND_PCM_RATE_22050 };
static const PcmSample lowflrstudio = { pcm_LowFlrStudio, sizeof(pcm_LowFlrStudio), SOUND_PCM_RATE_22050 };
static const PcmSample lowstudio = { pcm_LowStudio, sizeof(pcm_LowStudio), SOUND_PCM_RATE_22050 };
static const PcmSample lowtimbale = { pcm_LowTimbale, sizeof(pcm_LowTimbale), SOUND_PCM_RATE_22050 };
static const PcmSample lowtumba = { pcm_LowTumba, sizeof(pcm_LowTumba), SOUND_PCM_RATE_22050 };
static const PcmSample lowwoodblock = { pcm_LowWoodblock, sizeof(pcm_LowWoodblock), SOUND_PCM_RATE_22050 };
static const PcmSample maracas = { pcm_Maracas, sizeof(pcm_Maracas), SOUND_PCM_RATE_22050 };
static const PcmSample mdhistudio = { pcm_MdHiStudio, sizeof(pcm_MdHiStudio), SOUND_PCM_RATE_22050 };
static const PcmSample mdstudio = { pcm_MdStudio, sizeof(pcm_MdStudio), SOUND_PCM_RATE_22050 };
static const PcmSample metronomebell = { pcm_MetronomeBell, sizeof(pcm_MetronomeBell), SOUND_PCM_RATE_22050 };
static const PcmSample metronomeclick = { pcm_MetronomeClick, sizeof(pcm_MetronomeClick), SOUND_PCM_RATE_22050 };
// static const PcmSample piccolo1_1 = { pcm_Piccolo1_1, sizeof(pcm_Piccolo1_1), SOUND_PCM_RATE_22050 };
// static const PcmSample piccolo1_2 = { pcm_Piccolo1_2, sizeof(pcm_Piccolo1_2), SOUND_PCM_RATE_22050 };
static const PcmSample quicahi = { pcm_QuicaHi, sizeof(pcm_QuicaHi), SOUND_PCM_RATE_22050 };
static const PcmSample quicalow = { pcm_QuicaLow, sizeof(pcm_QuicaLow), SOUND_PCM_RATE_22050 };
static const PcmSample ride1 = { pcm_Ride1, sizeof(pcm_Ride1), SOUND_PCM_RATE_22050 };
static const PcmSample ride2 = { pcm_Ride2, sizeof(pcm_Ride2), SOUND_PCM_RATE_22050 };
static const PcmSample rimtap = { pcm_RimTap, sizeof(pcm_RimTap), SOUND_PCM_RATE_22050 };
static const PcmSample scratchpull = { pcm_ScratchPull, sizeof(pcm_ScratchPull), SOUND_PCM_RATE_22050 };
static const PcmSample scratchpush = { pcm_ScratchPush, sizeof(pcm_ScratchPush), SOUND_PCM_RATE_22050 };
static const PcmSample shorttaikohit = { pcm_ShortTaikoHit, sizeof(pcm_ShortTaikoHit), SOUND_PCM_RATE_22050 };
static const PcmSample shortwhistle = { pcm_ShortWhistle, sizeof(pcm_ShortWhistle), SOUND_PCM_RATE_22050 };
static const PcmSample slap = { pcm_Slap, sizeof(pcm_Slap), SOUND_PCM_RATE_22050 };
static const PcmSample sleighbells = { pcm_SleighBells, sizeof(pcm_SleighBells), SOUND_PCM_RATE_22050 };
static const PcmSample smallshaker = { pcm_SmallShaker, sizeof(pcm_SmallShaker), SOUND_PCM_RATE_22050 };
// static const PcmSample snr1_1 = { pcm_Snr1_1, sizeof(pcm_Snr1_1), SOUND_PCM_RATE_22050 };
// static const PcmSample snr1_2 = { pcm_Snr1_2, sizeof(pcm_Snr1_2), SOUND_PCM_RATE_22050 };
// static const PcmSample snr2_1 = { pcm_Snr2_1, sizeof(pcm_Snr2_1), SOUND_PCM_RATE_22050 };
// static const PcmSample snr2_2 = { pcm_Snr2_2, sizeof(pcm_Snr2_2), SOUND_PCM_RATE_22050 };
static const PcmSample splash = { pcm_Splash, sizeof(pcm_Splash), SOUND_PCM_RATE_22050 };
static const PcmSample squareclick = { pcm_SquareClick, sizeof(pcm_SquareClick), SOUND_PCM_RATE_22050 };
static const PcmSample stdkick = { pcm_StdKick, sizeof(pcm_StdKick), SOUND_PCM_RATE_22050 };
// static const PcmSample stdkick1 = { pcm_StdKick1, sizeof(pcm_StdKick1), SOUND_PCM_RATE_22050 };
// static const PcmSample stdkick2 = { pcm_StdKick2, sizeof(pcm_StdKick2), SOUND_PCM_RATE_22050 };
// static const PcmSample stdkick3 = { pcm_StdKick3, sizeof(pcm_StdKick3), SOUND_PCM_RATE_22050 };
// static const PcmSample stdkick4 = { pcm_StdKick4, sizeof(pcm_StdKick4), SOUND_PCM_RATE_22050 };
// static const PcmSample stdkick5 = { pcm_StdKick5, sizeof(pcm_StdKick5), SOUND_PCM_RATE_22050 };
// static const PcmSample stdkick6 = { pcm_StdKick6, sizeof(pcm_StdKick6), SOUND_PCM_RATE_22050 };
// static const PcmSample stdkick7 = { pcm_StdKick7, sizeof(pcm_StdKick7), SOUND_PCM_RATE_22050 };
static const PcmSample stdsnr1 = { pcm_StdSnr1, sizeof(pcm_StdSnr1), SOUND_PCM_RATE_22050 };
static const PcmSample stdsnr2 = { pcm_StdSnr2, sizeof(pcm_StdSnr2), SOUND_PCM_RATE_22050 };
static const PcmSample sticks = { pcm_Sticks, sizeof(pcm_Sticks), SOUND_PCM_RATE_22050 };
static const PcmSample tambourine = { pcm_Tambourine, sizeof(pcm_Tambourine), SOUND_PCM_RATE_22050 };
static const PcmSample triangle = { pcm_Triangle, sizeof(pcm_Triangle), SOUND_PCM_RATE_22050 };
static const PcmSample vibraslap = { pcm_VibraSlap, sizeof(pcm_VibraSlap), SOUND_PCM_RATE_22050 };

const PcmSample* percussionPcmSample[128] = {
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, // 25
    NULL, &highq, &slap, &scratchpush, &scratchpull,
    &sticks, &squareclick, &metronomeclick, &metronomebell, &stdkick,
    &stdkick, &rimtap, &stdsnr1, &clap, &stdsnr2,
    &lowflrstudio, &hi_hatclosed, &flrstudio, &hi_hatfoot, &lowstudio,
    &hi_hathalf_open, &mdstudio, &mdhistudio, &crsh1, &histudio, // 50
    &ride1, &chinacrash, &bellride, &tambourine, &splash,
    &cowbell, &crsh2, &vibraslap, &ride2, &bongorim,
    &lowbongo, &closedslap, &highconga, &lowtumba, &hightimbale,
    &lowtimbale, &highagogo, &lowagogo, &cabasa, &maracas,
    &shortwhistle, &longwhistle, &guirodown, &guiroup, &clave, // 75
    &highwoodblock, &lowwoodblock, &quicahi, &quicalow, &triangle,
    &triangle, &smallshaker, &sleighbells, &belltree, &castanet,
    &shorttaikohit, &longtaikohit, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, // 100
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL
};
