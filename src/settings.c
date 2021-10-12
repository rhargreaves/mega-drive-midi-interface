#include "settings.h"

bool settings_is_megawifi_rom(void)
{
#if MEGAWIFI
    return true;
#else
    return false;
#endif
}

bool settings_debug_load(void)
{
#if DEBUG_LOAD
    return true;
#else
    return false;
#endif
}

bool settings_debug_ticks(void)
{
#if DEBUG_TICKS
    return true;
#else
    return false;
#endif
}

bool settings_debug_serial(void)
{
#if DEBUG_SERIAL
    return true;
#else
    return false;
#endif
}

bool settings_debug_megawifi_init(void)
{
#if DEBUG_MEGAWIFI_INIT
    return true;
#else
    return false;
#endif
}
