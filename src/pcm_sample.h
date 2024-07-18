#include "types.h"
#include "samples.h"

typedef struct PcmSample {
    const u8* data;
    u32 length;
    u8 rate;
} PcmSample;

extern const PcmSample* percussionPcmSample[128];
