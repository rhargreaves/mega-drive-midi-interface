#include "ui_fm.h"
#include "midi.h"
#include "synth.h"
#include "ui.h"
#include <genesis.h>

#include "sprite.h"

#define UNKNOWN_FM_CHANNEL 0xFF

static const u8 BASE_Y = 6;
static const u8 OP_HEADING_X = 15;
static const u8 FM_HEADING_X = 0;

static bool synthParameterValuesDirty = false;
static bool showChanParameters = false;
static u8 chanParasMidiChan = 0;
static u8 chanParasFmChan = 0;
static Sprite* algorSprites[FM_ALGORITHMS];

static u8 lastChanParasFmChan = 0;
static u8 lastChanParasMidiChannel = 0;
static FmChannel lastChannel = {};
static Global lastGlobal = {};
static bool forceRefresh = false;

typedef const char* FormatTextFunc(u8 value);

static void updateFmValues(void);
static void initAlgorithmSprites(void);
static void updateFmValuesIfChanSelected(void);
static void synthParameterUpdated(u8 fmChan, ParameterUpdated parameterUpdated);
static bool updateFmValueText(u8* last, const u8* current, bool forceRefresh,
    FormatTextFunc formatFunc, u8 x, u8 y);

void ui_fm_init(void)
{
    synth_setParameterUpdateCallback(synthParameterUpdated);
    initAlgorithmSprites();
}

void ui_fm_update(void)
{
    updateFmValuesIfChanSelected();
}

static void initAlgorithmSprites(void)
{
    const SpriteDefinition* algors[] = { &algor_0, &algor_1, &algor_2, &algor_3,
        &algor_4, &algor_5, &algor_6, &algor_7 };

    for (int i = 0; i < FM_ALGORITHMS; i++) {
        const SpriteDefinition* algor = algors[i];
        Sprite* sprite = SPR_addSprite(algor, fix32ToInt(FIX32(9 * 8)),
            fix32ToInt(FIX32((BASE_Y + 6) * 8)),
            TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
        SPR_setVisibility(sprite, HIDDEN);
        algorSprites[i] = sprite;
    }

    VDP_setPaletteColors(
        (PAL0 * 16), activity.palette->data, activity.palette->length);
}

static void synthParameterUpdated(u8 fmChan, ParameterUpdated parameterUpdated)
{
    if (fmChan == chanParasFmChan || parameterUpdated == Lfo) {
        synthParameterValuesDirty = true;
    }
}

static void printChannelParameterHeadings(void)
{
    VDP_setTextPalette(PAL3);
    ui_drawText("Op.   1   2   3   4", OP_HEADING_X, BASE_Y + 3);
    ui_drawText(" TL", OP_HEADING_X, BASE_Y + 4);
    ui_drawText(" AR", OP_HEADING_X, BASE_Y + 5);
    ui_drawText("MUL", OP_HEADING_X, BASE_Y + 6);
    ui_drawText(" DT", OP_HEADING_X, BASE_Y + 7);
    ui_drawText(" RS", OP_HEADING_X, BASE_Y + 8);
    ui_drawText(" AM", OP_HEADING_X, BASE_Y + 9);
    ui_drawText("D1R", OP_HEADING_X, BASE_Y + 10);
    ui_drawText("D2R", OP_HEADING_X, BASE_Y + 11);
    ui_drawText(" SL", OP_HEADING_X, BASE_Y + 12);
    ui_drawText(" RR", OP_HEADING_X, BASE_Y + 13);
    ui_drawText("SSG", OP_HEADING_X, BASE_Y + 14);

    ui_drawText("MIDI", FM_HEADING_X, BASE_Y + 3);
    ui_drawText("FM", FM_HEADING_X + 8, BASE_Y + 3);
    ui_drawText("Alg", FM_HEADING_X, BASE_Y + 5);
    ui_drawText("FB", FM_HEADING_X, BASE_Y + 6);
    ui_drawText("LFO", FM_HEADING_X, BASE_Y + 9);
    ui_drawText("AMS", FM_HEADING_X, BASE_Y + 10);
    ui_drawText("FMS", FM_HEADING_X, BASE_Y + 11);
    ui_drawText("Pan", FM_HEADING_X, BASE_Y + 12);
    VDP_setTextPalette(PAL0);
}

static void printOperatorValue(u16 value, u8 op, u8 line)
{
    const u8 OP_VALUE_X = OP_HEADING_X + 4;
    const u8 OP_VALUE_GAP = 4;

    char buffer[4];
    sprintf(buffer, "%3d", value);
    ui_drawText(buffer, OP_VALUE_X + (op * OP_VALUE_GAP), BASE_Y + line);
}

static void hideAllAlgorithms(void)
{
    for (u8 i = 0; i < FM_ALGORITHMS; i++) {
        SPR_setVisibility(algorSprites[i], HIDDEN);
    }
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
    return UNKNOWN_FM_CHANNEL;
}

static const char* stereoText(u8 stereo)
{
    switch (stereo) {
    case 0:
        return "  ";
    case 1:
        return "R ";
    case 2:
        return "L ";
    default:
        return "LR";
    }
}

static const char* lfoEnableText(u8 lfoEnable)
{
    switch (lfoEnable) {
    case 0:
        return "Off";
    default:
        return "On ";
    }
}

static const char* lfoFreqText(u8 lfoFreq)
{
    static const char TEXT[][8] = { "3.98Hz", "5.56Hz", "6.02Hz", "6.37Hz",
        "6.88Hz", "9.63Hz", "48.1Hz", "72.2Hz" };
    return TEXT[lfoFreq];
}

static const char* amsText(u8 ams)
{
    static const char TEXT[][7] = { "0dB   ", "1.4dB ", "5.9dB ", "11.8dB" };
    return TEXT[ams];
}

static const char* fmsText(u8 fms)
{
    static const char TEXT[][5]
        = { "0%  ", "3.4%", "6.7%", "10% ", "14% ", "20% ", "40% ", "80% " };
    return TEXT[fms];
}

static bool updateFmValue(u8* last, const u8* current, bool forceRefresh,
    const char* format, u8 x, u8 y)
{
    char buffer[4];
    if (*last != *current || forceRefresh) {
        sprintf(buffer, format, *current);
        ui_drawText(buffer, x, y);
        *last = *current;
        return true;
    }
    return false;
}

static bool updateFmValueText(u8* last, const u8* current, bool forceRefresh,
    FormatTextFunc formatFunc, u8 x, u8 y)
{
    if (*last != *current || forceRefresh) {
        ui_drawText(formatFunc(*current), x, y);
        *last = *current;
        return true;
    }
    return false;
}

static const char* chanNumber(u8 chan)
{
    static char buffer[4];
    sprintf(buffer, "%-2d", chan + 1);
    return buffer;
}

static void updateOperatorValueText(
    u8* last, const u8* current, bool forceRefresh, u8 op, u8 line)
{
    const u8 OP_VALUE_X = OP_HEADING_X + 4;
    const u8 OP_VALUE_GAP = 4;

    if (*last != *current || forceRefresh) {
        char buffer[4];
        sprintf(buffer, "%3d", *current);
        ui_drawText(buffer, OP_VALUE_X + (op * OP_VALUE_GAP), BASE_Y + line);
        *last = *current;
    }
}

static void updateFmValues(void)
{
    const FmChannel* channel = synth_channelParameters(chanParasFmChan);
    const Global* global = synth_globalParameters();

    const u8 col1_value_x = FM_HEADING_X + 4;
    const u8 col2_value_x = FM_HEADING_X + 11;

    updateFmValueText(&lastChanParasMidiChannel, &chanParasMidiChan,
        forceRefresh, chanNumber, col1_value_x + 1, BASE_Y + 3);
    updateFmValueText(&lastChanParasFmChan, &chanParasFmChan, forceRefresh,
        chanNumber, col2_value_x, BASE_Y + 3);
    if (updateFmValue(&lastChannel.algorithm, &channel->algorithm, forceRefresh,
            "%d", col1_value_x, BASE_Y + 5)) {
        updateAlgorithmDiagram(channel->algorithm);
    }
    updateFmValue(&lastChannel.feedback, &channel->feedback, forceRefresh, "%d",
        col1_value_x, BASE_Y + 6);
    updateFmValueText(&lastGlobal.lfoEnable, &global->lfoEnable, forceRefresh,
        lfoEnableText, col1_value_x, BASE_Y + 9);
    updateFmValueText(&lastGlobal.lfoFrequency, &global->lfoFrequency,
        forceRefresh, lfoFreqText, col1_value_x + 4, BASE_Y + 9);
    updateFmValueText(&lastChannel.ams, &channel->ams, forceRefresh, amsText,
        col1_value_x, BASE_Y + 10);
    updateFmValueText(&lastChannel.fms, &channel->fms, forceRefresh, fmsText,
        col1_value_x, BASE_Y + 11);
    updateFmValueText(&lastChannel.stereo, &channel->stereo, forceRefresh,
        stereoText, col1_value_x, BASE_Y + 12);

    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        const Operator* oper = &channel->operators[op];
        Operator* lastOper = &lastChannel.operators[op];

        updateOperatorValueText(
            &lastOper->totalLevel, &oper->totalLevel, forceRefresh, op, 4);
        updateOperatorValueText(
            &lastOper->attackRate, &oper->attackRate, forceRefresh, op, 5);
        updateOperatorValueText(
            &lastOper->multiple, &oper->multiple, forceRefresh, op, 6);
        updateOperatorValueText(
            &lastOper->detune, &oper->detune, forceRefresh, op, 7);
        updateOperatorValueText(
            &lastOper->rateScaling, &oper->rateScaling, forceRefresh, op, 8);
        updateOperatorValueText(&lastOper->amplitudeModulation,
            &oper->amplitudeModulation, forceRefresh, op, 9);
        updateOperatorValueText(&lastOper->firstDecayRate,
            &oper->firstDecayRate, forceRefresh, op, 10);
        updateOperatorValueText(&lastOper->secondaryDecayRate,
            &oper->secondaryDecayRate, forceRefresh, op, 11);
        updateOperatorValueText(&lastOper->secondaryAmplitude,
            &oper->secondaryAmplitude, forceRefresh, op, 12);
        updateOperatorValueText(
            &lastOper->releaseRate, &oper->releaseRate, forceRefresh, op, 13);
        updateOperatorValueText(
            &lastOper->ssgEg, &oper->ssgEg, forceRefresh, op, 14);
    }
    forceRefresh = false;
}

static void updateFmValuesIfChanSelected(void)
{
    if (!showChanParameters) {
        return;
    }

    u8 chan = getFmChanForMidiChan(chanParasMidiChan);
    if (chan == UNKNOWN_FM_CHANNEL) {
        return;
    }
    if (chanParasFmChan != chan) {
        chanParasFmChan = chan;
        synthParameterValuesDirty = true;
    }

    if (synthParameterValuesDirty) {
        updateFmValues();
        synthParameterValuesDirty = false;
    }
}

void ui_fm_setMidiChannelParametersVisibility(u8 chan, bool show)
{
    showChanParameters = show;
    chanParasMidiChan = chan;
    if (show) {
        forceRefresh = true;
        printChannelParameterHeadings();
    } else {
        VDP_clearTextArea(0, MARGIN_Y + BASE_Y + 3, MAX_X, 12);
        hideAllAlgorithms();
        SPR_update();
    }
    synthParameterValuesDirty = true;
}
