#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "asserts.h"
#include "wraps.h"
#include <cmocka.h>

void stub_comm_read_returns_midi_event(u8 status, u8 data, u8 data2)
{
    will_return(__wrap_comm_read, status);
    will_return(__wrap_comm_read, data);
    will_return(__wrap_comm_read, data2);
}
