#include "sonic.h"
#include <genesis.h>

#include "sprite.h"

static Sprite* sprite;
static s16 frame;

#define ANIM_STAND 0
#define ANIM_WAIT 1
#define ANIM_WALK 2
#define ANIM_RUN 3
#define ANIM_BRAKE 4
#define ANIM_UP 5
#define ANIM_CROUNCH 6
#define ANIM_ROLL 7

#define MIN_POSX FIX32(10)
#define MAX_POSX FIX32(400)
#define MAX_POSY FIX32(156)

void sonic_init(void)
{
    SYS_disableInts();
    SPR_init(16, 256, 256);
    sprite = SPR_addSprite(&sonic_sprite, fix32ToInt(FIX32(0)),
        fix32ToInt(FIX32(0)), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_setAnim(sprite, ANIM_WAIT);
    VDP_setPaletteColors(
        (PAL2 * 16), sonic_sprite.palette->data, sonic_sprite.palette->length);
    SPR_update();
    SYS_enableInts();
}

void sonic_vsync(void)
{
    if (frame == 0)
        frame = 1;
    else
        frame = 0;
    SPR_setFrame(sprite, frame);
    SPR_update();
}
