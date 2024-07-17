#include "cmocka_inc.h"

#include "note_priority.h"

static NotePriorityStack testStack;

static int test_note_priority_setup(UNUSED void** state)
{
    note_priority_init(&testStack);
    return 0;
}

static void test_note_priority_evicts_old_items(UNUSED void** state)
{
    const u16 additive = 50;

    for (u16 i = 0; i <= 11; i++) {
        note_priority_push(&testStack, i + additive);
    }

    for (s16 i = 11; i > 1; i--) {
        u8 item = note_priority_pop(&testStack);
        u8 expected = i + additive;
        assert_int_equal(item, expected);
    }

    u8 nilPop = note_priority_pop(&testStack);
    assert_int_equal(nilPop, 0);
}
