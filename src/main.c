#include "midi.h"
#include "midi_receiver.h"
#include "synth.h"
#include "sys.h"
#include "ui.h"

static void vsync(void);

int main(void)
{
    synth_init();
    ui_init();
    midi_init();
    midi_receiver_init();
    SYS_setVIntCallback(vsync);
    midi_receiver_perpectual_read();
}

static void vsync(void)
{
    ui_vsync();
}
