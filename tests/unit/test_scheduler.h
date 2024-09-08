#include "cmocka_inc.h"

int test_scheduler_setup(UNUSED void** state);
void test_scheduler_nothing_called_on_vsync(UNUSED void** state);
void test_scheduler_processes_frame_events_once_after_vsync(UNUSED void** state);
void test_scheduler_registered_frame_handler_called_on_vsync(UNUSED void** state);
void test_scheduler_multiple_registered_frame_handlers_called_on_vsync(UNUSED void** state);
void test_scheduler_registered_tick_handler_called(UNUSED void** state);
void test_scheduler_multiple_registered_tick_handlers_called(UNUSED void** state);
void test_scheduler_errors_if_too_many_frame_handlers_are_registered(UNUSED void** state);
void test_scheduler_errors_if_too_many_tick_handlers_are_registered(UNUSED void** state);
