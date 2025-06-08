#include "ui.h"
#include "comm/comm.h"
#include "comm/comm_everdrive.h"
#include "comm/comm_everdrive_pro.h"
#include "comm/comm_serial.h"
#include "comm/comm_megawifi.h"
#include "log.h"
#include "utils.h"
#include "midi.h"
#include "midi_rx.h"
#include "comm/serial.h"
#include "synth.h"
#include "ui_fm.h"
#include "sprite.h"
#include "scheduler.h"
#include "settings.h"

#define RIGHTED_TEXT_X(text) (MAX_EFFECTIVE_X - (sizeof(text) - 1) + 1)
#define CENTRED_TEXT_X(text) ((MAX_EFFECTIVE_X - (sizeof(text) - 1)) / 2)

#define DEVICE_X 1
#define FM_DEVICE_Y 3
#define PSG_DEVICE_Y 14
#define CHAN_Y 2
#define MIDI_Y (CHAN_Y)
#define MAX_LOG_LINES 3
#define LOG_Y (MAX_EFFECTIVE_Y - MAX_LOG_LINES - 1)
#define LOG_X 2
#define COMM_EXTRA_X 17
#define PITCH_X 6

#define ROUTING_X 1
#define ROUTING_Y (MAX_EFFECTIVE_Y - 2)

#define PALETTE_INDEX(pal, index) ((pal * 16) + index)
#define FONT_COLOUR_INDEX 15
#define BG_COLOUR_INDEX 0

#define FRAMES_BEFORE_UPDATE_CHAN_ACTIVITY 1
#define FRAMES_BEFORE_UPDATE_ACTIVITY 5
#define FRAMES_BEFORE_UPDATE_LOAD 15
#define FRAMES_BEFORE_UPDATE_LOAD_PERCENT 15

#define TILE_LED_INDEX TILE_USER_INDEX
#define TILE_ROUTING_INDEX (TILE_LED_INDEX + 8)
#define TILE_BORDERS_INDEX (TILE_ROUTING_INDEX + 4)
#define TILE_IMAGES_INDEX (TILE_BORDERS_INDEX + 8)
#define TILE_DEVICE_FM_INDEX (TILE_IMAGES_INDEX + 7)
#define TILE_DEVICE_PSG_INDEX (TILE_DEVICE_FM_INDEX + 6)

#define TILE_MEGAWIFI_STATUS_INDEX (TILE_DEVICE_PSG_INDEX + 4)
#define TILE_WAITING_ED_INDEX (TILE_MEGAWIFI_STATUS_INDEX + 5)
#define TILE_SP_INDEX (TILE_WAITING_ED_INDEX + 8)

#define TILE_BORDERS_LINE_START_INDEX (TILE_BORDERS_INDEX)
#define TILE_BORDERS_LINE_INDEX (TILE_BORDERS_INDEX + 1)
#define TILE_BORDERS_LINE_END_INDEX (TILE_BORDERS_INDEX + 2)

#define LED_TILE_COUNT 4
#define LED_TILE_OFF_OFFSET (LED_TILE_COUNT)
#define LOAD_TILE_Y (MAX_EFFECTIVE_Y + 1)

static const char HEADER[] = "Mega Drive MIDI Interface";
static const char MIDI_CH_TEXT[16][3] = { " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9",
    "10", "11", "12", "13", "14", "15", "16" };
static const char DEV_CH_TEXT[10][2] = { "1", "2", "3", "4", "5", "6", "1", "2", "3", "4" };
static const char MIDI_CH_UNASSIGNED_TEXT[] = " -";

#define MIN_UI_MIDI_PITCH 21 // A0

static const char MIDI_PITCH_NAMES[128 - MIN_UI_MIDI_PITCH][4] = { "A-0", "A#0", "B-0", "C-1",
    "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1", "C-2", "C#2",
    "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2", "C-3", "C#3", "D-3",
    "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3", "C-4", "C#4", "D-4", "D#4",
    "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4", "C-5", "C#5", "D-5", "D#5", "E-5",
    "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5", "C-6", "C#6", "D-6", "D#6", "E-6", "F-6",
    "F#6", "G-6", "G#6", "A-6", "A#6", "B-6", "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7",
    "G-7", "G#7", "A-7", "A#7", "B-7", "C-8", "C#8", "D-8", "D#8", "E-8", "F-8", "F#8", "G-8",
    "G#8", "A-8", "A#8", "B-8", "C-9", "C#9", "D-9", "D#9", "E-9", "F-9", "F#9", "G-9" };

static void init_load(void);
static void print_channels(void);
static void print_header(void);
static void update_load(void);
static u16 load_percent(void);
static void update_key_on_off(void);
static void draw_text(const char* text, u16 x, u16 y);
static void print_chan_activity(u8* pitches);
static void print_comm_mode(void);
static void init_routing_mode_tiles(void);
static void print_routing_mode_if_needed(void);
static void print_routing_mode(bool enabled);
static void print_mappings_if_dirty(u8* midiChans);
static void print_mappings(void);

static u8 lastPitches[DEV_PHYSICAL_CHANS] = { 0 };

static u16 loadPercentSum = 0;
static bool commInited = false;

void ui_init(void)
{
    scheduler_addFrameHandler(ui_update);
    SPR_init();

    // 0x0000A0 = blue
    // 0xe0ffff = light blue / white
    // 0x3b2dee = purple blue

    VDP_loadFont(custom_font.tileset, DMA);
    VDP_setBackgroundColor(BG_COLOUR_INDEX);
    PAL_setColors((PAL0 * 16), pal_0.data, pal_0.length, CPU);
    PAL_setColors((PAL2 * 16), pal_2.data, pal_2.length, CPU);

    PAL_setColor(PALETTE_INDEX(PAL3, 1), RGB24_TO_VDPCOLOR(0x808080));
    PAL_setColor(PALETTE_INDEX(PAL3, 2), RGB24_TO_VDPCOLOR(0x3b2dee));
    print_header();
    print_channels();

    update_load();
    init_load();
    print_comm_mode();
    print_mappings();
    init_routing_mode_tiles();
    print_routing_mode(midi_dynamic_mode());
    ui_fm_init();
}

static void update_ch3sp(bool enabled)
{
    if (enabled) {
        VDP_drawImageEx(BG_A, &img_sp, TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, TILE_SP_INDEX),
            DEVICE_X + 2, FM_DEVICE_Y + (2 * 2), FALSE, FALSE);
    } else {
        VDP_clearTextArea(DEVICE_X + 2, FM_DEVICE_Y + (2 * 2), 1, 1);
    }
}

static void print_mappings(void)
{
    u8 midiChans[DEV_PHYSICAL_CHANS] = { 0 };
    DeviceChannel* chans = midi_channel_mappings();
    for (u8 i = 0; i < DEV_PHYSICAL_CHANS; i++) {
        midiChans[i] = chans[i].midiChannel;
    }
    print_mappings_if_dirty(midiChans);
}

static bool showLogs = true;
static u8 logCurrentY = 0;

static void clear_log_area(void)
{
    VDP_clearTextArea(
        MARGIN_X + LOG_X, LOG_Y + MARGIN_Y, MAX_EFFECTIVE_X - LOG_X, MAX_LOG_LINES + 1);
    logCurrentY = 0;
}

static void print_log(void)
{
    if (!showLogs) {
        return;
    }

    Log* log = log_dequeue();
    if (log == NULL) {
        return;
    }
    if (logCurrentY >= MAX_LOG_LINES) {
        clear_log_area();
        logCurrentY = 0;
    }
    switch (log->level) {
    case Warn:
        VDP_setTextPalette(PAL1);
        break;
    default:
        VDP_setTextPalette(PAL2);
        break;
    }
    draw_text(log->msg, MARGIN_X + LOG_X, LOG_Y + MARGIN_Y + logCurrentY);
    VDP_setTextPalette(PAL0);
    logCurrentY++;
}

void ui_show_logs(void)
{
    showLogs = true;
}

void ui_hide_logs(void)
{
    clear_log_area();
    showLogs = false;
}

static void print_ticks(void)
{
    char t[6];
    sprintf(t, "%-5u", scheduler_ticks());
    draw_text(t, 0, 1);
}

static bool lastCh3SpecialMode = false;

void ui_update(u16 delta)
{
    update_key_on_off();

    static u16 activityFrame = 0;
    activityFrame += delta;
    if (activityFrame >= FRAMES_BEFORE_UPDATE_ACTIVITY) {
        activityFrame = 0;
        print_mappings();
        print_comm_mode();
        print_log();
        print_routing_mode_if_needed();
        if (settings_debug_ticks()) {
            print_ticks();
        }

        bool ch3SpecialMode = synth_global_parameters()->specialMode;
        if (ch3SpecialMode != lastCh3SpecialMode) {
            update_ch3sp(ch3SpecialMode);
            lastCh3SpecialMode = ch3SpecialMode;
        }
    }

    static u16 loadCalculationFrame = 0;
    loadCalculationFrame += delta;
    if (loadCalculationFrame >= FRAMES_BEFORE_UPDATE_LOAD_PERCENT) {
        loadCalculationFrame = 0;
        loadPercentSum += load_percent();
    }

    static u16 loadFrame = 0;
    loadFrame += delta;
    if (loadFrame >= FRAMES_BEFORE_UPDATE_LOAD) {
        loadFrame = 0;
        update_load();
    }

    ui_fm_update();
    SYS_doVBlankProcessEx(IMMEDIATELY);
}

static u16 load_percent(void)
{
    u16 idle = comm_idle_count();
    u16 busy = comm_busy_count();
    if (idle == 0 && busy == 0) {
        return 0;
    }
    return (busy * 100) / (idle + busy);
}

void ui_draw_text(const char* text, u16 x, u16 y)
{
    draw_text(text, x, y);
}

static void draw_text(const char* text, u16 x, u16 y)
{
    VDP_drawText(text, MARGIN_X + x, MARGIN_Y + y);
}

static void set_tile(u16 tileIndex, u8 pal, u16 x, u16 y)
{
    VDP_setTileMapXY(BG_A, TILE_ATTR_FULL(pal, 0, FALSE, FALSE, tileIndex), x, y);
}

static void print_header(void)
{
    draw_text(HEADER, 5, 0);
    draw_text(BUILD, RIGHTED_TEXT_X(BUILD), 0);

    VDP_loadTileSet(&ts_borders, TILE_BORDERS_INDEX, DMA);

    set_tile(TILE_BORDERS_LINE_START_INDEX, PAL3, DEVICE_X, FM_DEVICE_Y);
    set_tile(TILE_BORDERS_LINE_INDEX, PAL3, DEVICE_X, FM_DEVICE_Y + 1);
    set_tile(TILE_BORDERS_LINE_INDEX, PAL3, DEVICE_X, FM_DEVICE_Y + 2);
    VDP_drawImageEx(BG_A, &img_device_fm,
        TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, TILE_DEVICE_FM_INDEX), DEVICE_X, FM_DEVICE_Y + 3,
        FALSE, FALSE);
    set_tile(TILE_BORDERS_LINE_INDEX, PAL3, DEVICE_X, FM_DEVICE_Y + 8);
    set_tile(TILE_BORDERS_LINE_INDEX, PAL3, DEVICE_X, FM_DEVICE_Y + 9);
    set_tile(TILE_BORDERS_LINE_END_INDEX, PAL3, DEVICE_X, FM_DEVICE_Y + 10);

    set_tile(TILE_BORDERS_LINE_START_INDEX, PAL3, DEVICE_X, PSG_DEVICE_Y + 1);
    set_tile(TILE_BORDERS_LINE_INDEX, PAL3, DEVICE_X, PSG_DEVICE_Y + 2);
    VDP_drawImageEx(BG_A, &img_device_psg,
        TILE_ATTR_FULL(PAL3, 0, FALSE, FALSE, TILE_DEVICE_PSG_INDEX), DEVICE_X, PSG_DEVICE_Y + 3,
        FALSE, FALSE);
    set_tile(TILE_BORDERS_LINE_INDEX, PAL3, DEVICE_X, PSG_DEVICE_Y + 6);
    set_tile(TILE_BORDERS_LINE_END_INDEX, PAL3, DEVICE_X, PSG_DEVICE_Y + 7);
}

static void print_channels(void)
{
    VDP_setTextPalette(PAL3);
    for (u8 i = 0; i < 10; i++) {
        draw_text(DEV_CH_TEXT[i], 1, MIDI_Y + (i * 2));
    }
    VDP_setTextPalette(PAL0);
}

static void update_key_on_off(void)
{
    u8 pitches[DEV_PHYSICAL_CHANS];
    for (u8 chan = 0; chan < MAX_FM_CHANS + MAX_PSG_CHANS; chan++) {
        DeviceChannel* devChan = &midi_channel_mappings()[chan];
        if (devChan->noteOn) {
            pitches[chan] = devChan->pitch;
        } else {
            pitches[chan] = 0;
        }
    }
    print_chan_activity(pitches);
}

static void print_mappings_if_dirty(u8* midiChans)
{
    static u8 lastMidiChans[DEV_PHYSICAL_CHANS];
    if (memcmp(lastMidiChans, midiChans, sizeof(u8) * DEV_PHYSICAL_CHANS) == 0) {
        return;
    }

    for (u8 devChan = 0; devChan < 10; devChan++) {
        u8 midiCh = midiChans[devChan];
        const char* text;
        if (midiCh == UNASSIGNED_MIDI_CHANNEL) {
            text = MIDI_CH_UNASSIGNED_TEXT;
        } else {
            text = MIDI_CH_TEXT[midiChans[devChan]];
        }
        draw_text(text, 3, MIDI_Y + (devChan * 2));
    }
}

static void print_chan_activity(u8* pitches)
{
    bool setPalette = false;
    for (u8 i = 0; i < DEV_PHYSICAL_CHANS; i++) {
        u8 pitch = pitches[i];
        if (pitch == lastPitches[i]) {
            continue;
        }
        if (pitch == 0) {
            VDP_clearTextArea(PITCH_X + MARGIN_X, MIDI_Y + (i * 2) + MARGIN_Y, 3, 1);
        } else {
            if (!setPalette) {
                VDP_setTextPalette(PAL2);
                setPalette = true;
            }
            draw_text(
                pitch > MIN_UI_MIDI_PITCH ? MIDI_PITCH_NAMES[pitch - MIN_UI_MIDI_PITCH] : "???",
                PITCH_X, MIDI_Y + (i * 2));
        }
        lastPitches[i] = pitch;
    }
    if (setPalette) {
        VDP_setTextPalette(PAL0);
    }
}

static void print_megawifi_info(void)
{
    const Image* MW_IMAGES[] = { &img_megawifi_detecting, &img_megawifi_not_detected,
        &img_megawifi_initialising, &img_megawifi_listening, &img_megawifi_connected };

    u16 index = comm_megawifi_status();
    if (index >= LENGTH_OF(MW_IMAGES)) {
        log_warn("Invalid MegaWiFi status: %d", index);
        return;
    }

    VDP_clearTextArea(17, MAX_EFFECTIVE_Y + 1, 15, 1);
    VDP_drawImageEx(BG_A, MW_IMAGES[index],
        TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_MEGAWIFI_STATUS_INDEX), 17, MAX_EFFECTIVE_Y + 1,
        FALSE, FALSE);
}

static void print_comm_mode(void)
{
    if (commInited) {
        return;
    }

    if (comm_everdrive_pro_is_present()) {
        VDP_drawImageEx(BG_A, &img_wait_edpro,
            TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_WAITING_ED_INDEX), COMM_EXTRA_X,
            MAX_EFFECTIVE_Y + 1, FALSE, FALSE);
    } else if (comm_everdrive_is_present()) {
        VDP_drawImageEx(BG_A, &img_wait_edx7,
            TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_WAITING_ED_INDEX), COMM_EXTRA_X,
            MAX_EFFECTIVE_Y + 1, FALSE, FALSE);
    }

    const Image* MODES_IMAGES[] = { &img_comm_waiting, &img_comm_ed_usb, &img_comm_ed_pro_usb,
        &img_comm_serial, &img_comm_megawifi, &img_comm_demo, 0 };
    u16 index;
    switch (comm_mode()) {
    case Discovery:
        index = 0;
        break;
    case Everdrive:
        index = 1;
        commInited = true;
        break;
    case EverdrivePro:
        index = 2;
        commInited = true;
        break;
    case Serial:
        index = 3;
        commInited = true;
        break;
    case MegaWiFi:
        index = 4;
        commInited = true;
        break;
    case Demo:
        index = 5;
        commInited = true;
        break;
    default:
        index = 0;
        break;
    }

    VDP_drawImageEx(BG_A, MODES_IMAGES[index],
        TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_IMAGES_INDEX), 9, MAX_EFFECTIVE_Y + 1, FALSE,
        FALSE);

    if (commInited) {
        VDP_clearTextArea(COMM_EXTRA_X, MAX_EFFECTIVE_Y + 1, 5, 1);
    }

    if (settings_is_megawifi_rom()) {
        print_megawifi_info();
    }
}

static void init_load(void)
{
    draw_text("%", 0, MAX_EFFECTIVE_Y);
    VDP_loadTileSet(&ts_load, TILE_LED_INDEX, CPU);
}

static void print_load_text(u16 percent)
{
    static char text_buffer[3];
    if (percent > 99) {
        percent = 99;
    }
    sprintf(text_buffer, "%-2i", percent);
    draw_text(text_buffer, 5, MAX_EFFECTIVE_Y);
}

static void update_load(void)
{
    u16 percent = loadPercentSum / (FRAMES_BEFORE_UPDATE_LOAD / FRAMES_BEFORE_UPDATE_LOAD_PERCENT);
    loadPercentSum = 0;

    s16 led_level = percent == 0 ? 0 : (percent / (100 / LED_TILE_COUNT)) + 1;
    for (u16 i = 0; i < LED_TILE_COUNT; i++) {
        u16 tile_index = TILE_LED_INDEX + i + LED_TILE_OFF_OFFSET;
        if (i < led_level) {
            tile_index -= LED_TILE_OFF_OFFSET;
        }
        set_tile(tile_index, PAL2, 2 + i, LOAD_TILE_Y);
    }
    if (settings_debug_load()) {
        print_load_text(percent);
    }
    comm_reset_counts();
}

static void init_routing_mode_tiles(void)
{
    set_tile(TILE_ROUTING_INDEX, PAL3, ROUTING_X, ROUTING_Y);
    set_tile(TILE_ROUTING_INDEX + 1, PAL3, ROUTING_X + 1, ROUTING_Y);
    set_tile(TILE_ROUTING_INDEX + 2, PAL3, ROUTING_X, ROUTING_Y + 1);
    set_tile(TILE_ROUTING_INDEX + 3, PAL3, ROUTING_X + 1, ROUTING_Y + 1);
}

static void print_routing_mode(bool enabled)
{
    VDP_loadTileSet(enabled ? &ts_dynamic : &ts_static, TILE_ROUTING_INDEX, DMA);
}

static void print_routing_mode_if_needed(void)
{
    static bool lastDynamicModeStatus = false;
    bool enabled = midi_dynamic_mode();
    if (lastDynamicModeStatus != enabled) {
        print_routing_mode(enabled);
        lastDynamicModeStatus = enabled;
    }
}
