#include "settings.h"

bool settings_isMegaWiFiRom(void)
{
#if MEGAWIFI
    return true;
#else
    return false;
#endif
}

bool settings_debugLoad(void)
{
#if DEBUG_LOAD
    return true;
#else
    return false;
#endif
}

bool settings_debugSerial(void)
{
#if DEBUG_SERIAL
    return true;
#else
    return false;
#endif
}
