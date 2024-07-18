#include "comm.h"
#include "envelopes.h"
#include "log.h"
#include "midi.h"
#include "midi_receiver.h"
#include "presets.h"
#include "scheduler.h"
#include "sys.h"
#include "ui.h"
#include "comm_megawifi.h"
#include "everdrive_led.h"
#include "comm_demo.h"
#include "vdp.h"

int main()
{
    log_init();
    scheduler_init();
    midi_init(M_BANK_0, P_BANK_0, ENVELOPES);
    ui_init();
    comm_init();
    everdrive_led_init();
    midi_receiver_init();
    SYS_setVIntCallback(scheduler_vsync);
    scheduler_run();
}
