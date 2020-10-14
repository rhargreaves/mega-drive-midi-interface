#pragma once
#include "mw/megawifi.h"

typedef enum mw_err mw_err;

mw_err mediator_receive(void);
void mediator_send(u8 ch, char* data, u16 len);
