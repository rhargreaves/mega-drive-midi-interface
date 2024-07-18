#include "note_priority.h"
#include "memory.h"

void note_priority_init(NotePriorityStack* notePriority)
{
    notePriority->top = -1;
    memset(notePriority->slot, 0, NOTE_PRIORITY_LENGTH);
}

// 0      1      2   ...
// Oldest -> Most Recent
void note_priority_push(NotePriorityStack* notePriority, u8 pitch)
{
    if (notePriority->top == NOTE_PRIORITY_LENGTH - 1) {
        return;
    }
    notePriority->slot[++notePriority->top] = pitch;
}

u8 note_priority_pop(NotePriorityStack* notePriority)
{
    if (notePriority->top == -1) {
        return 0;
    }

    return notePriority->slot[notePriority->top--];
}

void note_priority_remove(NotePriorityStack* notePriority, u8 pitch)
{
    s16 writeCursor = -1;
    for (u16 i = 0; i <= notePriority->top; i++) {
        u8 item = notePriority->slot[i];
        if (item == pitch) {
            continue;
        } else {
            notePriority->slot[++writeCursor] = item;
        }
    }
    notePriority->top = writeCursor;
}

bool note_priority_isFull(NotePriorityStack* notePriority)
{
    return notePriority->top == NOTE_PRIORITY_LENGTH - 1;
}
