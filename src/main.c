#include <genesis.h>
#include <interface.h>
#include <main.h>
#include <synth.h>

static const u16 MAX_X = 40;
static const char HEADER[] = "Sega Mega Drive MIDI Interface";

void vsync(void);

int main(void)
{
    SYS_setVIntCallback(vsync);
    VDP_drawText(HEADER, (MAX_X - sizeof(HEADER)) / 2, 2);
    VDP_drawText(BUILD, (MAX_X - sizeof(BUILD)) / 2, 3);
    interface_init();
    while (TRUE) {
        interface_tick();
    }
}

void vsync(void)
{
    char* lastError = interface_lastError();
    if (lastError != NULL) {
        VDP_setTextPalette(PAL1);
        VDP_drawText(lastError, 1, 5);
        interface_clearError();
    }
}
