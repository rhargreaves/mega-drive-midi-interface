#include "ui.h"
#include "comm.h"
#include "interface.h"
#include <genesis.h>

static const u16 MAX_X = 40;
static const char HEADER[] = "Sega Mega Drive MIDI Interface";

static void vsync(void);
static void printHeader(void);
static void printLoad(void);
static void printLastError(void);

void ui_init(void)
{
    printHeader();
    SYS_setVIntCallback(vsync);
}

static void vsync(void)
{
    static u8 frame = 0;
    if (++frame % 20 == 0) {
        printLastError();
        printLoad();
    }
}

static void printHeader(void)
{
    VDP_drawText(HEADER, (MAX_X - sizeof(HEADER)) / 2, 2);
    VDP_drawText(BUILD, (MAX_X - sizeof(BUILD)) / 2, 3);
}

static void printLoad(void)
{
    static char loadText[20];
    u16 idle = comm_idleCount();
    u16 busy = comm_busyCount();
    u16 percent = (busy * 100) / (idle + busy);
    sprintf(loadText, "Load %i%s  ", percent, "%");
    comm_resetCounts();
    VDP_setTextPalette(PAL0);
    VDP_drawText(loadText, 1, 7);
}

static void printLastError(void)
{
    static u8 lastStatus = 0;
    char text[20];
    u8 unknownStatus = interface_lastUnknownStatus();
    if (unknownStatus != lastStatus && unknownStatus != 0) {
        sprintf(text, "Unknown Status %02X", unknownStatus);
        VDP_setTextPalette(PAL1);
        VDP_drawText(text, 1, 9);
        lastStatus = unknownStatus;
    }
}
