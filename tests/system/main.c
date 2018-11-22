#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

int main(void)
{
    const struct CMUnitTest tests[] = {};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
