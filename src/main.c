#include "comm/comm.h"
#include "comm/comm_demo.h"
#include "comm/comm_megawifi.h"
#include "comm/everdrive_led.h"
#include "envelopes.h"
#include "genesis.h"
#include "log.h"
#include "midi.h"
#include "midi_rx.h"
#include "presets.h"
#include "scheduler.h"
#include "ui.h"

#if (MODULE_MEGAWIFI == 0)
#error MegaWiFi module disabled. Build SGDK with MODULE_MEGAWIFI = 1
#endif

int main()
{
    log_init();
    scheduler_init();
    SYS_setVIntCallback(scheduler_vsync);
    midi_init(M_BANK_0, P_BANK_0, ENVELOPES);
    ui_init();
    comm_init();
    everdrive_led_init();
    midi_rx_init();
    midi_rx_run_startup_sequence();
    scheduler_run();
}
