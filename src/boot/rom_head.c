#include "genesis.h"

// clang-format off
__attribute__((externally_visible)) const ROMHeader rom_header = {
#if MEGAWIFI
    .console        = "SEGA MEGAWIFI   ",
#elif (ENABLE_BANK_SWITCH != 0)
    .console        = "SEGA SSF        ",
#else
    .console        = "SEGA MEGA DRIVE ",
#endif
    .copyright      = "(C)ROB H. GPLv3 ",
    .title_local    = "MEGA DRIVE MIDI INTERFACE                       ",
    .title_int      = "MEGA DRIVE MIDI INTERFACE                       ",
    .serial         = "GM 00000000-00",
    .checksum       = 0x000,
    .IOSupport      = "JD              ",
    .rom_start      = 0x00000000,
    .rom_end        = 0x003FFFFF,
    .ram_start      = 0xE0FF0000,
    .ram_end        = 0xE0FFFFFF,
    .sram_sig       = "RA",
    .sram_type      = 0xF820,
    .sram_start     = 0x00200000,
    .sram_end       = 0x0020FFFF,
    .modem_support  = "            ",
    .notes          = "                                        ",
    .region         = "JUE             "
};
