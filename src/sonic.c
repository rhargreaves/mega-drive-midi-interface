#include "sonic.h"
#include "midi.h"
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
static u16 framesSinceBeat;
static s16 currentAnimation = ANIM_STAND;

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
        fix32ToInt(FIX32(0)), TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    SPR_setAnim(sprite, currentAnimation);
    SPR_update();
    VDP_setPaletteColors(
        (PAL3 * 16), sonic_sprite.palette->data, sonic_sprite.palette->length);
    SYS_enableInts();
}

static void switchAnimation(u8 animation)
{
    if (animation != currentAnimation) {
        animationFrame = 0;
        SPR_setAnimAndFrame(sprite, animation, animationFrame);
        SPR_update();
        currentAnimation = animation;
    }
}

static void incrementFrame(void)
{
    SPR_setFrame(sprite, animationFrame);
    SPR_update();
    if (++animationFrame == sonicAnimation[currentAnimation].loopEnd + 1) {
        animationFrame = sonicAnimation[currentAnimation].loopStart;
    }
}

static void midiBeat(void)
{
    static u8 sixteenth = 0;
    if (sixteenth++ == 2) {
        sixteenth = 0;
        if (framesSinceBeat <= 9) {
            switchAnimation(ANIM_ROLL);
        } else if (framesSinceBeat > 10 && framesSinceBeat <= 15) {
            switchAnimation(ANIM_RUN);
        } else if (framesSinceBeat > 16 && framesSinceBeat <= 75) {
            switchAnimation(ANIM_WALK);
        } else if (framesSinceBeat > 76) {
            switchAnimation(ANIM_STAND);
        }
        framesSinceBeat = 0;
    }
    incrementFrame();
}

void sonic_vsync(void)
{
    framesSinceBeat++;
    static u16 lastClock = 0;
    u16 clock = midi_timing()->clocks / 6;
    if (clock == 0) {
        switchAnimation(ANIM_STAND);
    } else if (clock != lastClock) {
        midiBeat();
        lastClock = clock;
    }
}
