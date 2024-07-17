#include "cmocka_inc.h"

#include "note_priority.h"

static NotePriorityStack testStack;

static int test_note_priority_setup(UNUSED void** state)
{
    note_priority_init(&testStack);
    return 0;
}

static void test_note_priority_ignores_push_when_full(UNUSED void** state)
{
    const u16 additive = 50;

    for (u16 i = 0; i < NOTE_PRIORITY_LENGTH + 1; i++) {
        // print_message("pushing %d\n", i + additive);
        note_priority_push(&testStack, i + additive);
    }

    for (s16 i = NOTE_PRIORITY_LENGTH - 1; i >= 0; i--) {
        u8 item = note_priority_pop(&testStack);
        u8 expected = i + additive;

        // print_message("expected %d, popping %d\n", expected, item);
        assert_int_equal(item, expected);
    }

    u8 nilPop = note_priority_pop(&testStack);
    assert_int_equal(nilPop, 0);
}

static void test_note_priority_indicates_when_full(UNUSED void** state)
{
    for (u16 i = 0; i < NOTE_PRIORITY_LENGTH; i++) {
        assert_int_equal(note_priority_isFull(&testStack), false);
        note_priority_push(&testStack, i);
    }

    assert_int_equal(note_priority_isFull(&testStack), true);
}
