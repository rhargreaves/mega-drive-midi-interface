#include <comm.h>
#include <genesis.h>
#include <interface.h>
#include <main.h>
#include <synth.h>

static const u16 MAX_X = 40;
static const char HEADER[] = "Sega Mega Drive MIDI Interface";

static u8 maxFrames;

void vsync(void);

int main(void)
{
    maxFrames = SYS_isNTSC() ? 60 : 50;

    SYS_setVIntCallback(vsync);
    VDP_drawText(HEADER, (MAX_X - sizeof(HEADER)) / 2, 2);
    VDP_drawText(BUILD, (MAX_X - sizeof(BUILD)) / 2, 3);
    interface_init();
    while (TRUE) {
        interface_tick();
    }
}

void vsync(void)
{
    char* lastError = interface_lastError();
    if (lastError != NULL) {
        VDP_setTextPalette(PAL1);
        VDP_drawText(lastError, 1, 9);
        interface_clearError();
    }

    static u8 frame = 0;
    if (++frame % 20 == 0) {
        static char idleTimeText[50];
        u16 idle = comm_idleCount();
        u16 busy = comm_busyCount();
        u16 percent = (busy * 100) / (idle + busy);
        sprintf(idleTimeText, "Load %i %s     ", percent, "%");
        comm_resetCounts();
        VDP_setTextPalette(PAL1);
        VDP_drawText(idleTimeText, 1, 7);
    }
}
