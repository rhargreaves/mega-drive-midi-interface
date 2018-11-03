#include <genesis.h>
#include <main.h>

static const u16 MAX_X = 40;
static const char HEADER[] = "Sega Mega Drive MIDI Interface";

int main(void)
{
    while (TRUE) {
        VDP_showFPS(FALSE);
        VDP_drawText(HEADER, (MAX_X - sizeof(HEADER)) / 2, 2);
        VDP_drawText(BUILD, (MAX_X - sizeof(BUILD)) / 2, 3);
        VDP_waitVSync();
    }
}
