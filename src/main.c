#include "interface.h"
#include "ui.h"
#include <genesis.h>

int main(void)
{
    ui_init();
    interface_init();
    interface_loop();
}
