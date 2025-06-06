#include "ui_fm.h"
#include "midi.h"
#include "synth.h"
#include "sprite.h"
#include "ui.h"
#include "utils.h"

#define UNKNOWN_FM_CHANNEL 0xFF

#define BASE_Y 2
#define OP_BASE_Y (BASE_Y + 7)
#define OP_HEADING_X 15
#define FM_HEADING_X 15

#define ALGORITHM_X (FM_HEADING_X + 14)
#define ALGORITHM_Y (BASE_Y + 1)

typedef const char* FormatTextFunc(u8 value);

typedef struct FmDisplayData {
    Global* global;
    FmChannel* channel;
    u8 midiChan;
    u8 fmChan;
} FmDisplayData;

static bool synthParameterValuesDirty = false;
static bool showChanParameters = false;
static u8 chanParasMidiChan = 0;
static u8 chanParasFmChan = 0;
static Sprite* algorSprites[FM_ALGORITHMS];

static FmChannel lastChannel = {};
static bool forceRefresh = false;

static void update_fm_values(void);
static void init_algorithm_sprites(void);
static void update_fm_values_if_chan_selected(void);
static void synth_parameter_updated(u8 fmChan, ParameterUpdated parameterUpdated);

static const char* stereo_text(u8 stereo);
static const char* lfo_enable_text(u8 lfoEnable);
static const char* ch3_special_mode_text(u8 enabled);
static const char* ams_text(u8 ams);
static const char* fms_text(u8 fms);
static const char* chan_number(u8 chan);
static const char* format_num(u8 value);
static const char* lfo_freq_text(u8 lfoFreq);

static u8 get_midi_channel(const FmDisplayData* data);
static u8 get_fm_channel(const FmDisplayData* data);
static u8 get_algorithm(const FmDisplayData* data);
static u8 get_feedback(const FmDisplayData* data);
static u8 get_fms(const FmDisplayData* data);
static u8 get_ams(const FmDisplayData* data);
static u8 get_lfo_enable(const FmDisplayData* data);
static u8 get_special_mode(const FmDisplayData* data);
static u8 get_stereo(const FmDisplayData* data);
static u8 get_lfo_frequency(const FmDisplayData* data);

typedef struct UIField {
    u8 headerX;
    u8 headerY;
    u8 valueOffset;
    const char* name;
    const char* (*formatFunc)(u8 value);
    u8 (*getValue)(const FmDisplayData* data);
    u8 lastValue;
} UIField;

static UIField fmFields[] = {
    { 0, 0, 3, "Ch", chan_number, get_midi_channel, 0 },
    { 7, 0, 3, "FM", chan_number, get_fm_channel, 0 },

    { 0, 2, 4, "Alg", format_num, get_algorithm, 0 },
    { 7, 2, 3, "Fb", format_num, get_feedback, 0 },

    { 0, 3, 4, "Pan", stereo_text, get_stereo, 0 },

    { 0, 4, 4, "FMS", fms_text, get_fms, 0 },
    { 8, 4, 4, "AMS", ams_text, get_ams, 0 },

    { 0, 5, 4, "LFO", lfo_enable_text, get_lfo_enable, 0 },
    { 8, 5, 0, "", lfo_freq_text, get_lfo_frequency, 0 },

    { 9, 23, 4, "Ch3", ch3_special_mode_text, get_special_mode, 0 },
};

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
        Sprite* sprite = SPR_addSprite(algor, fix32ToInt(FIX32(ALGORITHM_X * 8)),
            fix32ToInt(FIX32(ALGORITHM_Y * 8)), TILE_ATTR(PAL0, TRUE, FALSE, FALSE));
        SPR_setVisibility(sprite, HIDDEN);
        algorSprites[i] = sprite;
    }
}

static void synth_parameter_updated(u8 fmChan, ParameterUpdated parameterUpdated)
{
    if (fmChan == chanParasFmChan || parameterUpdated == Lfo || parameterUpdated == SpecialMode) {
        synthParameterValuesDirty = true;
    }
}

// Value getter functions
static u8 get_midi_channel(const FmDisplayData* data)
{
    return data->midiChan;
}

static u8 get_fm_channel(const FmDisplayData* data)
{
    return data->fmChan;
}

static u8 get_algorithm(const FmDisplayData* data)
{
    return data->channel->algorithm;
}

static u8 get_feedback(const FmDisplayData* data)
{
    return data->channel->feedback;
}

static u8 get_fms(const FmDisplayData* data)
{
    return data->channel->fms;
}

static u8 get_ams(const FmDisplayData* data)
{
    return data->channel->ams;
}

static u8 get_lfo_enable(const FmDisplayData* data)
{
    return data->global->lfoEnable;
}

static u8 get_special_mode(const FmDisplayData* data)
{
    return data->global->specialMode;
}

static u8 get_stereo(const FmDisplayData* data)
{
    return data->channel->stereo;
}

static u8 get_lfo_frequency(const FmDisplayData* data)
{
    return data->global->lfoFrequency;
}

static void print_channel_parameter_headings(void)
{
    VDP_setTextPalette(PAL3);
    ui_draw_text("    Op1   2   3   4", OP_HEADING_X, OP_BASE_Y);
    ui_draw_text(" TL", OP_HEADING_X, OP_BASE_Y + 1);
    ui_draw_text(" AR", OP_HEADING_X, OP_BASE_Y + 2);
    ui_draw_text("MUL", OP_HEADING_X, OP_BASE_Y + 3);
    ui_draw_text(" DT", OP_HEADING_X, OP_BASE_Y + 4);
    ui_draw_text(" RS", OP_HEADING_X, OP_BASE_Y + 5);
    ui_draw_text(" AM", OP_HEADING_X, OP_BASE_Y + 6);
    ui_draw_text(" DR", OP_HEADING_X, OP_BASE_Y + 7);
    ui_draw_text(" SR", OP_HEADING_X, OP_BASE_Y + 8);
    ui_draw_text(" SL", OP_HEADING_X, OP_BASE_Y + 9);
    ui_draw_text(" RR", OP_HEADING_X, OP_BASE_Y + 10);
    ui_draw_text("SSG", OP_HEADING_X, OP_BASE_Y + 11);

    // Print all FM field headers using the loop
    for (u8 i = 0; i < LENGTH_OF(fmFields); i++) {
        ui_draw_text(
            fmFields[i].name, FM_HEADING_X + fmFields[i].headerX, BASE_Y + fmFields[i].headerY);
    }
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
        = { "4Hz  ", "5.5Hz", "6Hz  ", "6.5Hz", "7Hz  ", "10Hz ", "48Hz ", "72Hz " };
    return TEXT[lfoFreq];
}

static const char* ams_text(u8 ams)
{
    static const char TEXT[][5] = { "0dB ", "1dB ", "6dB ", "12dB" };
    return TEXT[ams];
}

static const char* fms_text(u8 fms)
{
    static const char TEXT[][5]
        = { "0%  ", "~3% ", "~6% ", "10% ", "14% ", "20% ", "40% ", "80% " };
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

static bool update_op_value(u8 last, u8 current, bool forceRefresh, u8 op, u8 line)
{
    const u8 OP_VALUE_X = OP_HEADING_X + 4;
    const u8 OP_VALUE_GAP = 4;

    if (last != current || forceRefresh) {
        char buffer[4];
        sprintf(buffer, "%3d", current);
        ui_draw_text(buffer, OP_VALUE_X + (op * OP_VALUE_GAP), OP_BASE_Y + line + 1);
        return true;
    }
    return false;
}

static void update_op_values(const FmChannel* channel, bool forceRefresh)
{
    for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
        const Operator* oper = &channel->operators[op];
        Operator* lastOper = &lastChannel.operators[op];
        if (update_op_value(lastOper->totalLevel, oper->totalLevel, forceRefresh, op, 0))
            lastOper->totalLevel = oper->totalLevel;
        if (update_op_value(lastOper->attackRate, oper->attackRate, forceRefresh, op, 1))
            lastOper->attackRate = oper->attackRate;
        if (update_op_value(lastOper->multiple, oper->multiple, forceRefresh, op, 2))
            lastOper->multiple = oper->multiple;
        if (update_op_value(lastOper->detune, oper->detune, forceRefresh, op, 3))
            lastOper->detune = oper->detune;
        if (update_op_value(lastOper->rateScaling, oper->rateScaling, forceRefresh, op, 4))
            lastOper->rateScaling = oper->rateScaling;
        if (update_op_value(
                lastOper->amplitudeModulation, oper->amplitudeModulation, forceRefresh, op, 5))
            lastOper->amplitudeModulation = oper->amplitudeModulation;
        if (update_op_value(lastOper->decayRate, oper->decayRate, forceRefresh, op, 6))
            lastOper->decayRate = oper->decayRate;
        if (update_op_value(lastOper->sustainRate, oper->sustainRate, forceRefresh, op, 7))
            lastOper->sustainRate = oper->sustainRate;
        if (update_op_value(lastOper->sustainLevel, oper->sustainLevel, forceRefresh, op, 8))
            lastOper->sustainLevel = oper->sustainLevel;
        if (update_op_value(lastOper->releaseRate, oper->releaseRate, forceRefresh, op, 9))
            lastOper->releaseRate = oper->releaseRate;
        if (update_op_value(lastOper->ssgEg, oper->ssgEg, forceRefresh, op, 10))
            lastOper->ssgEg = oper->ssgEg;
    }
}

// Generic function to update a field if its value has changed
static bool update_field_if_changed(UIField* field, const FmDisplayData* data, bool forceRefresh)
{
    u8 currentValue = field->getValue(data);
    if (field->lastValue != currentValue || forceRefresh) {
        ui_draw_text(field->formatFunc(currentValue),
            FM_HEADING_X + field->headerX + field->valueOffset, BASE_Y + field->headerY);
        field->lastValue = currentValue;
        return true;
    }
    return false;
}

static void update_fm_values(void)
{
    const FmChannel* channel = synth_channel_parameters(chanParasFmChan);
    const Global* global = synth_global_parameters();
    const FmDisplayData data = { .global = (Global*)global,
        .channel = (FmChannel*)channel,
        .midiChan = chanParasMidiChan,
        .fmChan = chanParasFmChan };

    for (u8 i = 0; i < LENGTH_OF(fmFields); i++) {
        update_field_if_changed(&fmFields[i], &data, forceRefresh);
    }

    u8 currentAlgorithm = channel->algorithm;
    if (lastChannel.algorithm != currentAlgorithm || forceRefresh) {
        update_algorithm_diagram(currentAlgorithm);
        lastChannel.algorithm = currentAlgorithm;
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
