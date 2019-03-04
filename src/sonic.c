#include "sonic.h"
#include <genesis.h>

#include "sprite.h"

static Sprite* sprite;

#define MIN_POSX FIX32(10)
#define MAX_POSX FIX32(400)
#define MAX_POSY FIX32(156)

void sonic_init(void)
{
    SYS_disableInts();
    SPR_init(16, 256, 256);
    sprite = SPR_addSprite(&sonic_sprite, fix32ToInt(FIX32(0)),
        fix32ToInt(FIX32(0)), TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    SPR_update();
    SYS_enableInts();
}
