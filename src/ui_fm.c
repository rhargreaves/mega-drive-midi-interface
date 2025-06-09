#include "ui_fm.h"
#include "midi.h"
#include "synth.h"
#include "sprite.h"
#include "ui.h"
#include "utils.h"

#define UNKNOWN_FM_CHANNEL 0xFF

#define BASE_Y 2
#define OP_BASE_Y (BASE_Y + 7)

#define FM_HEADING_X 18
#define OP_HEADING_X FM_HEADING_X

#define ALGORITHM_X (FM_HEADING_X + 14)
#define ALGORITHM_Y (BASE_Y + 1)

typedef const char* FormatTextFunc(u8 value);

typedef struct FmDisplayData {
    Global* global;
    FmChannel* channel;
    u8 midiChan;
    u8 fmChan;
} FmDisplayData;

typedef struct UIField {
    u8 headerX;
    u8 headerY;
    u8 valueOffset;
    const char* name;
    const char* (*formatFunc)(u8 value);
    u8 (*getValue)(const FmDisplayData* data);
} UIField;

typedef struct UIOpField {
    u8 line;
    const char* name;
    u8 (*getValue)(const FmChannel* channel, u8 op);
} UIOpField;

static bool synthParameterValuesDirty = false;
static bool showChanParameters = false;
static u8 chanParasMidiChan = 0;
static u8 chanParasFmChan = 0;
static Sprite* algorSprites[FM_ALGORITHMS];

static u8 lastAlgorithm = 0xFF;
static bool forceRefresh = false;

static void update_fm_values(void);
static void init_algorithm_sprites(void);
static void update_fm_values_if_chan_selected(void);
static void synth_parameter_updated(u8 fmChan, ParameterUpdated parameterUpdated);

static const char* stereo_text(u8 stereo);
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
static u8 get_stereo(const FmDisplayData* data);
static u8 get_lfo_frequency(const FmDisplayData* data);
static u8 get_op_total_level(const FmChannel* channel, u8 op);
static u8 get_op_attack_rate(const FmChannel* channel, u8 op);
static u8 get_op_multiple(const FmChannel* channel, u8 op);
static u8 get_op_detune(const FmChannel* channel, u8 op);
static u8 get_op_rate_scaling(const FmChannel* channel, u8 op);
static u8 get_op_amplitude_modulation(const FmChannel* channel, u8 op);
static u8 get_op_decay_rate(const FmChannel* channel, u8 op);
static u8 get_op_sustain_rate(const FmChannel* channel, u8 op);
static u8 get_op_sustain_level(const FmChannel* channel, u8 op);
static u8 get_op_release_rate(const FmChannel* channel, u8 op);
static u8 get_op_ssg_eg(const FmChannel* channel, u8 op);

static const UIField fmFields[] = {
    { 0, 0, 3, "Ch", chan_number, get_midi_channel },
    { 7, 0, 3, "FM", chan_number, get_fm_channel },
    { 0, 2, 4, "Alg", format_num, get_algorithm },
    { 7, 2, 3, "Fb", format_num, get_feedback },
    { 0, 3, 4, "Pan", stereo_text, get_stereo },
    { 0, 4, 4, "LFO", lfo_freq_text, get_lfo_frequency },
    { 0, 5, 4, "FMS", fms_text, get_fms },
    { 8, 5, 4, "AMS", ams_text, get_ams },
};

static const UIOpField opFields[] = {
    { 0, " TL", get_op_total_level },
    { 1, " AR", get_op_attack_rate },
    { 2, "MUL", get_op_multiple },
    { 3, " DT", get_op_detune },
    { 4, " RS", get_op_rate_scaling },
    { 5, " AM", get_op_amplitude_modulation },
    { 6, " DR", get_op_decay_rate },
    { 7, " SR", get_op_sustain_rate },
    { 8, " SL", get_op_sustain_level },
    { 9, " RR", get_op_release_rate },
    { 10, "SSG", get_op_ssg_eg },
};

static u8 fmLastValues[LENGTH_OF(fmFields)];
static u8 opLastValues[LENGTH_OF(opFields)][MAX_FM_OPERATORS];

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
    if (fmChan == chanParasFmChan || parameterUpdated == Lfo) {
        synthParameterValuesDirty = true;
    }
}

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

static u8 get_stereo(const FmDisplayData* data)
{
    return data->channel->stereo;
}

static u8 get_lfo_frequency(const FmDisplayData* data)
{
    if (!data->global->lfoEnable) {
        return 0;
    }
    return data->global->lfoFrequency + 1;
}

static u8 get_op_total_level(const FmChannel* channel, u8 op)
{
    return channel->operators[op].totalLevel;
}

static u8 get_op_attack_rate(const FmChannel* channel, u8 op)
{
    return channel->operators[op].attackRate;
}

static u8 get_op_multiple(const FmChannel* channel, u8 op)
{
    return channel->operators[op].multiple;
}

static u8 get_op_detune(const FmChannel* channel, u8 op)
{
    return channel->operators[op].detune;
}

static u8 get_op_rate_scaling(const FmChannel* channel, u8 op)
{
    return channel->operators[op].rateScaling;
}

static u8 get_op_amplitude_modulation(const FmChannel* channel, u8 op)
{
    return channel->operators[op].amplitudeModulation;
}

static u8 get_op_decay_rate(const FmChannel* channel, u8 op)
{
    return channel->operators[op].decayRate;
}

static u8 get_op_sustain_rate(const FmChannel* channel, u8 op)
{
    return channel->operators[op].sustainRate;
}

static u8 get_op_sustain_level(const FmChannel* channel, u8 op)
{
    return channel->operators[op].sustainLevel;
}

static u8 get_op_release_rate(const FmChannel* channel, u8 op)
{
    return channel->operators[op].releaseRate;
}

static u8 get_op_ssg_eg(const FmChannel* channel, u8 op)
{
    return channel->operators[op].ssgEg;
}

static void print_channel_parameter_headings(void)
{
    VDP_setTextPalette(PAL3);
    ui_draw_text("    Op1   2   3   4", OP_HEADING_X, OP_BASE_Y);
    for (u8 i = 0; i < LENGTH_OF(opFields); i++) {
        ui_draw_text(opFields[i].name, OP_HEADING_X, OP_BASE_Y + opFields[i].line + 1);
    }
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

static const char* lfo_freq_text(u8 lfoFreq)
{
    static const char TEXT[][8]
        = { "Off  ", "4Hz  ", "5.5Hz", "6Hz  ", "6.5Hz", "7Hz  ", "10Hz ", "48Hz ", "72Hz " };
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
        = { "0%  ", "3%  ", "6%  ", "10% ", "14% ", "20% ", "40% ", "80% " };
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

static bool update_op_field_if_changed(
    u8 fieldIndex, const FmChannel* channel, u8 op, bool forceRefresh)
{
    const u8 OP_VALUE_X = OP_HEADING_X + 4;
    const u8 OP_VALUE_GAP = 4;
    const UIOpField* field = &opFields[fieldIndex];

    u8 currentValue = field->getValue(channel, op);
    if (opLastValues[fieldIndex][op] != currentValue || forceRefresh) {
        char buffer[4];
        sprintf(buffer, "%3d", currentValue);
        ui_draw_text(buffer, OP_VALUE_X + (op * OP_VALUE_GAP), OP_BASE_Y + field->line + 1);
        opLastValues[fieldIndex][op] = currentValue;
        return true;
    }
    return false;
}

static void update_op_values(const FmChannel* channel, bool forceRefresh)
{
    for (u8 i = 0; i < LENGTH_OF(opFields); i++) {
        for (u8 op = 0; op < MAX_FM_OPERATORS; op++) {
            update_op_field_if_changed(i, channel, op, forceRefresh);
        }
    }
}

static bool update_field_if_changed(u8 fieldIndex, const FmDisplayData* data, bool forceRefresh)
{
    const UIField* field = &fmFields[fieldIndex];
    u8 currentValue = field->getValue(data);
    if (fmLastValues[fieldIndex] != currentValue || forceRefresh) {
        ui_draw_text(field->formatFunc(currentValue),
            FM_HEADING_X + field->headerX + field->valueOffset, BASE_Y + field->headerY);
        fmLastValues[fieldIndex] = currentValue;
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
        update_field_if_changed(i, &data, forceRefresh);
    }

    u8 currentAlgorithm = channel->algorithm;
    if (lastAlgorithm != currentAlgorithm || forceRefresh) {
        update_algorithm_diagram(currentAlgorithm);
        lastAlgorithm = currentAlgorithm;
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
        forceRefresh = true;
        print_channel_parameter_headings();
    } else {
        VDP_clearTextArea(FM_HEADING_X, MARGIN_Y + BASE_Y, MAX_X, 20);
        hide_all_algorithms();
        SPR_update();
    }
    synthParameterValuesDirty = true;
}
