#include "comm.h"
#include "midi.h"
#include "midi_receiver.h"
#include "presets.h"
#include "sys.h"
#include "ui.h"

static void initAllModules(void);
static void vsync(void);

int main(void)
{
    initAllModules();
    SYS_setVIntCallback(vsync);
    midi_receiver_perpectual_read();
}

static void initAllModules(void)
{
    comm_init();
    ui_init();
    midi_init((Channel**)M_BANK_0, (PercussionPreset**)P_BANK_0);
    midi_receiver_init();
}

static void vsync(void)
{
    ui_vsync();
}
