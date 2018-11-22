#include "ui.h"
#include "comm.h"
#include "interface.h"
#include <genesis.h>

#define MAX_X 40
#define MAX_ERROR_X 30
#define ERROR_Y 9
#define FRAMES_BEFORE_UPDATE 10

static const char HEADER[] = "Sega Mega Drive MIDI Interface";

static void vsync(void);
static void printHeader(void);
static void printLoad(void);
static u16 loadPercent(void);
static void printLastError(void);
static void printErrorText(const char* text);

void ui_init(void)
{
    printHeader();
    SYS_setVIntCallback(vsync);
}

static void vsync(void)
{
    static u8 frame = 0;
    if (++frame == FRAMES_BEFORE_UPDATE) {
        printLastError();
        printLoad();
        frame = 0;
    }
}

static void printHeader(void)
{
    VDP_drawText(HEADER, (MAX_X - sizeof(HEADER)) / 2, 2);
    VDP_drawText(BUILD, (MAX_X - sizeof(BUILD)) / 2, 3);
}

static u16 loadPercent(void)
{
    u16 idle = comm_idleCount();
    u16 busy = comm_busyCount();
    return (busy * 100) / (idle + busy);
}

static void printLoad(void)
{
    static char loadText[16];
    sprintf(loadText, "Load %i%s  ", loadPercent(), "%");
    comm_resetCounts();
    VDP_setTextPalette(PAL0);
    VDP_drawText(loadText, 1, 7);
}

static void printLastError(void)
{
    static u8 lastStatus = 0;
    char text[MAX_ERROR_X];
    u8 unknownStatus = interface_lastUnknownStatus();
    if (unknownStatus != lastStatus && unknownStatus != 0) {
        sprintf(text, "Unknown Status %02X", unknownStatus);
        printErrorText(text);
        lastStatus = unknownStatus;
    }

    static ControlChange lastCc;
    ControlChange* cc = interface_lastUnknownCC();
    if ((cc->controller != lastCc.controller || cc->value != lastCc.value)
        && (cc->controller != 0 || cc->value != 0)) {
        sprintf(text, "Unknown CC %02X Value %02X", cc->controller, cc->value);
        printErrorText(text);
        lastCc.controller = cc->controller;
        lastCc.value = cc->value;
    }
}

static void printErrorText(const char* text)
{
    VDP_setTextPalette(PAL1);
    VDP_clearText(1, ERROR_Y, MAX_ERROR_X);
    VDP_drawText(text, 1, ERROR_Y);
}
