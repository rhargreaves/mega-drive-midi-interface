
#include "sys_wraps.h"
#include <stdbool.h>

#ifndef UNIT_TESTS
extern void __real_SYS_disableInts(void);
extern void __real_SYS_enableInts(void);

void __wrap_SYS_disableInts(void)
{
}

void __wrap_SYS_enableInts(void)
{
}
#endif
