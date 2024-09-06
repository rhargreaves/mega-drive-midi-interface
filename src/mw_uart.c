#include "mw_uart.h"

bool mw_uart_is_present(void)
{
    const u8 random = 0x57;
    UART_SPR = random;
    return UART_SPR == random;
}
