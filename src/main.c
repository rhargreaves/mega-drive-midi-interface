#include "interface.h"
#include "midi.h"
#include "synth.h"
#include "sys.h"
#include "ui.h"

static void vsync(void);

int main(void)
{
    synth_init();
    ui_init();
    midi_init();
    interface_init();
    SYS_setVIntCallback(vsync);
    interface_loop();
}

static void vsync(void)
{
    ui_vsync();
}
