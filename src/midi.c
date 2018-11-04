#include <midi.h>
#include <synth.h>

void midi_process(Message* message)
{
    if ((message->status & 0b10010000) == 0b10010000) {
        synth_noteOn(0);
    } else if ((message->status & 0b10000000) == 0b10000000) {
        synth_noteOff(0);
    }
}
