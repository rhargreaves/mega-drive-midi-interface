#include "midi_cc.h"
#include "midi.h"

void midi_cc_raw(u8 chan, u8 controller, u8 value)
{
    midi_cc(chan, controller, value);
}
