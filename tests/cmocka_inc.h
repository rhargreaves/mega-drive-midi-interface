#include "genesis.h"
#include <setjmp.h>
#include <cmocka.h>

#define INT_PTR(val) (&((int) { val }))

#define UNUSED __attribute__((unused))

#define expect_value_with_pos(function, parameter, value, file, line)                              \
    _expect_value(#function, #parameter, file, line, cast_to_largest_integral_type(value), 1)
#define expect_any_with_pos(function, parameter, file, line)                                       \
    _expect_any(#function, #parameter, file, line, 1)
#define will_return_with_pos(function, value, file, line)                                          \
    _will_return(#function, file, line, cast_to_largest_integral_type(value), 1)
