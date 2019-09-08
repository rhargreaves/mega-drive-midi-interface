#include "interface.h"
#include "midi_fm.h"
#include "midi_psg.h"
#include "sys.h"
#include "ui.h"

static void vsync(void);

int main(void)
{
    ui_init();
    SYS_setVIntCallback(vsync);
    midi_psg_init();
    midi_fm_init();
    interface_init();
    interface_loop();
}

static void vsync(void)
{
    ui_vsync();
}
