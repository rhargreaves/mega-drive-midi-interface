#pragma once
#include "applemidi.h"

enum mw_err rtpmidi_processRtpMidiPacket(
    char* buffer, u16 length, u16* lastSeqNum);
