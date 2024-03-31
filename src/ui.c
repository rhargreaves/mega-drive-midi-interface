#include "ui.h"
#include "buffer.h"
#include "comm.h"
#include "comm_everdrive_pro.h"
#include "comm_serial.h"
#include "comm_megawifi.h"
#include "log.h"
#include "memcmp.h"
#include "midi.h"
#include "midi_receiver.h"
#include "serial.h"
#include "synth.h"
#include "ui_fm.h"
#include <vdp.h>
#include <sys.h>
#include "vdp_bg.h"
#include "vdp_dma.h"
#include "vdp_spr.h"
#include "vdp_tile.h"
#include "vdp_pal.h"
#include "memory.h"
#include <sprite_eng.h>
#include "sprite.h"
#include "scheduler.h"
#include "settings.h"

#define MAX_EFFECTIVE_X (MAX_X - MARGIN_X - MARGIN_X)
#define MAX_EFFECTIVE_Y (MAX_Y - MARGIN_Y - MARGIN_Y)
#define MAX_ERROR_X 30
#define ERROR_Y (MAX_EFFECTIVE_Y - 2)

#define RIGHTED_TEXT_X(text) (MAX_EFFECTIVE_X - (sizeof(text) - 1) + 1)
#define CENTRED_TEXT_X(text) ((MAX_EFFECTIVE_X - (sizeof(text) - 1)) / 2)
#define CHAN_X_GAP 3
#define ACTIVITY_FM_X 6

#define DEVICE_Y 3
#define CHAN_Y 3
#define MIDI_Y (CHAN_Y + 2)
#define ACTIVITY_Y (MIDI_Y + 2)
#define LOG_Y (ACTIVITY_Y + 3)
#define MAX_LOG_LINES 14

#define PALETTE_INDEX(pal, index) ((pal * 16) + index)
#define FONT_COLOUR_INDEX 15
#define BG_COLOUR_INDEX 0

#define FRAMES_BEFORE_UPDATE_CHAN_ACTIVITY 1
#define FRAMES_BEFORE_UPDATE_ACTIVITY 5
#define FRAMES_BEFORE_UPDATE_LOAD 15
#define FRAMES_BEFORE_UPDATE_LOAD_PERCENT 15

#define TILE_LED_INDEX TILE_USERINDEX
#define TILE_ROUTING_INDEX (TILE_LED_INDEX + 8)
#define TILE_BORDERS_INDEX (TILE_ROUTING_INDEX + 4)
#define TILE_IMAGES_INDEX (TILE_BORDERS_INDEX + 8)
#define TILE_DEVICE_FM_INDEX (TILE_IMAGES_INDEX + 7)
#define TILE_DEVICE_PSG_INDEX (TILE_DEVICE_FM_INDEX + 6)
#define TILE_MEGAWIFI_STATUS_INDEX (TILE_DEVICE_PSG_INDEX + 4)

#define TILE_BORDERS_LEFT_CORNER_INDEX (TILE_BORDERS_INDEX)
#define TILE_BORDERS_H_LINE_INDEX (TILE_BORDERS_INDEX + 1)
#define TILE_BORDERS_H_LINE_END_INDEX (TILE_BORDERS_INDEX + 3)
#define TILE_BORDERS_H_LINE_START_INDEX (TILE_BORDERS_INDEX + 4)
#define TILE_BORDERS_RIGHT_CORNER_INDEX (TILE_BORDERS_INDEX + 2)

static const char HEADER[] = "Mega Drive MIDI Interface";
static const char CHAN_HEADER[] = "Ch.   1  2  3  4  5  6  1  2  3  4";
static const char MIDI_HEADER[] = "MIDI";
static const char MIDI_CH_TEXT[17][3] = { " -", " 1", " 2", " 3", " 4", " 5",
    " 6", " 7", " 8", " 9", "10", "11", "12", "13", "14", "15", "16" };

static void init_load(void);
static void print_channels(void);
static void print_header(void);
static void update_load(void);
static u16 load_percent(void);
static void update_key_on_off(void);
static void draw_text(const char* text, u16 x, u16 y);
static void print_chan_activity(u16 busy);
static void print_comm_mode(void);
static void populate_mappings(u8* midiChans);
static void init_routing_mode_tiles(void);
static void print_routing_mode_if_needed(void);
static void print_routing_mode(bool enabled);
static void print_mappings_if_dirty(u8* midiChans);
static void print_mappings(void);

static u16 loadPercentSum = 0;
static bool commInited = false;

static Sprite* activitySprites[DEV_CHANS];

void ui_init(void)
{
    scheduler_addFrameHandler(ui_update);
    SPR_init();
    VDP_setBackgroundColor(BG_COLOUR_INDEX);
    VDP_setPaletteColor(BG_COLOUR_INDEX, RGB24_TO_VDPCOLOR(0x202020));
    VDP_setPaletteColor(
        PALETTE_INDEX(PAL1, FONT_COLOUR_INDEX), RGB24_TO_VDPCOLOR(0xFFFF00));
    VDP_setPaletteColor(
        PALETTE_INDEX(PAL3, FONT_COLOUR_INDEX), RGB24_TO_VDPCOLOR(0x808080));
    print_header();
    print_channels();
    update_load();
    init_load();
    print_comm_mode();
    print_mappings();
    init_routing_mode_tiles();
    print_routing_mode(midi_dynamic_mode());
    SYS_disableInts();

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

static void print_mappings(void)
{
    u8 midiChans[DEV_CHANS] = { 0 };
    populate_mappings(midiChans);
    print_mappings_if_dirty(midiChans);
}

static bool showLogs = true;
static u8 logCurrentY = 0;

static void clear_log_area(void)
{
    VDP_clearTextArea(
        MARGIN_X, LOG_Y + MARGIN_Y, MAX_EFFECTIVE_X, MAX_LOG_LINES);
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
    draw_text(log->msg, 0, LOG_Y + logCurrentY);
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
    v_sprintf(t, "%-5u", scheduler_ticks());
    draw_text(t, 0, 1);
}

void ui_update(void)
{
    update_key_on_off();

    static u8 activityFrame = 0;
    if (++activityFrame == FRAMES_BEFORE_UPDATE_ACTIVITY) {
        activityFrame = 0;
        print_mappings();
        print_comm_mode();
        print_log();
        print_routing_mode_if_needed();
        if (settings_debug_ticks()) {
            print_ticks();
        }
    }

    static u8 loadCalculationFrame = 0;
    if (++loadCalculationFrame == FRAMES_BEFORE_UPDATE_LOAD_PERCENT) {
        loadCalculationFrame = 0;
        loadPercentSum += load_percent();
    }

    static u8 loadFrame = 0;
    if (++loadFrame == FRAMES_BEFORE_UPDATE_LOAD) {
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

static void set_tile(u16 tileIndex, u16 x, u16 y)
{
    VDP_setTileMapXY(
        BG_A, TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, tileIndex), x, y);
}

static void print_header(void)
{
    draw_text(HEADER, 5, 0);
    draw_text(BUILD, RIGHTED_TEXT_X(BUILD), 0);

    VDP_loadTileSet(&ts_borders, TILE_BORDERS_INDEX, DMA);
    set_tile(TILE_BORDERS_LEFT_CORNER_INDEX, 7, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 8, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 9, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 10, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 11, DEVICE_Y);

    VDP_drawImageEx(BG_A, &img_device_fm,
        TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_DEVICE_FM_INDEX), 12,
        DEVICE_Y, FALSE, FALSE);

    set_tile(TILE_BORDERS_H_LINE_INDEX, 18, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 19, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 20, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 21, DEVICE_Y);
    set_tile(TILE_BORDERS_RIGHT_CORNER_INDEX, 22, DEVICE_Y);

    set_tile(TILE_BORDERS_LEFT_CORNER_INDEX, 25, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 26, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 27, DEVICE_Y);

    VDP_drawImageEx(BG_A, &img_device_psg,
        TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_DEVICE_PSG_INDEX), 28,
        DEVICE_Y, FALSE, FALSE);

    set_tile(TILE_BORDERS_H_LINE_INDEX, 32, DEVICE_Y);
    set_tile(TILE_BORDERS_H_LINE_INDEX, 33, DEVICE_Y);
    set_tile(TILE_BORDERS_RIGHT_CORNER_INDEX, 34, DEVICE_Y);
}

static void print_channels(void)
{
    VDP_setTextPalette(PAL3);
    draw_text(CHAN_HEADER, 0, CHAN_Y);
    draw_text(MIDI_HEADER, 0, MIDI_Y);
    draw_text("Act.", 0, ACTIVITY_Y);
    VDP_setTextPalette(PAL0);
}

static void update_key_on_off(void)
{
    static u16 lastBusy = 0;
    u16 busy = synth_busy() | (midi_psg_busy() << 6);
    if (busy != lastBusy) {
        print_chan_activity(busy);
        lastBusy = busy;
    }
}

static u8 midi_chan_for_ui(DeviceChannel* mappings, u8 index)
{
    return (mappings[index].midiChannel) + 1;
}

static void print_mappings_if_dirty(u8* midiChans)
{
    static u8 lastMidiChans[DEV_CHANS];
    if (memcmp(lastMidiChans, midiChans, sizeof(u8) * DEV_CHANS) == 0) {
        return;
    }
    memcpy(lastMidiChans, midiChans, sizeof(u8) * DEV_CHANS);
    for (u8 ch = 0; ch < 10; ch++) {
        draw_text(MIDI_CH_TEXT[midiChans[ch]], 5 + (ch * 3), MIDI_Y);
    }
}

static void populate_mappings(u8* midiChans)
{
    DeviceChannel* chans = midi_channel_mappings();
    for (u8 i = 0; i < DEV_CHANS; i++) {
        midiChans[i] = midi_chan_for_ui(chans, i);
    }
}

static void print_chan_activity(u16 busy)
{
    for (u8 chan = 0; chan < MAX_FM_CHANS + MAX_PSG_CHANS; chan++) {
        SPR_setFrame(activitySprites[chan], ((busy >> chan) & 1) ? 1 : 0);
    }
    SPR_update();
}

static void print_megawifi_info(void)
{
    const Image* MW_IMAGES[]
        = { &img_megawifi_not_detected, &img_megawifi_detected,
              &img_megawifi_listening, &img_megawifi_connected };
    u16 index = 0;
    switch (comm_megawifi_status()) {
    case NotDetected:
        index = 0;
        break;
    case Detected:
        index = 1;
        break;
    case Listening:
        index = 2;
        break;
    case Connected:
        index = 3;
        break;
    }

    VDP_drawImageEx(BG_A, MW_IMAGES[index],
        TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_MEGAWIFI_STATUS_INDEX), 17,
        MAX_EFFECTIVE_Y + 1, FALSE, FALSE);
}

static void print_comm_mode(void)
{
    if (commInited) {
        return;
    }
    const Image* MODES_IMAGES[]
        = { &img_comm_waiting, &img_comm_ed_usb, &img_comm_ed_pro_usb,
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
        TILE_ATTR_FULL(PAL2, 0, FALSE, FALSE, TILE_IMAGES_INDEX), 9,
        MAX_EFFECTIVE_Y + 1, FALSE, FALSE);

    if (settings_is_megawifi_rom()) {
        print_megawifi_info();
    }
}

static void init_load(void)
{
    draw_text("%", 0, MAX_EFFECTIVE_Y);
    VDP_setPaletteColors((PAL2 * 16), pal_load.data, pal_load.length);
    VDP_loadTileSet(&ts_load, TILE_LED_INDEX, CPU);
}

static void print_load_text(u16 percent)
{
    static char text_buffer[3];
    if (percent > 99) {
        percent = 99;
    }
    v_sprintf(text_buffer, "%-2i", percent);
    draw_text(text_buffer, 5, MAX_EFFECTIVE_Y);
}

#define LED_TILE_COUNT 4
#define LED_TILE_OFF_OFFSET (LED_TILE_COUNT)
#define LOAD_TILE_Y (MAX_EFFECTIVE_Y + 1)

static void update_load(void)
{
    u16 percent = loadPercentSum
        / (FRAMES_BEFORE_UPDATE_LOAD / FRAMES_BEFORE_UPDATE_LOAD_PERCENT);
    loadPercentSum = 0;

    s16 led_level = percent == 0 ? 0 : (percent / (100 / LED_TILE_COUNT)) + 1;
    for (u16 i = 0; i < LED_TILE_COUNT; i++) {
        u16 tile_index = TILE_LED_INDEX + i + LED_TILE_OFF_OFFSET;
        if (i < led_level) {
            tile_index -= LED_TILE_OFF_OFFSET;
        }
        set_tile(tile_index, 2 + i, LOAD_TILE_Y);
    }
    if (settings_debug_load()) {
        print_load_text(percent);
    }
    comm_reset_counts();
}

static void init_routing_mode_tiles(void)
{
    set_tile(TILE_ROUTING_INDEX, MAX_EFFECTIVE_X, MIDI_Y + 1);
    set_tile(TILE_ROUTING_INDEX + 1, MAX_EFFECTIVE_X + 1, MIDI_Y + 1);
    set_tile(TILE_ROUTING_INDEX + 2, MAX_EFFECTIVE_X, MIDI_Y + 2);
    set_tile(TILE_ROUTING_INDEX + 3, MAX_EFFECTIVE_X + 1, MIDI_Y + 2);
}

static void print_routing_mode(bool enabled)
{
    VDP_loadTileSet(
        enabled ? &ts_dynamic : &ts_static, TILE_ROUTING_INDEX, DMA);
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
