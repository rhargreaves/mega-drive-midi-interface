
#include "types.h"

void comm_serial_init(void);
u8 comm_serial_readReady(void);
u8 comm_serial_read(void);
u8 comm_serial_writeReady(void);
void comm_serial_write(u8 data);
