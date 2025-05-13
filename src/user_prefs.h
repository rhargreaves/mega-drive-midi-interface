#pragma once
// clang-format off

/*
setting : default mapping mode
values  : MappingMode_Auto, MappingMode_Static, MappingMode_Dynamic
default : MappingMode_Auto
*/
#define DEFAULT_MAPPING_MODE MappingMode_Auto

/*
setting  : startup MIDI sequence
values   : any valid sequence of MIDI commands (e.g. note on: {0x90, 47, 0x7F})
default  : empty sequence ({ })
examples :
// 1. do nothing (default)
{ }

// 2. route MIDI channel 10 to FM channel 6
{ \
    0xF0, 0x00, 0x22, 0x77, 0x00, 0x09, 0x05, 0xF7, \
    0xF0, 0x00, 0x22, 0x77, 0x00, 0x7F, 0x09, 0xF7  \
}
*/
#define STARTUP_MIDI_SEQUENCE { }
