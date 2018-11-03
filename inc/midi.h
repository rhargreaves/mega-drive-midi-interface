#pragma once
#include <types.h>

typedef struct Message Message;

struct Message {
    u8 status;
    u8 data;
    u8 data2;
};

void midi_process(Message* message);
