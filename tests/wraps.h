#pragma once
#include <stdint.h>
#include <midi.h>
#include <sprite_eng.h>
#include <types.h>
#include <sys.h>
#include <ext/mw/megawifi.h>
#include "log.h"
#include "synth.h"
#include "scheduler.h"
#include <vdp_bg.h>

typedef enum mw_err mw_err;

extern bool __real_comm_read_ready(void);
extern void __real_comm_init(void);
extern void __real_comm_write(u8 data);
extern u8 __real_comm_read(void);
extern u16 __real_comm_idle_count(void);
extern u16 __real_comm_busy_count(void);
extern void __real_comm_reset_counts(void);
extern void __real_comm_megawifi_midiEmitCallback(u8 midiByte);

extern void __real_comm_demo_init(void);
extern u8 __real_comm_demo_read_ready(void);
extern u8 __real_comm_demo_read(void);
extern u8 __real_comm_demo_write_ready(void);
extern void __real_comm_demo_write(u8 data);
extern void __real_comm_demo_vsync(void);

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
void __wrap_synth_operatorFirstDecayRate(u8 channel, u8 op, u8 firstDecayRate);
void __wrap_synth_operatorSecondDecayRate(
    u8 channel, u8 op, u8 secondDecayRate);
void __wrap_synth_operatorSecondaryAmplitude(
    u8 channel, u8 op, u8 secondaryAmplitude);
void __wrap_synth_operatorAmplitudeModulation(
    u8 channel, u8 op, u8 amplitudeModulation);
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

bool __wrap_comm_read_ready(void);
u8 __wrap_comm_read(void);
void __wrap_comm_write(u8 data);
void __wrap_comm_megawifi_init(void);
void __wrap_fm_writeReg(u16 part, u8 reg, u8 data);
void __wrap_psg_note_on(u8 channel, u16 freq);
void __wrap_psg_note_off(u8 channel);
void __wrap_psg_attenuation(u8 channel, u8 attenuation);
void __wrap_midi_note_off(u8 chan, u8 pitch);
void __wrap_midi_note_on(u8 chan, u8 pitch, u8 velocity);
void __wrap_midi_channel_volume(u8 chan, u8 volume);
void __wrap_midi_pitch_bend(u8 chan, u16 bend);
void __wrap_midi_setPolyphonic(bool state);
bool __wrap_midi_getPolyphonic(void);
void __wrap_midi_cc(u8 chan, u8 controller, u8 value);
void __wrap_midi_program(u8 chan, u8 program);
void __wrap_midi_sysex(u8* data, u16 length);
bool __wrap_midi_dynamic_mode(void);
DeviceChannel* __wrap_midi_channel_mappings(void);
void __wrap_midi_psg_tick(void);
void __wrap_midi_psg_load_envelope(const u8* eef);
void __wrap_midi_reset(void);
void __wrap_ui_fm_set_parameters_visibility(u8 chan, bool show);
void __wrap_ui_update(void);
void __wrap_YM2612_writeReg(const u16 part, const u8 reg, const u8 data);
void __wrap_VDP_drawText(const char* str, u16 x, u16 y);
void __wrap_SYS_setVIntCallback(VoidCallback* CB);
void __wrap_VDP_setTextPalette(u16 palette);
void __wrap_VDP_clearText(u16 x, u16 y, u16 w);
void __wrap_PSG_setEnvelope(u8 channel, u8 value);
void __wrap_PSG_setTone(u8 channel, u16 value);
Sprite* __wrap_SPR_addSprite(
    const SpriteDefinition* spriteDef, s16 x, s16 y, u16 attribut);
void __wrap_SPR_update();
void __wrap_SYS_disableInts();
void __wrap_SYS_enableInts();
void __wrap_SPR_init(u16 maxSprite, u16 vramSize, u16 unpackBufferSize);
void __wrap_VDP_setPaletteColors(u16 index, const u16* values, u16 count);
void __wrap_VDP_setPaletteColor(u16 index, u16 value);
void __wrap_VDP_setBackgroundColor(u8 index);
void __wrap_SPR_setAnim(Sprite* sprite, s16 anim);
void __wrap_SPR_setFrame(Sprite* sprite, s16 frame);
void __wrap_SPR_setAnimAndFrame(Sprite* sprite, s16 anim, s16 frame);
void __wrap_SPR_setVisibility(Sprite* sprite, SpriteVisibility value);
void __wrap_VDP_setReg(u16 reg, u8 value);
u8 __wrap_VDP_getReg(u16 reg);
void __wrap_SYS_setExtIntCallback(VoidCallback* CB);
void __wrap_SYS_setInterruptMaskLevel(u16 value);
void __wrap_comm_init(void);
void __wrap_comm_serial_init(void);
u8 __wrap_comm_serial_read_ready(void);
u8 __wrap_comm_serial_read(void);
u8 __wrap_comm_serial_write_ready(void);
void __wrap_comm_serial_write(u8 data);

void __wrap_comm_everdrive_init(void);
u8 __wrap_comm_everdrive_read_ready(void);
u8 __wrap_comm_everdrive_read(void);
u8 __wrap_comm_everdrive_write_ready(void);
void __wrap_comm_everdrive_write(u8 data);

void __wrap_comm_everdrive_pro_init(void);
u8 __wrap_comm_everdrive_pro_read_ready(void);
u8 __wrap_comm_everdrive_pro_read(void);
u8 __wrap_comm_everdrive_pro_write_ready(void);
void __wrap_comm_everdrive_pro_write(u8 data);

void __wrap_comm_demo_init(void);
u8 __wrap_comm_demo_read_ready(void);
u8 __wrap_comm_demo_read(void);
u8 __wrap_comm_demo_write_ready(void);
void __wrap_comm_demo_write(u8 data);
void __wrap_comm_demo_vsync(void);

u16 __wrap_SYS_getCPULoad();
u32 __wrap_getFPS();
void __wrap_log_init(void);
void __wrap_log_info(const char* fmt, ...);
void __wrap_log_warn(const char* fmt, ...);
Log* __wrap_log_dequeue(void);
void __wrap_VDP_clearTextArea(u16 x, u16 y, u16 w, u16 h);
bool __wrap_region_isPal(void);
void wraps_region_setIsPal(bool isPal);

void __wrap_comm_megawifi_midiEmitCallback(u8 midiByte);
mw_err __wrap_mediator_recv_event(void);
mw_err __wrap_mediator_send_packet(u8 ch, char* data, u16 len);
void __wrap_SYS_die(char* err);

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
mw_err __wrap_mw_udp_set(uint8_t ch, const char* dst_addr, const char* dst_port,
    const char* src_port);
mw_err __wrap_mw_sock_conn_wait(uint8_t ch, int tout_frames);

void __wrap_midi_receiver_read_if_comm_ready(void);
void __wrap_scheduler_tick(void);
void __wrap_scheduler_addTickHandler(HandlerFunc* onTick);
void __wrap_scheduler_addFrameHandler(HandlerFunc* onFrame);

void __wrap_comm_megawifi_tick(void);
void __wrap_comm_megawifi_send(u8 ch, char* data, u16 len);

enum lsd_status __wrap_lsd_recv(
    char* buf, int16_t len, void* ctx, lsd_recv_cb recv_cb);
enum lsd_status __wrap_lsd_send(
    uint8_t ch, const char* data, int16_t len, void* ctx, lsd_send_cb send_cb);

void __wrap_Z80_requestBus(bool wait);
void __wrap_SYS_doVBlankProcessEx(VBlankProcessTime processTime);

void __wrap_VDP_setTileMapXY(VDPPlane plane, u16 tile, u16 x, u16 y);
u16 __wrap_VDP_loadTileSet(
    const TileSet* tileset, u16 index, TransferMethod tm);
u16 __wrap_VDP_drawImageEx(VDPPlane plane, const Image* image, u16 basetile,
    u16 x, u16 y, u16 loadpal, bool dma);

void __wrap_JOY_update(void);
u16 __wrap_JOY_readJoypad(u16 joy);
void __wrap_JOY_init(void);

void __wrap_TSK_userSet(VoidCallback* task);

int16_t __wrap_mw_def_ap_cfg_get(void);
