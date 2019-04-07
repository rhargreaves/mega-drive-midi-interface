#include "sonic.h"
#include "midi.h"
#include <genesis.h>

#include "sprite.h"

typedef struct SonicAnimation SonicAnimation;

struct SonicAnimation {
    u16 loopStart;
    u16 loopEnd;
    u16 speed;
};

typedef enum SonicState {
    STAND,
    WAIT,
    WALK,
    RUN,
    BRAKE,
    UP,
    CROUNCH,
    ROLL
} SonicState;

static SonicState currentState = STAND;

static const SonicAnimation sonicAnimation[8]
    = { { 0, 0, 50 }, { 1, 2, 25 }, { 0, 5, 10 }, { 0, 3, 10 }, { 0, 1, 25 },
          { 0, 0, 50 }, { 0, 0, 50 }, { 0, 4, 5 } };

static Sprite* sprite;
static s16 animationFrame;
static u16 framesSinceBeat;
static u16 framesSinceStanding;
static u16 framesSinceWait;
static u16 lastClock;
static u8 lastSixteenth;

static void switchState(SonicState state);
static void incrementFrame(void);
static void switchStateToMatchSpeed(void);
static void checkMidiClock(void);
static void switchToWaitIfIdle(void);
static void animateWaitState(void);

void sonic_init(const SpriteDefinition* sonicSprite)
{
    SYS_disableInts();
    SPR_init(16, 256, 256);
    sprite = SPR_addSprite(sonicSprite, fix32ToInt(FIX32(0)),
        fix32ToInt(FIX32(8)), TILE_ATTR(PAL3, TRUE, FALSE, FALSE));
    SPR_setAnim(sprite, (s16)currentState);
    SPR_update();
    VDP_setPaletteColors(
        (PAL3 * 16), sonicSprite->palette->data, sonicSprite->palette->length);
    SYS_enableInts();
}

void sonic_reset(void)
{
    currentState = STAND;
    animationFrame = 0;
    framesSinceBeat = 0;
    framesSinceStanding = 0;
    framesSinceWait = 0;
    lastClock = 0;
    lastSixteenth = 0;
}

void sonic_vsync(void)
{
    framesSinceBeat++;
    switch (currentState) {
    case STAND:
        switchToWaitIfIdle();
        break;
    case WAIT:
        animateWaitState();
        break;
    case ROLL:
    case WALK:
    case RUN:
        framesSinceStanding = 0;
        if (framesSinceBeat > 120) {
            switchState(STAND);
        }
        break;
    default:
        break;
    }
    checkMidiClock();
}

static void switchState(SonicState state)
{
    if (state != currentState) {
        animationFrame = 0;
        SPR_setAnimAndFrame(sprite, state, animationFrame);
        SPR_update();
        currentState = state;
    }
}

static void incrementFrame(void)
{
    SPR_setFrame(sprite, animationFrame);
    SPR_update();
    if (++animationFrame == sonicAnimation[currentState].loopEnd + 1) {
        animationFrame = sonicAnimation[currentState].loopStart;
    }
}

static void switchToWaitIfIdle(void)
{
    framesSinceStanding++;
    if (framesSinceStanding > (60 * 5)) {
        switchState(WAIT);
    }
}

static void animateWaitState(void)
{
    if (++framesSinceWait == 25) {
        framesSinceWait = 0;
        incrementFrame();
    }
}

static void checkMidiClock(void)
{
    u16 clock = midi_timing()->clocks / 6;
    if (clock != lastClock) {
        switchStateToMatchSpeed();
        incrementFrame();
        lastClock = clock;
    }
}

static void switchStateToMatchSpeed(void)
{
    if (lastSixteenth++ == 3) {
        lastSixteenth = 0;
        if (framesSinceBeat > 30) {
            switchState(WALK);
        } else if (framesSinceBeat > 22) {
            switchState(RUN);
        } else {
            switchState(ROLL);
        }
        framesSinceBeat = 0;
    }
}
