#include "genesis.h"
#include <setjmp.h>
#include <cmocka.h>
#include "asserts.h"
#include "mocks/mock_ym2612.h"
#include "wraps.h"

#define INT_PTR(val) (&((int) { val }))

#define UNUSED __attribute__((unused))
