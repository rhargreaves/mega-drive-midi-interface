
#pragma once
#include "genesis.h"
#include "settings.h"

const u8* __wrap_settings_startup_midi_sequence(size_t* length);
extern const u8* __real_settings_startup_midi_sequence(size_t* length);

void mock_settings_set_startup_midi_sequence(const u8* sequence, size_t length);