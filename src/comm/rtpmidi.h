#pragma once
#include "genesis.h"
#include "applemidi.h"

midi_pkt_result rtpmidi_processRtpMidiPacket(char* buffer, u16 length);
