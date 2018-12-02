#include "ui.h"
#include "comm.h"
#include "interface.h"
#include "synth.h"
#include <genesis.h>

#define MAX_Y 27
#define MAX_X 39
#define MARGIN_X 1
#define MARGIN_Y 1
#define MAX_EFFECTIVE_X MAX_X - MARGIN_X - MARGIN_X
#define MAX_EFFECTIVE_Y MAX_Y - MARGIN_Y - MARGIN_Y
#define MAX_ERROR_X 30
#define ERROR_Y (MAX_EFFECTIVE_Y - 2)
#define RIGHTED_TEXT_X(text) (MAX_EFFECTIVE_X - (sizeof(text) - 1)) + 1
#define CENTRED_TEXT_X(text) ((MAX_EFFECTIVE_X - (sizeof(text) - 1)) / 2)

#define FRAMES_BEFORE_UPDATE 10

static const char HEADER[] = "Mega Drive MIDI Interface";
static const char CHAN_HEADER1[] = "       FM              PSG    ";
static const char CHAN_HEADER2[] = "1  2  3  4  5  6    1  2  3  4";

static void vsync(void);
static void printChannels(void);
static void printHeader(void);
static void printLoad(void);
static u16 loadPercent(void);
static void printLastError(void);
static void printActivity(void);
static void printErrorText(const char* text);
static void drawText(const char* text, u16 x, u16 y);
static void clearText(u16 x, u16 y, u16 w);

void ui_init(void)
{
    printHeader();
    printChannels();
    SYS_setVIntCallback(vsync);
}

static void vsync(void)
{
    static u8 frame = 0;
    if (++frame == FRAMES_BEFORE_UPDATE) {
        printLastError();
        printLoad();
        printActivity();
        frame = 0;
    }
}

static void clearText(u16 x, u16 y, u16 w)
{
    VDP_clearText(MARGIN_X + x, MARGIN_Y + y, w);
}

static void drawText(const char* text, u16 x, u16 y)
{
    VDP_drawText(text, MARGIN_X + x, MARGIN_Y + y);
}

static void printHeader(void)
{
    drawText(HEADER, CENTRED_TEXT_X(HEADER), 0);
    drawText(BUILD, RIGHTED_TEXT_X(BUILD), MAX_EFFECTIVE_Y);
}

static void printChannels(void)
{
    drawText(CHAN_HEADER1, CENTRED_TEXT_X(CHAN_HEADER1), 2);
    drawText(CHAN_HEADER2, CENTRED_TEXT_X(CHAN_HEADER2), 4);
}

static void printActivity(void)
{
    const u8 ACTIVITY_X = 3;
    const u8 ACTIVITY_Y = 6;
    const u8 CHAN_X_GAP = 3;

    VDP_setTextPalette(PAL2);
    u8 busy = synth_busy();
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        if ((busy >> chan) & 1) {
            drawText("*", (chan * CHAN_X_GAP) + ACTIVITY_X, ACTIVITY_Y);
        } else {
            clearText((chan * CHAN_X_GAP) + ACTIVITY_X, ACTIVITY_Y, 1);
        }
    }
    VDP_setTextPalette(PAL0);
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
    drawText(loadText, 0, MAX_EFFECTIVE_Y);
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
    VDP_drawText(text, MARGIN_X, ERROR_Y);
    VDP_setTextPalette(PAL0);
}
