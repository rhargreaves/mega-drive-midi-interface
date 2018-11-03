#include <midi.h>
#include <synth.h>

void midi_process(Message* message)
{
    synth_noteOn(0);
}
