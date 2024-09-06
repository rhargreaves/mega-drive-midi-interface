#include "genesis.h"

// clang-format off
__attribute__((externally_visible)) const ROMHeader rom_header = {
#if MEGAWIFI
    "SEGA MEGAWIFI   ",
#else
    "SEGA SSF        ",
#endif
    "(C)ROB H. 2024  ",
    "MEGA DRIVE MIDI INTERFACE                       ",
    "MEGA DRIVE MIDI INTERFACE                       ",
    "GM 00000000-00",
    0x000,
    "JD              ",
    0x00000000,
#if (ENABLE_BANK_SWITCH != 0)
    0x003FFFFF,
#else
    0x000FFFFF,
#endif
    0xE0FF0000,
    0xE0FFFFFF,
    "RA",
    0xF820,
    0x00200000,
    0x0020FFFF,
    "            ",
    "                                        ",
    "JUE             "
};
