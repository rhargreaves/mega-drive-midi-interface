#include "ui.h"
#include "comm.h"
#include "interface.h"
#include <genesis.h>

static const u16 MAX_X = 40;
static const char HEADER[] = "Sega Mega Drive MIDI Interface";

static void vsync(void);
static void printLoad(u16 idle, u16 busy);

void ui_init(void)
{
    VDP_drawText(HEADER, (MAX_X - sizeof(HEADER)) / 2, 2);
    VDP_drawText(BUILD, (MAX_X - sizeof(BUILD)) / 2, 3);
    SYS_setVIntCallback(vsync);
}

static void vsync(void)
{
    char* lastError = interface_lastError();
    if (lastError != NULL) {
        VDP_setTextPalette(PAL1);
        VDP_drawText(lastError, 1, 9);
        interface_clearError();
    }

    static u8 frame = 0;
    if (++frame % 20 == 0) {
        u16 idle = comm_idleCount();
        u16 busy = comm_busyCount();
        printLoad(idle, busy);
    }
}

static void printLoad(u16 idle, u16 busy)
{
    static char idleTimeText[20];
    u16 percent = (busy * 100) / (idle + busy);
    sprintf(idleTimeText, "Load %i%s  ", percent, "%");
    comm_resetCounts();
    VDP_setTextPalette(PAL0);
    VDP_drawText(idleTimeText, 1, 7);
}
