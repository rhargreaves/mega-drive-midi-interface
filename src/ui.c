#include "ui.h"
#include "buffer.h"
#include "comm.h"
#include "comm_serial.h"
#include "log.h"
#include "memcmp.h"
#include "midi.h"
#include "midi_receiver.h"
#include "psg_chip.h"
#include "synth.h"
#include <genesis.h>

#include "sprite.h"

static bool showChanParameters = false;

#define MAX_Y 27
#define MAX_X 39
#define MARGIN_X 1
#define MARGIN_Y 1
#define MAX_EFFECTIVE_X (MAX_X - MARGIN_X - MARGIN_X)
#define MAX_EFFECTIVE_Y (MAX_Y - MARGIN_Y - MARGIN_Y)
#define MAX_ERROR_X 30
#define ERROR_Y (MAX_EFFECTIVE_Y - 2)
#define BEATS_X 6
#define BEATS_Y 2
#define DYN_X 22
#define DYN_Y 2
#define LOG_Y 22
#define RIGHTED_TEXT_X(text) (MAX_EFFECTIVE_X - (sizeof(text) - 1) + 1)
#define CENTRED_TEXT_X(text) ((MAX_EFFECTIVE_X - (sizeof(text) - 1)) / 2)
#define CHAN_X_GAP 3
#define ACTIVITY_FM_X 6
#define ACTIVITY_Y 8
#define CHAN_Y 4
#define MIDI_Y 6

#define PALETTE_INDEX(pal, index) ((pal * 16) + index)
#define FONT_COLOUR_INDEX 15
#define BG_COLOUR_INDEX 0

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
static void printBeat(void);
static u16 loadPercent(void);
static void printActivity(void);
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
static void printChannelParameters(void);

static u16 loadPercentSum = 0;
static bool commInited = false;
static bool commSerial = false;
static u16 lastUpdateFrame = 0;
static volatile u16 frame = 0;

static Sprite* algorSprites[FM_ALGORITHMS] = {};

static const u8 base_y = 8;
const u8 op_heading_x = 15;
const u8 para_heading_x = 0;

static void initAlgorithmSprites(void)
{
    SYS_disableInts();
    SPR_init();
    const SpriteDefinition* algors[] = { &algor_0, &algor_1, &algor_2, &algor_3,
        &algor_4, &algor_5, &algor_6, &algor_7 };

    for (int i = 0; i < FM_ALGORITHMS; i++) {
        const SpriteDefinition* algor = algors[i];
        Sprite* sprite = SPR_addSprite(algor, fix32ToInt(FIX32(9 * 8)),
            fix32ToInt(FIX32((base_y + 6) * 8)),
            TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
        SPR_setVisibility(sprite, HIDDEN);
        algorSprites[i] = sprite;
        VDP_setPaletteColors(
            (PAL0 * 16), algor->palette->data, algor->palette->length);
    }
    SPR_update();
    SYS_enableInts();
}

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
    printBeat();
    printCommMode();
    printMappings();
    printDynamicModeStatus(midi_dynamicMode());
    initAlgorithmSprites();
}

void ui_vsync(void)
{
    frame++;
}

static void printChannelParameterHeadings(void)
{
    VDP_setTextPalette(PAL3);
    drawText("Op.   1   2   3   4", op_heading_x, base_y + 3);
    drawText(" TL", op_heading_x, base_y + 4);
    drawText(" DT", op_heading_x, base_y + 5);
    drawText("MUL", op_heading_x, base_y + 6);
    drawText(" RS", op_heading_x, base_y + 7);
    drawText(" AM", op_heading_x, base_y + 8);
    drawText("D1R", op_heading_x, base_y + 9);
    drawText("D2R", op_heading_x, base_y + 10);
    drawText(" SL", op_heading_x, base_y + 11);
    drawText(" RR", op_heading_x, base_y + 12);
    drawText("SSG", op_heading_x, base_y + 13);

    drawText("MIDI", para_heading_x, base_y + 3);
    drawText("FM", para_heading_x + 8, base_y + 3);
    drawText("Alg", para_heading_x, base_y + 5);
    drawText("FB", para_heading_x, base_y + 6);
    drawText("LFO", para_heading_x, base_y + 9);
    drawText("Hz", para_heading_x + 8, base_y + 9);
    drawText("AMS", para_heading_x, base_y + 10);
    drawText("FMS", para_heading_x, base_y + 11);
    drawText("Ster", para_heading_x, base_y + 12);
    VDP_setTextPalette(PAL0);
}

static void printOperatorValue(u16 value, u8 op, u8 line)
{
    const u8 op_value_x = op_heading_x + 4;
    const u8 op_value_gap = 4;

    char buffer[4];
    sprintf(buffer, "%3d", value);
    drawText(buffer, op_value_x + (op * op_value_gap), base_y + line);
}

static void hideAllAlgorithms(void)
{
    for (u8 i = 0; i < FM_ALGORITHMS; i++) {
        SPR_setVisibility(algorSprites[i], HIDDEN);
    }
    SPR_update();
}

static void updateAlgorithmDiagram(u8 algorithm)
{
    hideAllAlgorithms();
    SPR_setVisibility(algorSprites[algorithm], VISIBLE);
    SPR_update();
}

static u8 getFmChanForMidiChan(u8 midiChan)
{
    DeviceChannel* devChans = midi_channelMappings();
    for (u8 i = 0; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* devChan = &devChans[i];
        if (devChan->midiChannel == midiChan) {
            return devChan->number;
        }
    }
    return -1;
}

static void printChannelParameters(void)
{
    printChannelParameterHeadings();

    u8 midiChan = 0;
    u8 chan = getFmChanForMidiChan(midiChan);
    if (chan == -1) {
        return;
    }
    const FmChannel* channel = synth_channelParameters(chan);
    const Global* global = synth_globalParameters();
    char buffer[4];
    sprintf(buffer, "%-2d", midiChan + 1);
    drawText(buffer, para_heading_x + 5, base_y + 3);
    sprintf(buffer, "%-3d", chan + 1);
    drawText(buffer, para_heading_x + 11, base_y + 3);
    sprintf(buffer, "%d", channel->algorithm);
    drawText(buffer, para_heading_x + 5, base_y + 5);
    sprintf(buffer, "%d", channel->feedback);
    drawText(buffer, para_heading_x + 5, base_y + 6);

    sprintf(buffer, "%d", global->lfoEnable);
    drawText(buffer, para_heading_x + 5, base_y + 9);
    sprintf(buffer, "%d", global->lfoFrequency);
    drawText(buffer, para_heading_x + 11, base_y + 9);

    sprintf(buffer, "%d", channel->ams);
    drawText(buffer, para_heading_x + 5, base_y + 10);
    sprintf(buffer, "%d", channel->fms);
    drawText(buffer, para_heading_x + 5, base_y + 11);

    char* stereoText;
    switch (channel->stereo) {
    case 0:
        stereoText = "  ";
        break;
    case 1:
        stereoText = "R ";
        break;
    case 2:
        stereoText = "L ";
        break;
    default:
        stereoText = "LR";
        break;
    }
    drawText(stereoText, para_heading_x + 5, base_y + 12);

    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        u8 line = 4;
        printOperatorValue(channel->operators[op].totalLevel, op, line++);
        printOperatorValue(channel->operators[op].detune, op, line++);
        printOperatorValue(channel->operators[op].multiple, op, line++);
        printOperatorValue(channel->operators[op].rateScaling, op, line++);
        printOperatorValue(
            channel->operators[op].amplitudeModulation, op, line++);
        printOperatorValue(channel->operators[op].firstDecayRate, op, line++);
        printOperatorValue(
            channel->operators[op].secondaryDecayRate, op, line++);
        printOperatorValue(
            channel->operators[op].secondaryAmplitude, op, line++);
        printOperatorValue(channel->operators[op].releaseRate, op, line++);
        printOperatorValue(channel->operators[op].ssgEg, op, line++);
    }

    updateAlgorithmDiagram(channel->algorithm);
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
    if (lastUpdateFrame == frame) {
        return;
    }

    static u8 activityFrame = 0;
    if (++activityFrame == FRAMES_BEFORE_UPDATE_ACTIVITY) {
        printActivity();
        printMappings();
        printBeat();
        printCommMode();
        printCommBuffer();
        printLog();
        if (showChanParameters) {
            printChannelParameters();
        }
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
        printDynamicModeIfNeeded();
        loadFrame = 0;
    }

    static u8 errorFrame = 0;
    if (++errorFrame == FRAMES_BEFORE_UPDATE_ERROR) {
        checkLastError();
        errorFrame = 0;
    }

    lastUpdateFrame = frame;
}

void ui_setMidiChannelParametersVisibility(u8 chan, bool show)
{
    showChanParameters = show;
    if (!show) {
        VDP_clearTextArea(0, MARGIN_Y + base_y + 3, MAX_X, 11);
        hideAllAlgorithms();
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
    char text[32];
    sprintf(text, "%4d Free", buffer_available());
    drawText(text, 29, MAX_EFFECTIVE_Y);
}

static void printActivity(void)
{
    static u16 lastBusy = 0;
    u16 busy = synth_busy() | (psg_busy() << 6);
    if (busy != lastBusy) {
        VDP_setTextPalette(PAL2);
        printChanActivity(busy, MAX_FM_CHANS + MAX_PSG_CHANS, ACTIVITY_FM_X);
        VDP_setTextPalette(PAL0);
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
    u8 lineLength = (CHAN_X_GAP * maxChannels) + 1;
    char line[lineLength];
    memset(line, ' ', lineLength - 1);
    line[lineLength] = '\0';
    for (u8 chan = 0; chan < maxChannels; chan++) {
        if ((busy >> chan) & 1) {
            line[chan * CHAN_X_GAP] = '*';
        }
    }
    drawText(line, x, ACTIVITY_Y);
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

static void printBaudRate(void)
{
    char baudRateText[9];
    sprintf(baudRateText, "%d bps", comm_serial_baudRate());
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
    VDP_setTextPalette(PAL2);
    drawText(enabled ? "Dynamic" : "Static ", DYN_X, DYN_Y);
    VDP_setTextPalette(PAL0);
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
