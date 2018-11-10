#include "interface.h"
#include "ui.h"

int main(void)
{
    ui_init();
    interface_init();
    interface_loop();
}
