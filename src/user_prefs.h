#pragma once

// setting: default mapping mode
// values: MappingMode_Auto, MappingMode_Static, MappingMode_Dynamic
// default: MappingMode_Auto
#define DEFAULT_MAPPING_MODE MappingMode_Auto

// setting: startup MIDI sequence
// values: any valid sequence of MIDI commands (e.g. note on: {0x90, 47, 0x7F})
// default: empty sequence ({ })
// clang-format off
#define STARTUP_MIDI_SEQUENCE { }
// clang-format on
