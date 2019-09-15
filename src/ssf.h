#pragma once
#include "types.h"

u16 ssf_usb_rd_ready(void);
u8 ssf_usb_read(void);
u16 ssf_usb_wr_ready(void);
u8 ssf_usb_write(u8 data);
