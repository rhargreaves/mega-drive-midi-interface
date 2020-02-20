#include "ui_fm.h"
#include "midi.h"
#include "synth.h"
#include "ui.h"
#include <genesis.h>

#include "sprite.h"

#define UNKNOWN_FM_CHANNEL 0xFF

static const u8 BASE_Y = 8;
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

static void updateFmValues(void);
static void initAlgorithmSprites(void);
static void updateFmValuesIfChanSelected(void);
static void synthParameterUpdated(u8 fmChan, ParameterUpdated parameterUpdated);

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
    ui_drawText(" DT", OP_HEADING_X, BASE_Y + 5);
    ui_drawText("MUL", OP_HEADING_X, BASE_Y + 6);
    ui_drawText(" RS", OP_HEADING_X, BASE_Y + 7);
    ui_drawText(" AM", OP_HEADING_X, BASE_Y + 8);
    ui_drawText("D1R", OP_HEADING_X, BASE_Y + 9);
    ui_drawText("D2R", OP_HEADING_X, BASE_Y + 10);
    ui_drawText(" SL", OP_HEADING_X, BASE_Y + 11);
    ui_drawText(" RR", OP_HEADING_X, BASE_Y + 12);
    ui_drawText("SSG", OP_HEADING_X, BASE_Y + 13);

    ui_drawText("MIDI", FM_HEADING_X, BASE_Y + 3);
    ui_drawText("FM", FM_HEADING_X + 8, BASE_Y + 3);
    ui_drawText("Alg", FM_HEADING_X, BASE_Y + 5);
    ui_drawText("FB", FM_HEADING_X, BASE_Y + 6);
    ui_drawText("LFO", FM_HEADING_X, BASE_Y + 9);
    ui_drawText("AMS", FM_HEADING_X, BASE_Y + 10);
    ui_drawText("FMS", FM_HEADING_X, BASE_Y + 11);
    ui_drawText("Str", FM_HEADING_X, BASE_Y + 12);
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

static void updateFmValues(void)
{
    const FmChannel* channel = synth_channelParameters(chanParasFmChan);
    const Global* global = synth_globalParameters();

    const u8 col1_value_x = FM_HEADING_X + 4;
    const u8 col2_value_x = FM_HEADING_X + 11;
    char buffer[4];

    if (chanParasMidiChan != lastChanParasMidiChannel || forceRefresh) {
        sprintf(buffer, "%-2d", chanParasMidiChan + 1);
        ui_drawText(buffer, col1_value_x + 1, BASE_Y + 3);
        lastChanParasMidiChannel = chanParasMidiChan;
    }

    if (chanParasFmChan != lastChanParasFmChan || forceRefresh) {
        sprintf(buffer, "%-3d", chanParasFmChan + 1);
        ui_drawText(buffer, col2_value_x, BASE_Y + 3);
        lastChanParasFmChan = chanParasFmChan;
    }

    if (channel->algorithm != lastChannel.algorithm || forceRefresh) {
        sprintf(buffer, "%d", channel->algorithm);
        ui_drawText(buffer, col1_value_x, BASE_Y + 5);
        lastChannel.algorithm = channel->algorithm;
    }

    if (channel->feedback != lastChannel.feedback || forceRefresh) {
        sprintf(buffer, "%d", channel->feedback);
        ui_drawText(buffer, col1_value_x, BASE_Y + 6);
        lastChannel.feedback = channel->feedback;
    }

    if (global->lfoEnable != lastGlobal.lfoEnable || forceRefresh) {
        ui_drawText(lfoEnableText(global->lfoEnable), col1_value_x, BASE_Y + 9);
        lastGlobal.lfoEnable = global->lfoEnable;
    }

    if (global->lfoFrequency != lastGlobal.lfoFrequency || forceRefresh) {
        ui_drawText(
            lfoFreqText(global->lfoFrequency), col1_value_x + 4, BASE_Y + 9);
        lastGlobal.lfoFrequency = global->lfoFrequency;
    }

    if (channel->ams != lastChannel.ams || forceRefresh) {
        ui_drawText(amsText(channel->ams), col1_value_x, BASE_Y + 10);
        lastChannel.ams = channel->ams;
    }

    if (channel->fms != lastChannel.fms || forceRefresh) {
        ui_drawText(fmsText(channel->fms), col1_value_x, BASE_Y + 11);
        lastChannel.fms = channel->fms;
    }

    if (channel->stereo != lastChannel.stereo || forceRefresh) {
        ui_drawText(stereoText(channel->stereo), col1_value_x, BASE_Y + 12);
        lastChannel.stereo = channel->stereo;
    }

    if (channel->algorithm != lastChannel.algorithm || forceRefresh) {
        updateAlgorithmDiagram(channel->algorithm);
        lastChannel.algorithm = channel->algorithm;
    }

    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        const Operator* oper = &channel->operators[op];
        Operator* lastOper = &lastChannel.operators[op];

        if (oper->totalLevel != lastOper->totalLevel || forceRefresh) {
            printOperatorValue(oper->totalLevel, op, 4);
            lastOper->totalLevel = oper->totalLevel;
        }

        if (oper->detune != lastOper->detune || forceRefresh) {
            printOperatorValue(oper->detune, op, 5);
            lastOper->detune = oper->detune;
        }

        if (oper->multiple != lastOper->multiple || forceRefresh) {
            printOperatorValue(oper->multiple, op, 6);
            lastOper->multiple = oper->multiple;
        }

        if (oper->rateScaling != lastOper->rateScaling || forceRefresh) {
            printOperatorValue(oper->rateScaling, op, 7);
            lastOper->rateScaling = oper->rateScaling;
        }

        if (oper->amplitudeModulation != lastOper->amplitudeModulation
            || forceRefresh) {
            printOperatorValue(oper->amplitudeModulation, op, 8);
            lastOper->amplitudeModulation = oper->amplitudeModulation;
        }

        if (oper->firstDecayRate != lastOper->firstDecayRate || forceRefresh) {
            printOperatorValue(oper->firstDecayRate, op, 9);
            lastOper->firstDecayRate = oper->firstDecayRate;
        }

        if (oper->secondaryDecayRate != lastOper->secondaryDecayRate
            || forceRefresh) {
            printOperatorValue(oper->secondaryDecayRate, op, 10);
            lastOper->secondaryDecayRate = oper->secondaryDecayRate;
        }

        if (oper->secondaryAmplitude != lastOper->secondaryAmplitude
            || forceRefresh) {
            printOperatorValue(oper->secondaryAmplitude, op, 11);
            lastOper->secondaryAmplitude = oper->secondaryAmplitude;
        }

        if (oper->releaseRate != lastOper->releaseRate || forceRefresh) {
            printOperatorValue(oper->releaseRate, op, 12);
            lastOper->releaseRate = oper->releaseRate;
        }

        if (oper->ssgEg != lastOper->ssgEg || forceRefresh) {
            printOperatorValue(oper->ssgEg, op, 13);
            lastOper->ssgEg = oper->ssgEg;
        }
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
        VDP_clearTextArea(0, MARGIN_Y + BASE_Y + 3, MAX_X, 11);
        hideAllAlgorithms();
        SPR_update();
    }
    synthParameterValuesDirty = true;
}
