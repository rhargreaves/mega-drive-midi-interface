
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "sonic.h"
#include <types.h>
#include <sprite_eng.h>
#include <cmocka.h>

static void test_sonic_init(void** state)
{
    Sprite dummySprite = { };
    Palette dummyPalette = { };
    SpriteDefinition dummySpriteDef = {
        .palette = &dummyPalette
    };

    will_return(__wrap_SPR_addSprite, &dummySprite);

    sonic_init(dummySpriteDef);
}

static void test_sonic_single_vsync_does_nothing(void** state)
{
    sonic_vsync();
}
