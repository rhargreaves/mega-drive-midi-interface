#include "ui_fm.h"
#include "midi.h"
#include "synth.h"
#include "sprite.h"
#include "ui.h"

#define UNKNOWN_FM_CHANNEL 0xFF

static const u8 BASE_Y = 7;
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

static void update_fm_values(void);
static void init_algorithm_sprites(void);
static void update_fm_values_if_chan_selected(void);
static void synth_parameter_updated(u8 fmChan, ParameterUpdated parameterUpdated);
static bool update_fm_value(
    u8 last, const u8 current, bool forceRefresh, FormatTextFunc formatFunc, u8 x, u8 y);

void ui_fm_init(void)
{
    synth_set_parameter_update_callback(synth_parameter_updated);
    init_algorithm_sprites();
}

void ui_fm_update(void)
{
    update_fm_values_if_chan_selected();
}

static void init_algorithm_sprites(void)
{
    const SpriteDefinition* algors[]
        = { &algor_0, &algor_1, &algor_2, &algor_3, &algor_4, &algor_5, &algor_6, &algor_7 };

    for (int i = 0; i < FM_ALGORITHMS; i++) {
        const SpriteDefinition* algor = algors[i];
        Sprite* sprite = SPR_addSprite(algor, fix32ToInt(FIX32(9 * 8)),
            fix32ToInt(FIX32((BASE_Y + 6) * 8)), TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
        SPR_setVisibility(sprite, HIDDEN);
        algorSprites[i] = sprite;
    }

    PAL_setColors((PAL0 * 16), activity.palette->data, activity.palette->length, CPU);
}

static void synth_parameter_updated(u8 fmChan, ParameterUpdated parameterUpdated)
{
    if (fmChan == chanParasFmChan || parameterUpdated == Lfo || parameterUpdated == SpecialMode) {
        synthParameterValuesDirty = true;
    }
}

static void print_channel_parameter_headings(void)
{
    VDP_setTextPalette(PAL3);
    ui_draw_text("Op.   1   2   3   4", OP_HEADING_X, BASE_Y + 3);
    ui_draw_text(" TL", OP_HEADING_X, BASE_Y + 4);
    ui_draw_text(" AR", OP_HEADING_X, BASE_Y + 5);
    ui_draw_text("MUL", OP_HEADING_X, BASE_Y + 6);
    ui_draw_text(" DT", OP_HEADING_X, BASE_Y + 7);
    ui_draw_text(" RS", OP_HEADING_X, BASE_Y + 8);
    ui_draw_text(" AM", OP_HEADING_X, BASE_Y + 9);
    ui_draw_text(" DR", OP_HEADING_X, BASE_Y + 10);
    ui_draw_text(" SR", OP_HEADING_X, BASE_Y + 11);
    ui_draw_text(" SL", OP_HEADING_X, BASE_Y + 12);
    ui_draw_text(" RR", OP_HEADING_X, BASE_Y + 13);
    ui_draw_text("SSG", OP_HEADING_X, BASE_Y + 14);

    ui_draw_text("MIDI", FM_HEADING_X, BASE_Y + 3);
    ui_draw_text("FM", FM_HEADING_X + 8, BASE_Y + 3);

    ui_draw_text("Alg", FM_HEADING_X, BASE_Y + 5);
    ui_draw_text("Fb", FM_HEADING_X, BASE_Y + 6);
    ui_draw_text("Pan", FM_HEADING_X, BASE_Y + 7);
    ui_draw_text("FMS", FM_HEADING_X, BASE_Y + 8);
    ui_draw_text("AMS", FM_HEADING_X, BASE_Y + 9);

    ui_draw_text("LFO", FM_HEADING_X, BASE_Y + 11);
    ui_draw_text("Ch3", FM_HEADING_X, BASE_Y + 12);

    VDP_setTextPalette(PAL0);
}

static void hide_all_algorithms(void)
{
    for (u8 i = 0; i < FM_ALGORITHMS; i++) {
        SPR_setVisibility(algorSprites[i], HIDDEN);
    }
}

static void update_algorithm_diagram(u8 algorithm)
{
    hide_all_algorithms();
    SPR_setVisibility(algorSprites[algorithm], VISIBLE);
    SPR_update();
}

static u8 get_fm_chan_for_midi_chan(u8 midiChan)
{
    DeviceChannel* devChans = midi_channel_mappings();
    for (u8 i = 0; i <= DEV_CHAN_MAX_FM; i++) {
        DeviceChannel* devChan = &devChans[i];
        if (devChan->midiChannel == midiChan) {
            return devChan->num;
        }
    }
    return UNKNOWN_FM_CHANNEL;
}

static const char* stereo_text(u8 stereo)
{
    switch (stereo) {
    case 0:
        return "Mute";
    case 1:
        return "R   ";
    case 2:
        return "L   ";
    default:
        return "LR  ";
    }
}

static const char* lfo_enable_text(u8 lfoEnable)
{
    switch (lfoEnable) {
    case 0:
        return "Off";
    default:
        return "On ";
    }
}

static const char* ch3_special_mode_text(u8 enabled)
{
    switch (enabled) {
    case true:
        return "Special";
    default:
        return "Normal ";
    }
}

static const char* lfo_freq_text(u8 lfoFreq)
{
    static const char TEXT[][8]
        = { "3.98Hz", "5.56Hz", "6.02Hz", "6.37Hz", "6.88Hz", "9.63Hz", "48.1Hz", "72.2Hz" };
    return TEXT[lfoFreq];
}

static const char* ams_text(u8 ams)
{
    static const char TEXT[][7] = { "0dB   ", "1.4dB ", "5.9dB ", "11.8dB" };
    return TEXT[ams];
}

static const char* fms_text(u8 fms)
{
    static const char TEXT[][5]
        = { "0%  ", "3.4%", "6.7%", "10% ", "14% ", "20% ", "40% ", "80% " };
    return TEXT[fms];
}

static const char* chan_number(u8 chan)
{
    static char buffer[3];
    sprintf(buffer, "%-2d", chan + 1);
    return buffer;
}

static const char* format_num(u8 value)
{
    static char buffer[3];
    sprintf(buffer, "%d", value);
    return buffer;
}

static bool update_fm_value(
    u8 last, const u8 current, bool forceRefresh, FormatTextFunc formatFunc, u8 x, u8 y)
{
    if (last != current || forceRefresh) {
        ui_draw_text(formatFunc(current), x, y);
        last = current;
        return true;
    }
    return false;
}

static bool update_fm_value_bool(
    bool last, bool current, bool forceRefresh, FormatTextFunc formatFunc, u8 x, u8 y)
{
    if (last != current || forceRefresh) {
        ui_draw_text(formatFunc(current), x, y);
        return true;
    }
    return false;
}

static bool update_op_value(u8 last, u8 current, bool forceRefresh, u8 op, u8 line)
{
    const u8 OP_VALUE_X = OP_HEADING_X + 4;
    const u8 OP_VALUE_GAP = 4;

    if (last != current || forceRefresh) {
        char buffer[4];
        sprintf(buffer, "%3d", current);
        ui_draw_text(buffer, OP_VALUE_X + (op * OP_VALUE_GAP), BASE_Y + line);
        return true;
    }
    return false;
}

static void update_op_values(const FmChannel* channel, bool forceRefresh)
{
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        const Operator* oper = &channel->operators[op];
        Operator* lastOper = &lastChannel.operators[op];

        if (update_op_value(lastOper->totalLevel, oper->totalLevel, forceRefresh, op, 4))
            lastOper->totalLevel = oper->totalLevel;
        if (update_op_value(lastOper->attackRate, oper->attackRate, forceRefresh, op, 5))
            lastOper->attackRate = oper->attackRate;
        if (update_op_value(lastOper->multiple, oper->multiple, forceRefresh, op, 6))
            lastOper->multiple = oper->multiple;
        if (update_op_value(lastOper->detune, oper->detune, forceRefresh, op, 7))
            lastOper->detune = oper->detune;
        if (update_op_value(lastOper->rateScaling, oper->rateScaling, forceRefresh, op, 8))
            lastOper->rateScaling = oper->rateScaling;
        if (update_op_value(
                lastOper->amplitudeModulation, oper->amplitudeModulation, forceRefresh, op, 9))
            lastOper->amplitudeModulation = oper->amplitudeModulation;
        if (update_op_value(lastOper->decayRate, oper->decayRate, forceRefresh, op, 10))
            lastOper->decayRate = oper->decayRate;
        if (update_op_value(lastOper->sustainRate, oper->sustainRate, forceRefresh, op, 11))
            lastOper->sustainRate = oper->sustainRate;
        if (update_op_value(lastOper->sustainLevel, oper->sustainLevel, forceRefresh, op, 12))
            lastOper->sustainLevel = oper->sustainLevel;
        if (update_op_value(lastOper->releaseRate, oper->releaseRate, forceRefresh, op, 13))
            lastOper->releaseRate = oper->releaseRate;
        if (update_op_value(lastOper->ssgEg, oper->ssgEg, forceRefresh, op, 14))
            lastOper->ssgEg = oper->ssgEg;
    }
}

static void update_fm_values(void)
{
    const FmChannel* channel = synth_channel_parameters(chanParasFmChan);
    const Global* global = synth_global_parameters();

    const u8 COL1_VALUE_X = FM_HEADING_X + 4;
    const u8 COL2_VALUE_X = FM_HEADING_X + 11;

    if (update_fm_value(lastChanParasMidiChannel, chanParasMidiChan, forceRefresh, chan_number,
            COL1_VALUE_X + 1, BASE_Y + 3)) {
        lastChanParasMidiChannel = chanParasMidiChan;
    }
    if (update_fm_value(lastChanParasFmChan, chanParasFmChan, forceRefresh, chan_number,
            COL2_VALUE_X, BASE_Y + 3))
        lastChanParasFmChan = chanParasFmChan;

    if (update_fm_value(lastChannel.algorithm, channel->algorithm, forceRefresh, format_num,
            COL1_VALUE_X, BASE_Y + 5)) {
        lastChannel.algorithm = channel->algorithm;
        update_algorithm_diagram(channel->algorithm);
    }
    if (update_fm_value(lastChannel.feedback, channel->feedback, forceRefresh, format_num,
            COL1_VALUE_X, BASE_Y + 6)) {
        lastChannel.feedback = channel->feedback;
    }
    if (update_fm_value(lastChannel.stereo, channel->stereo, forceRefresh, stereo_text,
            COL1_VALUE_X, BASE_Y + 7)) {
        lastChannel.stereo = channel->stereo;
    }
    if (update_fm_value(
            lastChannel.fms, channel->fms, forceRefresh, fms_text, COL1_VALUE_X, BASE_Y + 8)) {
        lastChannel.fms = channel->fms;
    }
    if (update_fm_value(
            lastChannel.ams, channel->ams, forceRefresh, ams_text, COL1_VALUE_X, BASE_Y + 9)) {
        lastChannel.ams = channel->ams;
    }
    if (update_fm_value(lastGlobal.lfoEnable, global->lfoEnable, forceRefresh, lfo_enable_text,
            COL1_VALUE_X, BASE_Y + 11)) {
        lastGlobal.lfoEnable = global->lfoEnable;
    }
    if (update_fm_value(lastGlobal.lfoFrequency, global->lfoFrequency, forceRefresh, lfo_freq_text,
            COL1_VALUE_X + 4, BASE_Y + 11)) {
        lastGlobal.lfoFrequency = global->lfoFrequency;
    }
    if (update_fm_value_bool(lastGlobal.specialMode, global->specialMode, forceRefresh,
            ch3_special_mode_text, COL1_VALUE_X, BASE_Y + 12)) {
        lastGlobal.specialMode = global->specialMode;
    }

    update_op_values(channel, forceRefresh);
    forceRefresh = false;
}

static void update_fm_values_if_chan_selected(void)
{
    if (!showChanParameters) {
        return;
    }

    u8 chan = get_fm_chan_for_midi_chan(chanParasMidiChan);
    if (chan == UNKNOWN_FM_CHANNEL) {
        return;
    }
    if (chanParasFmChan != chan) {
        chanParasFmChan = chan;
        synthParameterValuesDirty = true;
    }

    if (synthParameterValuesDirty) {
        update_fm_values();
        synthParameterValuesDirty = false;
    }
}

void ui_fm_set_parameters_visibility(u8 chan, bool show)
{
    showChanParameters = show;
    chanParasMidiChan = chan;
    if (show) {
        ui_hide_logs();
        forceRefresh = true;
        print_channel_parameter_headings();
    } else {
        VDP_clearTextArea(0, MARGIN_Y + BASE_Y + 3, MAX_X, 12);
        hide_all_algorithms();
        SPR_update();
        ui_show_logs();
    }
    synthParameterValuesDirty = true;
}
