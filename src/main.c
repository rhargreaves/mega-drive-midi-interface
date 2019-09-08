#include "interface.h"
#include "sys.h"
#include "ui.h"
#include "midi_psg.h"

static void vsync(void);

int main(void)
{
    ui_init();
    SYS_setVIntCallback(vsync);
    midi_psg_init();
    interface_init();
    interface_loop();
}

static void vsync(void)
{
    ui_vsync();
}
