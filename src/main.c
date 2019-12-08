#include "comm.h"
#include "midi.h"
#include "midi_receiver.h"
#include "presets.h"
#include "sys.h"
#include "ui.h"

int main()
{
    comm_init();
    midi_init(M_BANK_0, P_BANK_0);
    midi_receiver_init();
    ui_init();
    SYS_setVIntCallback(ui_vsync);
    midi_receiver_perpectual_read();
}
