#include "samples.h"
#include <types.h>

typedef struct PcmSample PcmSample;

struct PcmSample {
    const u8* data;
    u32 length;
    u8 rate;
};

extern const PcmSample* percussionPcmSample[128];
