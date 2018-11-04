#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>
#include <wraps.h>

void __wrap_synth_init(void)
{
    function_called();
}

void __wrap_synth_noteOn(u8 channel)
{
    check_expected(channel);
}

u8 __wrap_comm_read(void)
{
    return mock_type(u8);
}

void __wrap_midi_process(Message* message)
{
    check_expected(message);
}

void __wrap_fm_writeReg(u16 part, u8 reg, u8 data)
{
    check_expected(part);
    check_expected(reg);
    check_expected(data);
}
