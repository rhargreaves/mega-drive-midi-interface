#pragma once
#include "genesis.h"
#include "midi.h"
#include "log.h"
#include "synth.h"
#include "scheduler.h"

void wraps_disable_checks(void);
void wraps_enable_checks(void);
void wraps_disable_logging_checks(void);
void wraps_enable_logging_checks(void);

void __wrap_synth_enableLfo(u8 enable);
void __wrap_synth_globalLfoFrequency(u8 freq);
void __wrap_synth_noteOn(u8 channel);
void __wrap_synth_noteOff(u8 channel);
void __wrap_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
void __wrap_synth_totalLevel(u8 channel, u8 totalLevel);
void __wrap_synth_stereo(u8 channel, u8 mode);
void __wrap_synth_algorithm(u8 channel, u8 algorithm);
void __wrap_synth_feedback(u8 channel, u8 feedback);
void __wrap_synth_ams(u8 channel, u8 ams);
void __wrap_synth_fms(u8 channel, u8 fms);
void __wrap_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
void __wrap_synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
void __wrap_synth_operatorDetune(u8 channel, u8 op, u8 detune);
void __wrap_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
void __wrap_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
void __wrap_synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate);
void __wrap_synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate);
void __wrap_synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel);
void __wrap_synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation);
void __wrap_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate);
void __wrap_synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg);
void __wrap_synth_preset(u8 channel, const FmChannel* preset);
void __wrap_synth_volume(u8 channel, u8 volume);
const FmChannel* __wrap_synth_channelParameters(u8 channel);
const Global* __wrap_synth_globalParameters();
void __wrap_synth_setSpecialMode(bool enable);
void __wrap_synth_specialModePitch(u8 op, u8 octave, u16 freqNumber);
void __wrap_synth_specialModeVolume(u8 op, u8 volume);
void __wrap_synth_directWriteYm2612(u8 part, u8 reg, u8 data);
void __wrap_synth_enableDac(bool enable);
extern void __real_synth_init(const FmChannel* defaultPreset);
extern void __real_synth_noteOn(u8 channel);
extern void __real_synth_noteOff(u8 channel);
extern void __real_synth_enableLfo(u8 enable);
extern void __real_synth_globalLfoFrequency(u8 freq);
extern void __real_synth_pitch(u8 channel, u8 octave, u16 freqNumber);
extern void __real_synth_stereo(u8 channel, u8 stereo);
extern void __real_synth_ams(u8 channel, u8 ams);
extern void __real_synth_fms(u8 channel, u8 fms);
extern void __real_synth_algorithm(u8 channel, u8 algorithm);
extern void __real_synth_feedback(u8 channel, u8 feedback);
extern void __real_synth_operatorTotalLevel(u8 channel, u8 op, u8 totalLevel);
extern void __real_synth_operatorMultiple(u8 channel, u8 op, u8 multiple);
extern void __real_synth_operatorDetune(u8 channel, u8 op, u8 detune);
extern void __real_synth_operatorRateScaling(u8 channel, u8 op, u8 rateScaling);
extern void __real_synth_operatorAttackRate(u8 channel, u8 op, u8 attackRate);
extern void __real_synth_operatorDecayRate(u8 channel, u8 op, u8 decayRate);
extern void __real_synth_operatorSustainRate(u8 channel, u8 op, u8 sustainRate);
extern void __real_synth_operatorSustainLevel(u8 channel, u8 op, u8 sustainLevel);
extern void __real_synth_operatorAmplitudeModulation(u8 channel, u8 op, u8 amplitudeModulation);
extern void __real_synth_operatorReleaseRate(u8 channel, u8 op, u8 releaseRate);
extern void __real_synth_operatorSsgEg(u8 channel, u8 op, u8 ssgEg);
extern void __real_synth_pitchBend(u8 channel, u16 bend);
extern void __real_synth_preset(u8 channel, const FmChannel* preset);
extern void __real_synth_volume(u8 channel, u8 volume);
extern const FmChannel* __real_synth_channelParameters(u8 channel);
extern const Global* __real_synth_globalParameters();
extern void __real_synth_setSpecialMode(bool enable);
extern void __real_synth_specialModePitch(u8 op, u8 octave, u16 freqNumber);
extern void __real_synth_specialModeVolume(u8 op, u8 volume);
extern void __real_synth_enableDac(bool enable);
extern void __real_synth_directWriteYm2612(u8 part, u8 reg, u8 data);

void __wrap_comm_init(void);
bool __wrap_comm_read_ready(void);
u8 __wrap_comm_read(void);
void __wrap_comm_write(u8 data);
void __wrap_comm_serial_init(void);
bool __wrap_comm_serial_is_present(void);
u8 __wrap_comm_serial_read_ready(void);
u8 __wrap_comm_serial_read(void);
u8 __wrap_comm_serial_write_ready(void);
void __wrap_comm_serial_write(u8 data);
void __wrap_comm_everdrive_init(void);
bool __wrap_comm_everdrive_is_present(void);
u8 __wrap_comm_everdrive_read_ready(void);
u8 __wrap_comm_everdrive_read(void);
u8 __wrap_comm_everdrive_write_ready(void);
void __wrap_comm_everdrive_write(u8 data);
void __wrap_comm_everdrive_pro_init(void);
bool __wrap_comm_everdrive_pro_is_present(void);
u8 __wrap_comm_everdrive_pro_read_ready(void);
u8 __wrap_comm_everdrive_pro_read(void);
u8 __wrap_comm_everdrive_pro_write_ready(void);
void __wrap_comm_everdrive_pro_write(u8 data);
void __wrap_comm_demo_init(void);
bool __wrap_comm_demo_is_present(void);
u8 __wrap_comm_demo_read_ready(void);
u8 __wrap_comm_demo_read(void);
u8 __wrap_comm_demo_write_ready(void);
void __wrap_comm_demo_write(u8 data);
void __wrap_comm_demo_vsync(void);
void __wrap_comm_megawifi_init(void);
void __wrap_comm_megawifi_midiEmitCallback(u8 midiByte);
void __wrap_comm_megawifi_tick(void);
void __wrap_comm_megawifi_send(u8 ch, char* data, u16 len);
bool __wrap_comm_megawifi_is_present(void);
u8 __wrap_comm_megawifi_read_ready(void);
u8 __wrap_comm_megawifi_read(void);
u8 __wrap_comm_megawifi_write_ready(void);
void __wrap_comm_megawifi_write(u8 data);
bool __wrap_mw_uart_is_present(void);

extern void __real_comm_megawifi_midiEmitCallback(u8 midiByte);
extern void __real_comm_megawifi_init(void);
extern bool __real_comm_read_ready(void);
extern void __real_comm_demo_init(void);
extern u8 __real_comm_demo_read_ready(void);
extern u8 __real_comm_demo_read(void);
extern u8 __real_comm_demo_write_ready(void);
extern void __real_comm_demo_write(u8 data);
extern void __real_comm_demo_vsync(void);
extern bool __real_comm_megawifi_is_present(void);
extern u8 __real_comm_megawifi_read_ready(void);
extern u8 __real_comm_megawifi_read(void);
extern u8 __real_comm_megawifi_write_ready(void);
extern void __real_comm_megawifi_write(u8 data);

extern void __real_comm_init(void);
extern void __real_comm_write(u8 data);
extern u8 __real_comm_read(void);
extern u16 __real_comm_idle_count(void);
extern u16 __real_comm_busy_count(void);
extern void __real_comm_reset_counts(void);

void __wrap_midi_receiver_read_if_comm_ready(void);

void __wrap_ui_fm_set_parameters_visibility(u8 chan, bool show);
void __wrap_ui_update(void);

void __wrap_log_init(void);
void __wrap_log_info(const char* fmt, ...);
void __wrap_log_warn(const char* fmt, ...);
Log* __wrap_log_dequeue(void);
extern void __real_log_init(void);
extern void __real_log_info(const char* fmt, ...);
extern void __real_log_warn(const char* fmt, ...);
extern Log* __real_log_dequeue(void);

void __wrap_scheduler_tick(void);
void __wrap_scheduler_addTickHandler(HandlerFunc* onTick);
void __wrap_scheduler_addFrameHandler(HandlerFunc* onFrame);
extern void __real_scheduler_init(void);
extern void __real_scheduler_tick(void);
extern void __real_scheduler_addTickHandler(HandlerFunc* onTick);
extern void __real_scheduler_addFrameHandler(HandlerFunc* onFrame);

/* SDGK wraps */

void __wrap_VDP_drawText(const char* str, u16 x, u16 y);
void __wrap_SYS_setVIntCallback(VoidCallback* CB);
void __wrap_VDP_setTextPalette(u16 palette);
void __wrap_VDP_clearText(u16 x, u16 y, u16 w);
void __wrap_PSG_setEnvelope(u8 channel, u8 value);
void __wrap_PSG_setTone(u8 channel, u16 value);
Sprite* __wrap_SPR_addSprite(const SpriteDefinition* spriteDef, s16 x, s16 y, u16 attribut);
void __wrap_SPR_update();
void __wrap_SYS_disableInts();
void __wrap_SYS_enableInts();
void __wrap_SPR_init(u16 maxSprite, u16 vramSize, u16 unpackBufferSize);
void __wrap_PAL_setColor(u16 index, u16 value);
void __wrap_PAL_setColors(u16 index, const u16* pal, u16 count, TransferMethod tm);
void __wrap_VDP_setBackgroundColor(u8 index);
void __wrap_SPR_setAnim(Sprite* sprite, s16 anim);
void __wrap_SPR_setFrame(Sprite* sprite, s16 frame);
void __wrap_SPR_setAnimAndFrame(Sprite* sprite, s16 anim, s16 frame);
void __wrap_SPR_setVisibility(Sprite* sprite, SpriteVisibility value);
void __wrap_VDP_setReg(u16 reg, u8 value);
u8 __wrap_VDP_getReg(u16 reg);
void __wrap_SYS_setExtIntCallback(VoidCallback* CB);
void __wrap_SYS_setInterruptMaskLevel(u16 value);
void __wrap_VDP_clearTextArea(u16 x, u16 y, u16 w, u16 h);
u16 __wrap_SYS_isPAL(void);
void wraps_set_SYS_isPAL(bool isPal);
void __wrap_SYS_die(char* err);
void __wrap_SYS_doVBlankProcessEx(VBlankProcessTime processTime);
void __wrap_VDP_setTileMapXY(VDPPlane plane, u16 tile, u16 x, u16 y);
u16 __wrap_VDP_loadTileSet(const TileSet* tileset, u16 index, TransferMethod tm);
u16 __wrap_VDP_drawImageEx(
    VDPPlane plane, const Image* image, u16 basetile, u16 x, u16 y, u16 loadpal, bool dma);
void __wrap_JOY_update(void);
u16 __wrap_JOY_readJoypad(u16 joy);
void __wrap_JOY_init(void);
void __wrap_TSK_userSet(VoidCallback* task);
void __wrap_SND_PCM_startPlay(const u8* sample, const u32 len, const SoundPcmSampleRate rate,
    const SoundPanning pan, const u8 loop);
void __wrap_SND_PCM_stopPlay(void);
void __wrap_Z80_loadDriver(const u16 driver, const bool waitReady);
void __wrap_Z80_requestBus(bool wait);
bool __wrap_Z80_getAndRequestBus(bool wait);
void __wrap_Z80_releaseBus();
bool __wrap_SYS_doVBlankProcess();
s16 __wrap_fix16ToInt(fix16 value);
s32 __wrap_fix32ToInt(fix32 value);
fix16 __wrap_fix16Frac(fix16 value);
fix32 __wrap_fix32Frac(fix32 value);

/* MegaWiFi wraps */
typedef enum mw_err mw_err;

mw_err __wrap_mediator_recv_event(void);
mw_err __wrap_mediator_send_packet(u8 ch, char* data, u16 len);
int __wrap_mw_init(char* cmd_buf, uint16_t buf_len);
void __wrap_mw_process(void);
void __wrap_lsd_process(void);
mw_err __wrap_mw_detect(uint8_t* major, uint8_t* minor, char** variant);
void mock_mw_detect(u8 major, u8 minor);
int __wrap_loop_init(uint8_t max_func, uint8_t max_timer);
mw_err __wrap_mw_ap_assoc(uint8_t slot);
mw_err __wrap_mw_ap_assoc_wait(int tout_frames);
mw_err __wrap_mw_ip_current(struct mw_ip_cfg** ip);
void mock_ip_cfg(u32 ip_addr);
mw_err __wrap_mw_udp_set(
    uint8_t ch, const char* dst_addr, const char* dst_port, const char* src_port);
mw_err __wrap_mw_sock_conn_wait(uint8_t ch, int tout_frames);
enum lsd_status __wrap_lsd_recv(char* buf, int16_t len, void* ctx, lsd_recv_cb recv_cb);
enum lsd_status __wrap_lsd_send(
    uint8_t ch, const char* data, int16_t len, void* ctx, lsd_send_cb send_cb);
int16_t __wrap_mw_def_ap_cfg_get(void);
