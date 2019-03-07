#include "sonic.h"
#include <genesis.h>

#include "sprite.h"

static Sprite* sprite;
static s16 animationFrame;
static u16 frame;

#define ANIM_STAND 0
#define ANIM_WAIT 1
#define ANIM_WALK 2
#define ANIM_RUN 3
#define ANIM_BRAKE 4
#define ANIM_UP 5
#define ANIM_CROUNCH 6
#define ANIM_ROLL 7

static const int LOOP_START[] = { 0, 1, 0, 0, 0, 0, 0, 0 };
static const int LOOP_END[] = { 0, 2, 5, 3, 1, 0, 0, 4 };
static const int SPEEDS[] = { 50, 25, 10, 10, 25, 50, 50, 5 };

static int animation = ANIM_WAIT;

#define MIN_POSX FIX32(10)
#define MAX_POSX FIX32(400)
#define MAX_POSY FIX32(156)

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
    if (++frame != SPEEDS[animation]) {
        return;
    }
    frame = 0;
    incrementFrame();
}

static void incrementFrame(void)
{
    SPR_setFrame(sprite, animationFrame);
    SPR_update();
    if (++animationFrame == LOOP_END[animation] + 1) {
        animationFrame = LOOP_START[animation];
    }
}
