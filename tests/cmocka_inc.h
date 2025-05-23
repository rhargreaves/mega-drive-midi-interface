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
#define expect_memory_with_pos(function, parameter, memory, size, file, line)                      \
    _expect_memory(#function, #parameter, file, line, memory, size, 1)
#define expect_check_with_pos(function, parameter, check_function, check_data, file, line)         \
    _expect_check(#function, #parameter, file, line, check_function,                               \
        cast_to_largest_integral_type(check_data), NULL, 1)
