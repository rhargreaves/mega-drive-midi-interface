#include "ui.h"
#include "comm.h"
#include "interface.h"
#include "midi.h"
#include "psg_chip.h"
#include "synth.h"
#include <genesis.h>

#define MAX_Y 27
#define MAX_X 39
#define MARGIN_X 1
#define MARGIN_Y 1
#define MAX_EFFECTIVE_X (MAX_X - MARGIN_X - MARGIN_X)
#define MAX_EFFECTIVE_Y (MAX_Y - MARGIN_Y - MARGIN_Y)
#define MAX_ERROR_X 30
#define ERROR_Y (MAX_EFFECTIVE_Y - 2)
#define POLY_Y (MAX_EFFECTIVE_Y - 4)
#define RIGHTED_TEXT_X(text) (MAX_EFFECTIVE_X - (sizeof(text) - 1) + 1)
#define CENTRED_TEXT_X(text) ((MAX_EFFECTIVE_X - (sizeof(text) - 1)) / 2)
#define CHAN_X_GAP 3
#define ACTIVITY_FM_X 3
#define ACTIVITY_PSG_X (ACTIVITY_FM_X + ((MAX_FM_CHANS + 1) * CHAN_X_GAP))
#define ACTIVITY_Y 6

#define FRAMES_BEFORE_UPDATE_ACTIVITY 10
#define FRAMES_BEFORE_UPDATE_LOAD 50

static const char HEADER[] = "Mega Drive MIDI Interface";
static const char CHAN_HEADER1[] = "       FM               PSG    ";
static const char CHAN_HEADER2[] = "1  2  3  4  5  6     1  2  3  4";

static void vsync(void);
static void printChannels(void);
static void printHeader(void);
static void printLoad(void);
static u16 loadPercent(void);
static void printLastError(void);
static void printActivity(void);
static void printOverflowStatus(void);
static void printErrorText(const char* text);
static void drawText(const char* text, u16 x, u16 y);
static void clearText(u16 x, u16 y, u16 w);
static void printActivityForBusy(u8 busy, u16 maxChannels, u16 x);
static void printPolyphonicMode(void);

void ui_init(void)
{
    printHeader();
    printChannels();
    printLoad();
    SYS_setVIntCallback(vsync);
}

static void vsync(void)
{
    static u8 activityFrame = 0;
    if (++activityFrame == FRAMES_BEFORE_UPDATE_ACTIVITY) {
        printLastError();
        printOverflowStatus();
        printActivity();
        activityFrame = 0;
    }
    static u8 loadFrame = 0;
    if (++loadFrame == FRAMES_BEFORE_UPDATE_LOAD) {
        printLoad();
        printPolyphonicMode();
        loadFrame = 0;
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
    VDP_setTextPalette(PAL2);
    printActivityForBusy(synth_busy(), MAX_FM_CHANS, ACTIVITY_FM_X);
    printActivityForBusy(psg_busy(), MAX_PSG_CHANS, ACTIVITY_PSG_X);
    VDP_setTextPalette(PAL0);
}

static void printActivityForBusy(u8 busy, u16 maxChannels, u16 x)
{
    for (u8 chan = 0; chan < maxChannels; chan++) {
        if ((busy >> chan) & 1) {
            drawText("*", (chan * CHAN_X_GAP) + x, ACTIVITY_Y);
        } else {
            clearText((chan * CHAN_X_GAP) + x, ACTIVITY_Y, 1);
        }
    }
}

static u16 loadPercent(void)
{
    u16 idle = comm_idleCount();
    u16 busy = comm_busyCount();
    if (idle == 0 && busy == 0) {
        return 0;
    }
    return (busy * 100) / (idle + busy);
}

static void printLoad(void)
{
    static char loadText[16];
    u8 percent = loadPercent();
    VDP_setTextPalette(percent > 70 ? PAL1 : PAL0);
    sprintf(loadText, "Load %i%s  ", percent, "%");
    comm_resetCounts();
    drawText(loadText, 0, MAX_EFFECTIVE_Y);
    VDP_setTextPalette(PAL0);
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
    ControlChange* cc = midi_lastUnknownCC();
    if ((cc->controller != lastCc.controller || cc->value != lastCc.value)
        && (cc->controller != 0 || cc->value != 0)) {
        sprintf(text, "Unknown CC %02X Value %02X", cc->controller, cc->value);
        printErrorText(text);
        lastCc.controller = cc->controller;
        lastCc.value = cc->value;
    }
}

static void printOverflowStatus(void)
{
    static bool lastOverflow = false;
    bool overflow = midi_overflow();
    if (lastOverflow != overflow) {
        printErrorText("Polyphony Overflow");
    } else {
        clearText(0, ERROR_Y, MAX_ERROR_X);
    }
}

static void printErrorText(const char* text)
{
    VDP_setTextPalette(PAL1);
    clearText(0, ERROR_Y, MAX_ERROR_X);
    drawText(text, 0, ERROR_Y);
    VDP_setTextPalette(PAL0);
}

static void printPolyphonicMode(void)
{
    static bool lastStatus = false;
    bool status = interface_polyphonic();
    if (status != lastStatus) {
        VDP_setTextPalette(PAL2);
        if (status) {
            const char text[] = "Poly";
            drawText(text, 0, POLY_Y);
        } else {
            clearText(0, POLY_Y, 4);
        }
        VDP_setTextPalette(PAL0);
        lastStatus = status;
    }
}
