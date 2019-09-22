#include "ui.h"
#include "comm.h"
#include "midi.h"
#include "midi_receiver.h"
#include "psg_chip.h"
#ifdef SONIC_RUNNER
#include "sonic.h"
#include "sprite.h"
#endif
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
#define POLY_X 17
#define POLY_Y 2
#define BEATS_X 6
#define BEATS_Y 2
#define RIGHTED_TEXT_X(text) (MAX_EFFECTIVE_X - (sizeof(text) - 1) + 1)
#define CENTRED_TEXT_X(text) ((MAX_EFFECTIVE_X - (sizeof(text) - 1)) / 2)
#define CHAN_X_GAP 3
#define ACTIVITY_FM_X 6
#define ACTIVITY_Y 8

#define FRAMES_BEFORE_UPDATE_ACTIVITY 1
#define FRAMES_BEFORE_UPDATE_ERROR 10
#define FRAMES_BEFORE_UPDATE_LOAD 50
#define FRAMES_BEFORE_UPDATE_LOAD_PERCENT 5

static const char HEADER[] = "Mega Drive MIDI Interface";
static const char CHAN_HEADER[] = "Ch.  F1 F2 F3 F4 F5 F6 P1 P2 P3 P4";
static const char MIDI_HEADER[] = "MIDI";

static void printChannels(void);
static void printHeader(void);
static void printLoad(void);
static void printBeat(void);
static u16 loadPercent(void);
static void printLastError(void);
static void printActivity(void);
static void printOverflowStatus(void);
static void printErrorText(const char* text);
static void drawText(const char* text, u16 x, u16 y);
static void clearText(u16 x, u16 y, u16 w);
static void printActivityForBusy(u8 busy, u16 maxChannels, u16 x);
static void printPolyphonicMode(void);

static u16 loadPercentSum = 0;

void ui_init(void)
{
    VDP_setBackgroundColor(0);
    VDP_setPaletteColor(0, RGB24_TO_VDPCOLOR(0x40));

#ifdef SONIC_RUNNER
    sonic_init(&sonic_sprite);
#endif
    printHeader();
    printChannels();
    printLoad();
    printBeat();
}

void ui_vsync(void)
{
#ifdef SONIC_RUNNER
    sonic_vsync();
#endif
    static u8 activityFrame = 0;
    if (++activityFrame == FRAMES_BEFORE_UPDATE_ACTIVITY) {
        printActivity();
        printBeat();
        activityFrame = 0;
    }

    static u8 loadCalculationFrame = 0;
    if (++loadCalculationFrame == FRAMES_BEFORE_UPDATE_LOAD_PERCENT) {
        loadPercentSum += loadPercent();
        loadCalculationFrame = 0;
    }

    static u8 loadFrame = 0;
    if (++loadFrame == FRAMES_BEFORE_UPDATE_LOAD) {
        printLoad();
        printPolyphonicMode();
        loadFrame = 0;
    }

    static u8 errorFrame = 0;
    if (++errorFrame == FRAMES_BEFORE_UPDATE_ERROR) {
        printLastError();
        printOverflowStatus();
        errorFrame = 0;
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
    drawText(CHAN_HEADER, 0, 4);
    drawText(MIDI_HEADER, 0, 6);
    drawText("Act.", 0, 8);
}

static void printActivity(void)
{
    static u8 lastSynthBusy = 0;
    u8 synthBusy = synth_busy();
    if (synthBusy != lastSynthBusy) {
        VDP_setTextPalette(PAL2);
        printActivityForBusy(synthBusy, MAX_FM_CHANS, ACTIVITY_FM_X);
        VDP_setTextPalette(PAL0);
        lastSynthBusy = synthBusy;
    }

    static u8 lastPsgBusy = 0;
    u8 psgBusy = psg_busy();
    if (psgBusy != lastPsgBusy) {
        VDP_setTextPalette(PAL2);
        printActivityForBusy(psgBusy, MAX_PSG_CHANS,
            ACTIVITY_FM_X + (CHAN_X_GAP * MAX_FM_CHANS));
        VDP_setTextPalette(PAL0);
        lastPsgBusy = psgBusy;
    }

    u8 mappings[MIDI_CHANNELS];
    u8 fmChans[MAX_FM_CHANS] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    u8 psgChans[MAX_PSG_CHANS] = { 0xFF, 0xFF, 0xFF, 0xFF };

    midi_mappings(mappings);
    for (u8 i = 0; i < MIDI_CHANNELS; i++) {
        u8 chan = mappings[i];
        if (chan == 0x7F) {
            continue;
        }
        if (chan < MIN_PSG_CHAN) {
            if (fmChans[chan] == 0xFF) {
                fmChans[chan] = i;
            }
        } else {
            if (psgChans[chan - MIN_PSG_CHAN] == 0xFF) {
                psgChans[chan - MIN_PSG_CHAN] = i;
            }
        }
    }
    for (u8 i = 0; i < MAX_FM_CHANS; i++) {
        u8 midiChannel = fmChans[i];
        if (midiChannel == 0xFF) {
            clearText((i * CHAN_X_GAP) + 5, ACTIVITY_Y - 2, 2);
        } else {
        char buffer[3];
        sprintf(buffer, "%2d", midiChannel + 1);
        drawText(buffer, (i * CHAN_X_GAP) + 5, ACTIVITY_Y - 2);
        }
    }
    for (u8 i = 0; i < MAX_PSG_CHANS; i++) {
        u8 midiChannel = psgChans[i];
        if (midiChannel == 0xFF) {
            clearText(ACTIVITY_FM_X - 1 + (CHAN_X_GAP * (MAX_FM_CHANS + i)),
                ACTIVITY_Y - 2, 2);
        } else {
        char buffer[3];
        sprintf(buffer, "%2d", midiChannel + 1);
        drawText(buffer, ACTIVITY_FM_X - 1 + (CHAN_X_GAP * (MAX_FM_CHANS + i)),
            ACTIVITY_Y - 2);
            }
    }
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

static void printBeat(void)
{
    static u16 lastSixteenth = 0xFFFF;
    Timing* timing = midi_timing();
    if (timing->sixteenth != lastSixteenth) {
        static char text[16];
        sprintf(text, "%3i. %i. %i ", timing->bar + 1, timing->barBeat + 1,
            timing->sixteenth + 1);
        VDP_setTextPalette(PAL2);
        drawText(text, BEATS_X, BEATS_Y);
        VDP_setTextPalette(PAL0);
        lastSixteenth = timing->sixteenth;
    }
}

static void printLoad(void)
{
    static char loadText[16];
    u16 percent = loadPercentSum
        / (FRAMES_BEFORE_UPDATE_LOAD / FRAMES_BEFORE_UPDATE_LOAD_PERCENT);
    loadPercentSum = 0;
    VDP_setTextPalette(percent > 70 ? PAL1 : PAL0);
    sprintf(loadText, "Load %i%c  ", percent, '%');
    comm_resetCounts();
    drawText(loadText, 0, MAX_EFFECTIVE_Y);
    VDP_setTextPalette(PAL0);
}

static void printLastError(void)
{
    static u8 lastStatus = 0;
    static char text[MAX_ERROR_X];

    u8 unknownStatus = midi_receiver_lastUnknownStatus();
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
        if (overflow) {
            printErrorText("Polyphony Overflow");
        } else {
            clearText(0, ERROR_Y, MAX_ERROR_X);
        }
        lastOverflow = overflow;
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
    bool status = midi_getPolyphonic();
    if (status != lastStatus) {
        VDP_setTextPalette(PAL2);
        if (status) {
            const char text[] = "Poly";
            drawText(text, POLY_X, POLY_Y);
        } else {
            clearText(POLY_X, POLY_Y, 4);
        }
        VDP_setTextPalette(PAL0);
        lastStatus = status;
    }
}
