#include "cmocka_inc.h"

int test_note_priority_setup(UNUSED void** state);
void test_note_priority_ignores_push_when_full(UNUSED void** state);
void test_note_priority_indicates_when_full(UNUSED void** state);
void test_note_priority_returns_size(UNUSED void** state);
void test_note_priority_pop_returns_0_when_empty(UNUSED void** state);
