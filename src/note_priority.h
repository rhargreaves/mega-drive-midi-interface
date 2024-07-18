#pragma once
#include "types.h"

#define NOTE_PRIORITY_LENGTH 10

typedef struct NotePriorityStack {
    u8 slot[NOTE_PRIORITY_LENGTH];
    s16 top;
} NotePriorityStack;

void note_priority_init(NotePriorityStack* notePriority);
void note_priority_push(NotePriorityStack* notePriority, u8 pitch);
u8 note_priority_pop(NotePriorityStack* notePriority);
void note_priority_remove(NotePriorityStack* notePriority, u8 pitch);
bool note_priority_isFull(NotePriorityStack* notePriority);
