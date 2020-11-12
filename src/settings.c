#include "settings.h"

bool settings_isMegaWiFiRom(void)
{
#if MEGAWIFI
    return true;
#else
    return false;
#endif
}
