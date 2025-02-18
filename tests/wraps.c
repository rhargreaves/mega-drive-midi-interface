#include "wraps.h"
#include "cmocka_inc.h"
#include "debug.h"

static bool disableChecks = false;
static bool loggingChecks = false;

int _eflash;

void wraps_disable_checks(void)
{
    disableChecks = true;
}

void wraps_disable_logging_checks(void)
{
    loggingChecks = false;
}

void wraps_enable_logging_checks(void)
{
    loggingChecks = true;
}

void wraps_enable_checks(void)
{
    disableChecks = false;
}

void __wrap_synth_init(const FmChannel* defaultPreset)
{
    if (disableChecks)
        return;
    check_expected(defaultPreset);
}

void __wrap_synth_enableLfo(u8 enable)
{
    check_expected(enable);
}

void __wrap_synth_globalLfoFrequency(u8 freq)
{
    check_expected(freq);
}

void __wrap_synth_noteOn(u8 channel)
{
    if (disableChecks)
        return;
    check_expected(channel);
}

void __wrap_synth_noteOff(u8 channel)
{
    if (disableChecks)
        return;
    check_expected(channel);
}

void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber)
{
    debug_message("call: synth_pitch(%d, %d, %d)\n", channel, octave, freqNumber);
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(octave);
    check_expected(freqNumber);
}

void __wrap_synth_totalLevel(u8 channel, u8 totalLevel)
{
    check_expected(channel);
    check_expected(totalLevel);
}

void __wrap_synth_stereo(u8 channel, u8 mode)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(mode);
}

void __wrap_synth_algorithm(u8 channel, u8 algorithm)
{
    check_expected(channel);
    check_expected(algorithm);
}

void __wrap_synth_feedback(u8 channel, u8 feedback)
{
    check_expected(channel);
    check_expected(feedback);
}

void __wrap_synth_ams(u8 channel, u8 ams)
{
    check_expected(channel);
    check_expected(ams);
}

void __wrap_synth_fms(u8 channel, u8 fms)
{
    check_expected(channel);
    check_expected(fms);
}

void __wrap_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel)
{
    check_expected(channel);
    check_expected(op);
    check_expected(totalLevel);
}

void __wrap_synth_operatorMultiple(u8 channel, u8 op, u8 multiple)
{
    check_expected(channel);
    check_expected(op);
    check_expected(multiple);
}

void __wrap_synth_operatorDetune(u8 channel, u8 op, u8 detune)
{
    check_expected(channel);
    check_expected(op);
    check_expected(detune);
}

void __wrap_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling)
{
    check_expected(channel);
    check_expected(op);
    check_expected(rateScaling);
}

void __wrap_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(attackRate);
}

void __wrap_synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(decayRate);
}

void __wrap_synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(sustainRate);
}

void __wrap_synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel)
{
    check_expected(channel);
    check_expected(op);
    check_expected(sustainLevel);
}

void __wrap_synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation)
{
    check_expected(channel);
    check_expected(op);
    check_expected(amplitudeModulation);
}

void __wrap_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate)
{
    check_expected(channel);
    check_expected(op);
    check_expected(releaseRate);
}

void __wrap_synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg)
{
    check_expected(channel);
    check_expected(op);
    check_expected(ssgEg);
}

void __wrap_synth_preset(u8 channel, const FmChannel* preset)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(preset);
}

void __wrap_synth_volume(u8 channel, u8 volume)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(volume);
}

const FmChannel* __wrap_synth_channelParameters(u8 channel)
{
    return NULL;
}

const Global* __wrap_synth_globalParameters()
{
    return NULL;
}

void __wrap_synth_setSpecialMode(bool enable)
{
    check_expected(enable);
}

void __wrap_synth_specialModePitch(u8 op, u8 octave, u16 freqNumber)
{
    check_expected(op);
    check_expected(octave);
    check_expected(freqNumber);
}

void __wrap_synth_specialModeVolume(u8 op, u8 volume)
{
    check_expected(op);
    check_expected(volume);
}

void __wrap_synth_directWriteYm2612(u8 part, u8 reg, u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}

void __wrap_synth_enableDac(bool enable)
{
    check_expected(enable);
}

void __wrap_comm_write(u8 data)
{
    check_expected(data);
}

bool __wrap_comm_read_ready(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_read(void)
{
    return mock_type(u8);
}

u16 __wrap_comm_idle_count(void)
{
    return mock_type(u16);
}

u16 __wrap_comm_busy_count(void)
{
    return mock_type(u16);
}

void __wrap_comm_reset_counts(void)
{
}

void __wrap_comm_megawifi_init(void)
{
}

void __wrap_ui_fm_set_parameters_visibility(u8 chan, bool show)
{
    check_expected(chan);
    check_expected(show);
}

void __wrap_ui_update(void)
{
    function_called();
}

void __wrap_VDP_drawText(const char* str, u16 x, u16 y)
{
}

void __wrap_SYS_setVIntCallback(VoidCallback* CB)
{
}

void __wrap_VDP_setTextPalette(u16 palette)
{
}

void __wrap_VDP_clearText(u16 x, u16 y, u16 w)
{
}

void __wrap_PAL_setColor(u16 index, u16 value)
{
}

void __wrap_PAL_setColors(u16 index, const u16* pal, u16 count, TransferMethod tm)
{
}

void __wrap_VDP_setBackgroundColor(u8 index)
{
}

void __wrap_PSG_setEnvelope(u8 channel, u8 value)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(value);
}

void __wrap_PSG_setFrequency(u8 channel, u16 value)
{
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(value);
}

void __wrap_PSG_setTone(u8 channel, u16 value)
{
    debug_message("call: PSG_setTone(%d, %d)\n", channel, value);
    if (disableChecks)
        return;
    check_expected(channel);
    check_expected(value);
}

Sprite* __wrap_SPR_addSprite(const SpriteDefinition* spriteDef, s16 x, s16 y, u16 attribut)
{
    return mock_type(Sprite*);
}

void __wrap_SPR_update()
{
}

void __wrap_SYS_disableInts()
{
}

void __wrap_SYS_enableInts()
{
}

void __wrap_SPR_init(u16 maxSprite, u16 vramSize, u16 unpackBufferSize)
{
}

void __wrap_SPR_setVisibility(Sprite* sprite, SpriteVisibility value)
{
}

void __wrap_VDP_setPaletteColors(u16 index, const u16* values, u16 count)
{
}

void __wrap_comm_everdrive_init(void)
{
}

bool __wrap_comm_everdrive_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_everdrive_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_everdrive_write(u8 data)
{
    check_expected(data);
}

void __wrap_comm_everdrive_pro_init(void)
{
}

bool __wrap_comm_everdrive_pro_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_everdrive_pro_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_pro_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_everdrive_pro_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_everdrive_pro_write(u8 data)
{
    check_expected(data);
}

void __wrap_comm_demo_init(void)
{
}

bool __wrap_comm_demo_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_demo_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_demo_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_demo_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_demo_write(u8 data)
{
}

void __wrap_comm_demo_vsync(void)
{
    function_called();
}

bool __wrap_comm_megawifi_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_megawifi_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_megawifi_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_megawifi_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_megawifi_write(u8 data)
{
    check_expected(data);
}

bool __wrap_mw_uart_is_present(void)
{
    return mock_type(bool);
}

void __wrap_SPR_setAnim(Sprite* sprite, s16 anim)
{
}

void __wrap_SPR_setFrame(Sprite* sprite, s16 frame)
{
}

void __wrap_SPR_setAnimAndFrame(Sprite* sprite, s16 anim, s16 frame)
{
    check_expected(sprite);
    check_expected(anim);
    check_expected(frame);
}

void __wrap_VDP_setReg(u16 reg, u8 value)
{
}

u8 __wrap_VDP_getReg(u16 reg)
{
    return 0;
}

void __wrap_SYS_setExtIntCallback(VoidCallback* CB)
{
}

void __wrap_SYS_setInterruptMaskLevel(u16 value)
{
}

void __wrap_comm_init(void)
{
}

void __wrap_comm_serial_init(void)
{
}

bool __wrap_comm_serial_is_present(void)
{
    return mock_type(bool);
}

u8 __wrap_comm_serial_read_ready(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_serial_read(void)
{
    return mock_type(u8);
}

u8 __wrap_comm_serial_write_ready(void)
{
    return mock_type(u8);
}

void __wrap_comm_serial_write(u8 data)
{
    check_expected(data);
}

void __wrap_log_init(void)
{
}

void __wrap_log_info(const char* fmt, ...)
{
    if (!loggingChecks) {
        return;
    }
    check_expected(fmt);
}

void __wrap_log_warn(const char* fmt, ...)
{
    if (!loggingChecks) {
        return;
    }
    check_expected(fmt);
}

Log* __wrap_log_dequeue(void)
{
    return mock_type(Log*);
}

void __wrap_VDP_clearTextArea(u16 x, u16 y, u16 w, u16 h)
{
}

static u16 testIsPal = false;
u16 __wrap_SYS_isPAL(void)
{
    return testIsPal;
}

void wraps_set_SYS_isPAL(bool isPal)
{
    testIsPal = isPal;
}

void __wrap_comm_megawifi_midiEmitCallback(u8 midiByte)
{
    check_expected(midiByte);
}

mw_err __wrap_mediator_recv_event(void)
{
    function_called();
    return mock_type(mw_err);
}

mw_err __wrap_mediator_send_packet(u8 ch, char* data, u16 len)
{
    check_expected(ch);
    check_expected(data);
    check_expected(len);
    return mock_type(mw_err);
}

void __wrap_SYS_die(char* err)
{
    check_expected(err);
}

int __wrap_mw_init(char* cmd_buf, uint16_t buf_len)
{
    if (disableChecks)
        return MW_ERR_NONE;
    check_expected(cmd_buf);
    check_expected(buf_len);
    return mock_type(int);
}

void __wrap_mw_process(void)
{
    if (disableChecks)
        return;
    function_called();
}

void __wrap_lsd_process(void)
{
    if (disableChecks)
        return;
    function_called();
}

static u8 mock_ver_major;
static u8 mock_ver_minor;

void mock_mw_detect(u8 major, u8 minor)
{
    mock_ver_major = major;
    mock_ver_minor = minor;
}

mw_err __wrap_mw_detect(uint8_t* major, uint8_t* minor, char** variant)
{
    if (disableChecks)
        return MW_ERR_NOT_READY;
    *major = mock_ver_major;
    *minor = mock_ver_minor;
    return mock_type(mw_err);
}

int __wrap_loop_init(uint8_t max_func, uint8_t max_timer)
{
    if (disableChecks)
        return MW_ERR_NONE;
    check_expected(max_func);
    check_expected(max_timer);
    return mock_type(mw_err);
}

mw_err __wrap_mw_ap_assoc(uint8_t slot)
{
    if (disableChecks)
        return MW_ERR_NONE;
    check_expected(slot);
    return mock_type(mw_err);
}

mw_err __wrap_mw_ap_assoc_wait(int tout_frames)
{
    if (disableChecks)
        return MW_ERR_NONE;
    check_expected(tout_frames);
    return mock_type(mw_err);
}

static struct mw_ip_cfg mock_mw_ip_cfg = {};

void mock_ip_cfg(u32 ip_addr)
{
    mock_mw_ip_cfg.addr.addr = ip_addr;
}

mw_err __wrap_mw_ip_current(struct mw_ip_cfg** ip)
{
    (*ip) = &mock_mw_ip_cfg;
    if (disableChecks)
        return MW_ERR_NONE;
    return mock_type(mw_err);
}

mw_err __wrap_mw_udp_set(
    uint8_t ch, const char* dst_addr, const char* dst_port, const char* src_port)
{
    if (disableChecks)
        return MW_ERR_NONE;
    check_expected(ch);
    check_expected(dst_addr);
    check_expected(dst_port);
    check_expected(src_port);
    return mock_type(mw_err);
}

mw_err __wrap_mw_sock_conn_wait(uint8_t ch, int tout_frames)
{
    if (disableChecks)
        return MW_ERR_NONE;
    check_expected(ch);
    check_expected(tout_frames);
    return mock_type(mw_err);
}

void __wrap_midi_receiver_read_if_comm_ready(void)
{
    function_called();
}

void __wrap_scheduler_tick(void)
{
    function_called();
}

void __wrap_scheduler_addTickHandler(HandlerFunc* onTick)
{
    check_expected(onTick);
}

void __wrap_scheduler_addFrameHandler(HandlerFunc* onFrame)
{
    check_expected(onFrame);
}

void __wrap_comm_megawifi_tick(void)
{
    function_called();
}

void __wrap_comm_megawifi_send(u8 ch, char* data, u16 len)
{
    check_expected(ch);
    check_expected(data);
    check_expected(len);
}

enum lsd_status __wrap_lsd_recv(char* buf, int16_t len, void* ctx, lsd_recv_cb recv_cb)
{
    if (disableChecks)
        return 0;
    check_expected(buf);
    check_expected(len);
    check_expected(ctx);
    check_expected(recv_cb);
    return mock_type(enum lsd_status);
}

enum lsd_status __wrap_lsd_send(
    uint8_t ch, const char* data, int16_t len, void* ctx, lsd_send_cb send_cb)
{
    if (disableChecks)
        return 0;
    check_expected(ch);
    check_expected(data);
    check_expected(len);
    check_expected(ctx);
    check_expected(send_cb);
    return mock_type(enum lsd_status);
}

void __wrap_SYS_doVBlankProcessEx(VBlankProcessTime processTime)
{
}

void __wrap_VDP_setTileMapXY(VDPPlane plane, u16 tile, u16 x, u16 y)
{
}

u16 __wrap_VDP_loadTileSet(const TileSet* tileset, u16 index, TransferMethod tm)
{
    return 0;
}

u16 __wrap_VDP_drawImageEx(
    VDPPlane plane, const Image* image, u16 basetile, u16 x, u16 y, u16 loadpal, bool dma)
{
    return 0;
}

void __wrap_JOY_update(void)
{
    function_called();
}

u16 __wrap_JOY_readJoypad(u16 joy)
{
    return mock_type(u16);
}

void __wrap_JOY_init(void)
{
}

void __wrap_TSK_userSet(VoidCallback* task)
{
}

int16_t __wrap_mw_def_ap_cfg_get(void)
{
    return mock_type(int16_t);
}

void __wrap_SND_PCM_startPlay(const u8* sample, const u32 len, const SoundPcmSampleRate rate,
    const SoundPanning pan, const u8 loop)
{
    check_expected(sample);
    check_expected(len);
    check_expected(rate);
    check_expected(pan);
    check_expected(loop);
}

void __wrap_SND_PCM_stopPlay(void)
{
    function_called();
}

void __wrap_Z80_loadDriver(const u16 driver, const bool waitReady)
{
    if (disableChecks)
        return;
    check_expected(driver);
    check_expected(waitReady);
}

void __wrap_Z80_requestBus(bool wait)
{
    if (disableChecks)
        return;
    check_expected(wait);
}

bool __wrap_Z80_getAndRequestBus(bool wait)
{
    if (disableChecks)
        return false;

    check_expected(wait);

    return mock_type(bool);
}

void __wrap_Z80_releaseBus()
{
    if (disableChecks)
        return;

    function_called();
}

bool __wrap_SYS_doVBlankProcess()
{
    return mock_type(bool);
}

s16 __wrap_fix16ToInt(fix16 value)
{
    return 0;
}

s32 __wrap_fix32ToInt(fix32 value)
{
    return 0;
}

fix16 __wrap_fix16Frac(fix16 value)
{
    return 0;
}

fix32 __wrap_fix32Frac(fix32 value)
{
    return 0;
}
