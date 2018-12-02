#include "ui.h"
#include "comm.h"
#include "interface.h"
#include "synth.h"
#include <genesis.h>

#define MAX_Y 27
#define MAX_X 39
#define MARGIN_X 1
#define MARGIN_Y 1
#define MAX_ERROR_X 30
#define ERROR_Y (MAX_Y - MARGIN_Y - 2)
#define RIGHTED_TEXT_X(text) (MAX_X - (sizeof(text) - 1))
#define CENTRED_TEXT_X(text) ((MAX_X - (sizeof(text) - 1)) / 2)

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

static void printHeader(void)
{
    VDP_drawText(HEADER, CENTRED_TEXT_X(HEADER), MARGIN_Y);
    VDP_drawText(BUILD, RIGHTED_TEXT_X(BUILD), MAX_Y - MARGIN_Y);
}

static void printChannels(void)
{
    VDP_drawText(
        CHAN_HEADER1, (MAX_X - sizeof(CHAN_HEADER1)) / 2, MARGIN_Y + 2);
    VDP_drawText(
        CHAN_HEADER2, (MAX_X - sizeof(CHAN_HEADER2)) / 2, MARGIN_Y + 4);
}

static void printActivity(void)
{
    const u8 ACTIVITY_X = MARGIN_X + 3;
    const u8 ACTIVITY_Y = MARGIN_Y + 6;
    const u8 CHAN_X_GAP = 3;

    VDP_setTextPalette(PAL2);
    u8 busy = synth_busy();
    for (u8 chan = 0; chan < MAX_FM_CHANS; chan++) {
        if ((busy >> chan) & 1) {
            VDP_drawText("*", (chan * CHAN_X_GAP) + ACTIVITY_X, ACTIVITY_Y);
        } else {
            VDP_clearText((chan * CHAN_X_GAP) + ACTIVITY_X, ACTIVITY_Y, 1);
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
    VDP_drawText(loadText, MARGIN_X, MAX_Y - MARGIN_Y);
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
