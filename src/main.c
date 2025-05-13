#include "genesis.h"
#include "comm/comm.h"
#include "envelopes.h"
#include "log.h"
#include "midi.h"
#include "midi_receiver.h"
#include "presets.h"
#include "scheduler.h"
#include "ui.h"
#include "comm/comm_megawifi.h"
#include "comm/everdrive_led.h"
#include "comm/comm_demo.h"
#include "build_checks.h"

int main()
{
    log_init();
    scheduler_init();
    midi_init(M_BANK_0, P_BANK_0, ENVELOPES);
    ui_init();
    comm_init();
    everdrive_led_init();
    midi_receiver_init();
    midi_receiver_run_startup_sequence();
    SYS_setVIntCallback(scheduler_vsync);
    scheduler_run();
}
