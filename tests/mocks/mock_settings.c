
#include "mock_settings.h"

static const u8* startup_midi_sequence = NULL;
static size_t startup_midi_sequence_length = 0;

void mock_settings_set_startup_midi_sequence(const u8* sequence, size_t length)
{
    startup_midi_sequence = sequence;
    startup_midi_sequence_length = length;
}

const u8* __wrap_settings_startup_midi_sequence(size_t* length)
{
    *length = startup_midi_sequence_length;
    return startup_midi_sequence;
}
