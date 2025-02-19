#pragma once
#include "genesis.h"
#include "midi.h"
#include "log.h"
#include "synth.h"
#include "scheduler.h"

void wraps_disable_checks(void);
void wraps_enable_checks(void);

bool __wrap_mw_uart_is_present(void);

void __wrap_midi_receiver_read_if_comm_ready(void);

void __wrap_ui_fm_set_parameters_visibility(u8 chan, bool show);
void __wrap_ui_update(void);

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
