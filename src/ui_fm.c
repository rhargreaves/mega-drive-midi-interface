#include "ui_fm.h"
#include "midi.h"

#include "synth.h"
#include "ui.h"
#include <genesis.h>

#include "sprite.h"

static bool showChanParameters = false;

#define UNKNOWN_FM_CHANNEL 0xFF

static u8 chanParasMidiChan = 0;
static u8 chanParasFmChan = 0;

static bool synthParameterValuesDirty = false;

static const u8 base_y = 8;
const u8 op_heading_x = 15;
const u8 para_heading_x = 0;

static Sprite* algorSprites[FM_ALGORITHMS];

static void updateFmValues(void);

static void initAlgorithmSprites(void)
{
    const SpriteDefinition* algors[] = { &algor_0, &algor_1, &algor_2, &algor_3,
        &algor_4, &algor_5, &algor_6, &algor_7 };

    for (int i = 0; i < FM_ALGORITHMS; i++) {
        const SpriteDefinition* algor = algors[i];
        Sprite* sprite = SPR_addSprite(algor, fix32ToInt(FIX32(9 * 8)),
            fix32ToInt(FIX32((base_y + 6) * 8)),
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

void ui_fm_init(void)
{
    synth_setParameterUpdateCallback(synthParameterUpdated);
    initAlgorithmSprites();
}

static void printChannelParameterHeadings(void)
{
    VDP_setTextPalette(PAL3);
    ui_drawText("Op.   1   2   3   4", op_heading_x, base_y + 3);
    ui_drawText(" TL", op_heading_x, base_y + 4);
    ui_drawText(" DT", op_heading_x, base_y + 5);
    ui_drawText("MUL", op_heading_x, base_y + 6);
    ui_drawText(" RS", op_heading_x, base_y + 7);
    ui_drawText(" AM", op_heading_x, base_y + 8);
    ui_drawText("D1R", op_heading_x, base_y + 9);
    ui_drawText("D2R", op_heading_x, base_y + 10);
    ui_drawText(" SL", op_heading_x, base_y + 11);
    ui_drawText(" RR", op_heading_x, base_y + 12);
    ui_drawText("SSG", op_heading_x, base_y + 13);

    ui_drawText("MIDI", para_heading_x, base_y + 3);
    ui_drawText("FM", para_heading_x + 8, base_y + 3);
    ui_drawText("Alg", para_heading_x, base_y + 5);
    ui_drawText("FB", para_heading_x, base_y + 6);
    ui_drawText("LFO", para_heading_x, base_y + 9);
    ui_drawText("AMS", para_heading_x, base_y + 10);
    ui_drawText("FMS", para_heading_x, base_y + 11);
    ui_drawText("Str", para_heading_x, base_y + 12);
    VDP_setTextPalette(PAL0);
}

static void printOperatorValue(u16 value, u8 op, u8 line)
{
    const u8 op_value_x = op_heading_x + 4;
    const u8 op_value_gap = 4;

    char buffer[4];
    sprintf(buffer, "%3d", value);
    ui_drawText(buffer, op_value_x + (op * op_value_gap), base_y + line);
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

static u8 lastChanParasFmChan = 0;
static u8 lastChanParasMidiChannel = 0;
static FmChannel lastChannel = {};
static Global lastGlobal = {};
static bool forceRefresh = false;

static void updateFmValues(void)
{
    const FmChannel* channel = synth_channelParameters(chanParasFmChan);
    const Global* global = synth_globalParameters();

    const u8 col1_value_x = para_heading_x + 4;
    const u8 col2_value_x = para_heading_x + 11;
    char buffer[4];

    if (chanParasMidiChan != lastChanParasMidiChannel || forceRefresh) {
        sprintf(buffer, "%-2d", chanParasMidiChan + 1);
        ui_drawText(buffer, col1_value_x + 1, base_y + 3);
        lastChanParasMidiChannel = chanParasMidiChan;
    }

    if (chanParasFmChan != lastChanParasFmChan || forceRefresh) {
        sprintf(buffer, "%-3d", chanParasFmChan + 1);
        ui_drawText(buffer, col2_value_x, base_y + 3);
        lastChanParasFmChan = chanParasFmChan;
    }

    if (channel->algorithm != lastChannel.algorithm || forceRefresh) {
        sprintf(buffer, "%d", channel->algorithm);
        ui_drawText(buffer, col1_value_x, base_y + 5);
        lastChannel.algorithm = channel->algorithm;
    }

    if (channel->feedback != lastChannel.feedback || forceRefresh) {
        sprintf(buffer, "%d", channel->feedback);
        ui_drawText(buffer, col1_value_x, base_y + 6);
        lastChannel.feedback = channel->feedback;
    }

    if (global->lfoEnable != lastGlobal.lfoEnable || forceRefresh) {
        ui_drawText(lfoEnableText(global->lfoEnable), col1_value_x, base_y + 9);
        lastGlobal.lfoEnable = global->lfoEnable;
    }

    if (global->lfoFrequency != lastGlobal.lfoFrequency || forceRefresh) {
        ui_drawText(
            lfoFreqText(global->lfoFrequency), col1_value_x + 4, base_y + 9);
        lastGlobal.lfoFrequency = global->lfoFrequency;
    }

    if (channel->ams != lastChannel.ams || forceRefresh) {
        ui_drawText(amsText(channel->ams), col1_value_x, base_y + 10);
        lastChannel.ams = channel->ams;
    }

    if (channel->fms != lastChannel.fms || forceRefresh) {
        ui_drawText(fmsText(channel->fms), col1_value_x, base_y + 11);
        lastChannel.fms = channel->fms;
    }

    if (channel->stereo != lastChannel.stereo || forceRefresh) {
        ui_drawText(stereoText(channel->stereo), col1_value_x, base_y + 12);
        lastChannel.stereo = channel->stereo;
    }

    if (channel->algorithm != lastChannel.algorithm || forceRefresh) {
        updateAlgorithmDiagram(channel->algorithm);
        lastChannel.algorithm = channel->algorithm;
    }

    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        u8 line = 4;
        const Operator* oper = &channel->operators[op];
        Operator* lastOper = &lastChannel.operators[op];

        if (oper->totalLevel != lastOper->totalLevel || forceRefresh) {
            printOperatorValue(oper->totalLevel, op, line);
            lastOper->totalLevel = oper->totalLevel;
        }
        line++;

        if (oper->detune != lastOper->detune || forceRefresh) {
            printOperatorValue(oper->detune, op, line);
            lastOper->detune = oper->detune;
        }
        line++;

        if (oper->multiple != lastOper->multiple || forceRefresh) {
            printOperatorValue(oper->multiple, op, line);
            lastOper->multiple = oper->multiple;
        }
        line++;

        if (oper->rateScaling != lastOper->rateScaling || forceRefresh) {
            printOperatorValue(oper->rateScaling, op, line);
            lastOper->rateScaling = oper->rateScaling;
        }
        line++;

        if (oper->amplitudeModulation != lastOper->amplitudeModulation
            || forceRefresh) {
            printOperatorValue(oper->amplitudeModulation, op, line);
            lastOper->amplitudeModulation = oper->amplitudeModulation;
        }
        line++;

        if (oper->firstDecayRate != lastOper->firstDecayRate || forceRefresh) {
            printOperatorValue(oper->firstDecayRate, op, line);
            lastOper->firstDecayRate = oper->firstDecayRate;
        }
        line++;

        if (oper->secondaryDecayRate != lastOper->secondaryDecayRate
            || forceRefresh) {
            printOperatorValue(oper->secondaryDecayRate, op, line);
            lastOper->secondaryDecayRate = oper->secondaryDecayRate;
        }
        line++;

        if (oper->secondaryAmplitude != lastOper->secondaryAmplitude
            || forceRefresh) {
            printOperatorValue(oper->secondaryAmplitude, op, line);
            lastOper->secondaryAmplitude = oper->secondaryAmplitude;
        }
        line++;

        if (oper->releaseRate != lastOper->releaseRate || forceRefresh) {
            printOperatorValue(oper->releaseRate, op, line);
            lastOper->releaseRate = oper->releaseRate;
        }
        line++;

        if (oper->releaseRate != lastOper->releaseRate || forceRefresh) {
            printOperatorValue(oper->releaseRate, op, line);
            lastOper->releaseRate = oper->releaseRate;
        }
        line++;

        if (oper->ssgEg != lastOper->ssgEg || forceRefresh) {
            printOperatorValue(oper->ssgEg, op, line);
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
        VDP_clearTextArea(0, MARGIN_Y + base_y + 3, MAX_X, 11);
        hideAllAlgorithms();
        SPR_update();
    }
    synthParameterValuesDirty = true;
}

void ui_fm_update(void)
{
    updateFmValuesIfChanSelected();
}
