#include "interface.h"
#include "sys.h"
#include "ui.h"

static void vsync(void);

int main(void)
{
    ui_init();
    SYS_setVIntCallback(vsync);
    interface_init();
    interface_loop();
}

static void vsync(void)
{
    ui_vsync();
}
