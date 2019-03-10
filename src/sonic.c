#include "sonic.h"
#include <genesis.h>

#include "sprite.h"

#define ANIM_STAND 0
#define ANIM_WAIT 1
#define ANIM_WALK 2
#define ANIM_RUN 3
#define ANIM_BRAKE 4
#define ANIM_UP 5
#define ANIM_CROUNCH 6
#define ANIM_ROLL 7

static Sprite* sprite;
static s16 animationFrame;
static u16 vsyncFrame;
static int animation = ANIM_WAIT;

typedef struct SonicAnimation SonicAnimation;

struct SonicAnimation {
    u16 loopStart;
    u16 loopEnd;
    u16 speed;
};

static const SonicAnimation sonicAnimation[8]
    = { { 0, 0, 50 }, { 1, 2, 25 }, { 0, 5, 10 }, { 0, 3, 10 }, { 0, 1, 25 },
          { 0, 0, 50 }, { 0, 0, 50 }, { 0, 4, 5 } };

static void incrementFrame(void);

void sonic_init(void)
{
    SYS_disableInts();
    SPR_init(16, 256, 256);
    sprite = SPR_addSprite(&sonic_sprite, fix32ToInt(FIX32(0)),
        fix32ToInt(FIX32(0)), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_setAnim(sprite, animation);
    VDP_setPaletteColors(
        (PAL2 * 16), sonic_sprite.palette->data, sonic_sprite.palette->length);
    SPR_update();
    SYS_enableInts();
}

void sonic_vsync(void)
{
    if (++vsyncFrame != sonicAnimation[animation].speed) {
        return;
    }
    vsyncFrame = 0;
    incrementFrame();
}

static void incrementFrame(void)
{
    SPR_setFrame(sprite, animationFrame);
    SPR_update();
    if (++animationFrame == sonicAnimation[animation].loopEnd + 1) {
        animationFrame = sonicAnimation[animation].loopStart;
    }
}
