#include "midi_fm_sm.h"
#include "midi_fm.h"
#include "midi.h"

#define SM_OP_LEN 3

typedef struct SpecialModeOperator {
    u8 pitch;
    s8 cents;
} SpecialModeOperator;

static SpecialModeOperator smOperators[SM_OP_LEN];

void midi_fm_sm_note_on(u8 op, u8 pitch, s8 cents, u8 velocity)
{
    midi_fm_sm_pitch(op, pitch, cents);
    synth_specialModeVolume(op, velocity);
}

void midi_fm_sm_pitch(u8 op, u8 pitch, s8 cents)
{
    SpecialModeOperator* smOp = &smOperators[op];
    smOp->pitch = pitch;
    smOp->cents = cents;

    synth_specialModePitch(op, midi_fm_pitchToOctave(smOp->pitch),
        midi_fm_pitchCentsToFreqNum(smOp->pitch, smOp->cents));
}

void midi_fm_sm_reset(void)
{
    for (u8 op = 0; op < SM_OP_LEN; op++) {
        SpecialModeOperator* smOp = &smOperators[op];
        smOp->pitch = 0;
        smOp->cents = 0;
    }
}

// no-ops
void midi_fm_sm_note_off(u8 chan, u8 pitch)
{
}

void midi_fm_sm_channel_volume(u8 chan, u8 volume)
{
}

void midi_fm_sm_pan(u8 chan, u8 pan)
{
}

void midi_fm_sm_program(u8 chan, u8 program)
{
}

void midi_fm_sm_all_notes_off(u8 chan)
{
}
