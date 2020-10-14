#pragma once
#include "applemidi.h"

void midi_emit(u8 data);
mw_err rtpmidi_processRtpMidiPacket(char* buffer, u16 length);
