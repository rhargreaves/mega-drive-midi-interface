#include "cmocka_inc.h"

#include "vstring.h"

static void test_vstring_handles_variable_argument_list_correctly(UNUSED void** state)
{
    char buffer[10];

    v_sprintf(buffer, "%d %d", 1, 2);

    assert_memory_equal(buffer, "1 2", 4);
}
