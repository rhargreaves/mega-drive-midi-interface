#include "genesis.h"
#include <setjmp.h>
#include <cmocka.h>
#include "asserts.h"
#include "wraps.h"

#define INT_PTR(val) (&((int) { val }))

#define UNUSED __attribute__((unused))
