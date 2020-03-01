#include "ui.h"
#include "buffer.h"
#include "comm.h"
#include "comm_serial.h"
#include "log.h"
#include "memcmp.h"
#include "midi.h"
#include "midi_receiver.h"
#include "psg_chip.h"
#include "serial.h"
#include "synth.h"
#include "ui_fm.h"
#include <genesis.h>

#include "sprite.h"

#define MAX_EFFECTIVE_X (MAX_X - MARGIN_X - MARGIN_X)
#define MAX_EFFECTIVE_Y (MAX_Y - MARGIN_Y - MARGIN_Y)
#define MAX_ERROR_X 30
#define ERROR_Y (MAX_EFFECTIVE_Y - 2)
#define LOG_Y 22
#define RIGHTED_TEXT_X(text) (MAX_EFFECTIVE_X - (sizeof(text) - 1) + 1)
#define CENTRED_TEXT_X(text) ((MAX_EFFECTIVE_X - (sizeof(text) - 1)) / 2)
#define CHAN_X_GAP 3
#define ACTIVITY_FM_X 6

#define CHAN_Y 2
#define MIDI_Y CHAN_Y + 2
#define ACTIVITY_Y MIDI_Y + 2

#define PALETTE_INDEX(pal, index) ((pal * 16) + index)
#define FONT_COLOUR_INDEX 15
#define BG_COLOUR_INDEX 0

#define FRAMES_BEFORE_UPDATE_CHAN_ACTIVITY 1
#define FRAMES_BEFORE_UPDATE_ACTIVITY 5
#define FRAMES_BEFORE_UPDATE_ERROR 11
#define FRAMES_BEFORE_UPDATE_LOAD 47
#define FRAMES_BEFORE_UPDATE_LOAD_PERCENT 13

static const char HEADER[] = "Mega Drive MIDI Interface";
static const char CHAN_HEADER[] = "Ch.  F1 F2 F3 F4 F5 F6 P1 P2 P3 P4";
static const char MIDI_HEADER[] = "MIDI";

static void checkLastError(void);
static void printChannels(void);
static void printHeader(void);
static void printLoad(void);
static u16 loadPercent(void);
static void updateKeyOnOff(void);
static void drawText(const char* text, u16 x, u16 y);
static void printChanActivity(u16 busy, u16 maxChannels, u16 x);
static void printBaudRate(void);
static void printCommMode(void);
static void printCommBuffer(void);
static void populateMappings(u8* midiChans);
static void printDynamicModeIfNeeded(void);
static void printDynamicModeStatus(bool enabled);
static void printMappingsIfDirty(u8* midiChans);
static void printMappings(void);

static u16 loadPercentSum = 0;
static bool commInited = false;
static bool commSerial = false;

static Sprite* activitySprites[DEV_CHANS];

void ui_init(void)
{
    VDP_setBackgroundColor(BG_COLOUR_INDEX);
    VDP_setPaletteColor(BG_COLOUR_INDEX, RGB24_TO_VDPCOLOR(0x202020));
    VDP_setPaletteColor(
        PALETTE_INDEX(PAL1, FONT_COLOUR_INDEX), RGB24_TO_VDPCOLOR(0xFFFF00));
    VDP_setPaletteColor(
        PALETTE_INDEX(PAL3, FONT_COLOUR_INDEX), RGB24_TO_VDPCOLOR(0x808080));
    printHeader();
    printChannels();
    printLoad();
    printCommMode();
    printMappings();
    printDynamicModeStatus(midi_dynamicMode());

    SYS_disableInts();
    SPR_init();

    for (int i = 0; i < DEV_CHANS; i++) {
        Sprite* sprite = SPR_addSprite(&activity,
            fix32ToInt(FIX32(((i * CHAN_X_GAP) + 7) * 8)),
            fix32ToInt(FIX32((ACTIVITY_Y + 1) * 8)),
            TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
        SPR_setVisibility(sprite, VISIBLE);
        activitySprites[i] = sprite;
    }

    SPR_update();
    SYS_enableInts();

    ui_fm_init();
}

static void printMappings(void)
{
    u8 midiChans[DEV_CHANS] = { 0 };
    populateMappings(midiChans);
    printMappingsIfDirty(midiChans);
}

static void printLog(void)
{
    static u8 logLine = 0;
    const u8 maxLines = 3;

    Log* log = log_dequeue();
    if (log == NULL) {
        return;
    }
    if (logLine >= maxLines) {
        VDP_clearTextArea(
            MARGIN_X, LOG_Y + MARGIN_Y, MAX_EFFECTIVE_X, maxLines);
        logLine = 0;
    }
    switch (log->level) {
    case Warn:
        VDP_setTextPalette(PAL1);
        break;
    default:
        VDP_setTextPalette(PAL2);
        break;
    }
    drawText(log->msg, 0, LOG_Y + logLine);
    VDP_setTextPalette(PAL0);
    logLine++;
}

void ui_update(void)
{
    updateKeyOnOff();

    static u8 activityFrame = 0;
    if (++activityFrame == FRAMES_BEFORE_UPDATE_ACTIVITY) {
        activityFrame = 0;
        printMappings();
        printCommMode();
        printCommBuffer();
        printLog();
    }

    static u8 loadCalculationFrame = 0;
    if (++loadCalculationFrame == FRAMES_BEFORE_UPDATE_LOAD_PERCENT) {
        loadCalculationFrame = 0;
        loadPercentSum += loadPercent();
    }

    static u8 loadFrame = 0;
    if (++loadFrame == FRAMES_BEFORE_UPDATE_LOAD) {
        loadFrame = 0;
        printLoad();
        printDynamicModeIfNeeded();
    }

    static u8 errorFrame = 0;
    if (++errorFrame == FRAMES_BEFORE_UPDATE_ERROR) {
        errorFrame = 0;
        checkLastError();
    }

    ui_fm_update();
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

void ui_drawText(const char* text, u16 x, u16 y)
{
    drawText(text, x, y);
}

static void drawText(const char* text, u16 x, u16 y)
{
    VDP_drawText(text, MARGIN_X + x, MARGIN_Y + y);
}

static void printHeader(void)
{
    drawText(HEADER, 4, 0);
    drawText(BUILD, RIGHTED_TEXT_X(BUILD), 0);
}

static void printChannels(void)
{
    VDP_setTextPalette(PAL3);
    drawText(CHAN_HEADER, 0, CHAN_Y);
    drawText(MIDI_HEADER, 0, MIDI_Y);
    drawText("Act.", 0, ACTIVITY_Y);
    VDP_setTextPalette(PAL0);
}

static void printCommBuffer(void)
{
    if (!commSerial) {
        return;
    }
    u16 bufferAvailable = buffer_available();
    if (bufferAvailable < 32) {
        log_warn("Serial port buffer has %d bytes left", bufferAvailable, 0, 0);
    }
}

static void updateKeyOnOff(void)
{
    static u16 lastBusy = 0;
    u16 busy = synth_busy() | (psg_busy() << 6);
    if (busy != lastBusy) {
        printChanActivity(busy, MAX_FM_CHANS + MAX_PSG_CHANS, ACTIVITY_FM_X);
        lastBusy = busy;
    }
}

static u8 midiChannelForUi(DeviceChannel* mappings, u8 index)
{
    return (mappings[index].midiChannel) + 1;
}

static void printMappingsIfDirty(u8* midiChans)
{
    static u8 lastMidiChans[DEV_CHANS];
    if (memcmp(lastMidiChans, midiChans, sizeof(u8) * DEV_CHANS) == 0) {
        return;
    }
    memcpy(lastMidiChans, midiChans, sizeof(u8) * DEV_CHANS);

    char text[38];
    sprintf(text, "%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d", midiChans[0],
        midiChans[1], midiChans[2], midiChans[3], midiChans[4], midiChans[5],
        midiChans[6], midiChans[7], midiChans[8], midiChans[9]);
    drawText(text, 5, MIDI_Y);
}

static void populateMappings(u8* midiChans)
{
    DeviceChannel* chans = midi_channelMappings();
    for (u8 i = 0; i < DEV_CHANS; i++) {
        midiChans[i] = midiChannelForUi(chans, i);
    }
}

static void printChanActivity(u16 busy, u16 maxChannels, u16 x)
{
    for (u8 chan = 0; chan < maxChannels; chan++) {
        SPR_setFrame(activitySprites[chan], ((busy >> chan) & 1) ? 1 : 0);
    }
    SPR_update();
}

static void printBaudRate(void)
{
    char baudRateText[9];
    sprintf(baudRateText, "%dbps", comm_serial_baudRate());
    drawText(baudRateText, 17, MAX_EFFECTIVE_Y);
}

static void printCommMode(void)
{
    if (commInited) {
        return;
    }
    const char* MODES_TEXT[] = { "Waiting", "ED USB ", "Serial ", "Unknown" };
    u16 index;
    switch (comm_mode()) {
    case Discovery:
        index = 0;
        break;
    case Everdrive:
        index = 1;
        commInited = true;
        break;
    case Serial:
        index = 2;
        commInited = true;
        commSerial = true;
        printBaudRate();
        break;
    default:
        index = 3;
        break;
    }
    drawText(MODES_TEXT[index], 10, MAX_EFFECTIVE_Y);
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

static void printDynamicModeStatus(bool enabled)
{
    drawText(
        enabled ? "Dynamic" : " Static", MAX_EFFECTIVE_X - 6, MAX_EFFECTIVE_Y);
}

static void printDynamicModeIfNeeded(void)
{
    static bool lastDynamicModeStatus = false;
    bool enabled = midi_dynamicMode();
    if (lastDynamicModeStatus != enabled) {
        printDynamicModeStatus(enabled);
        lastDynamicModeStatus = enabled;
    }
}

static void checkLastError(void)
{
    static u8 lastStatus = 0;
    u8 unknownStatus = midi_receiver_lastUnknownStatus();
    if (unknownStatus != lastStatus && unknownStatus != 0) {
        log_warn("Unknown Status %02X", unknownStatus, 0, 0);
        lastStatus = unknownStatus;
    }

    static ControlChange lastCc;
    ControlChange* cc = midi_lastUnknownCC();
    if ((cc->controller != lastCc.controller || cc->value != lastCc.value)
        && (cc->controller != 0 || cc->value != 0)) {
        log_warn("Unknown CC %02X Value %02X", cc->controller, cc->value, 0);
        lastCc.controller = cc->controller;
        lastCc.value = cc->value;
    }
}
