#pragma once
#include "genesis.h"
#include "applemidi.h"

midi_pkt_result rtpmidi_processRtpMidiPacket(char* buffer, u16 length);
u16 rtpmidi_packRtpMidiPacket(const u8* midiData, u16 midiDataLength, u8* buffer, u16 bufferLength);
void rtpmidi_resetSendState(void);
