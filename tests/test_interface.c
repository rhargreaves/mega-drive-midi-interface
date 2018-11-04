#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <interface.h>
#include <midi.h>
#include <types.h>
#include <wraps.h>

static void interface_tick_passes_message_to_midi_processor(void** state)
{
    u8 expectedStatus = 1;
    u8 expectedData = 2;
    u8 expectedData2 = 3;
    Message expectedMessage = { expectedStatus, expectedData, expectedData2 };

    will_return(__wrap_comm_read, expectedStatus);
    will_return(__wrap_comm_read, expectedData);
    will_return(__wrap_comm_read, expectedData2);

    expect_memory(__wrap_midi_process, message, &expectedMessage, sizeof(Message));

    interface_tick();
}

static void interface_initialises_synth(void** state)
{
    expect_function_call(__wrap_synth_init);

    interface_init();
}
