#include <stdint.h>
#include <types.h>

#include <memory.h>

#include <vstring.h>

#include "mpool.h"

#define MP_ALIGN_MASK (MP_ALIGN - 1)

/// BSS end symbol, defined in linker script. Pool grows from here to the
/// end of the RAM
extern uint8_t _eflash;

/// End of the memory POOL
#define MP_POOL_END ((void*)0x01000000)

/// Mask used for alignment computations
#define MP_ALIGN_MASK (MP_ALIGN - 1)

#define MP_ALIGN_COMP(addr)                                                    \
    (uint8_t*)((                                                               \
        (((uint32_t)(addr)) + MP_ALIGN_MASK) & (~((uint32_t)MP_ALIGN_MASK))))

typedef struct {
    uint8_t* floor;
    uint8_t* pos;
    uint8_t init_done;
} mp_data;

/// Local module data
mp_data md = { 0, 0, 0 };

void mp_init(int force_init)
{
    if (!md.init_done || force_init) {
        // Ensure the origin is aligned and initialize current position
        md.floor = MP_ALIGN_COMP(&_eflash);
        md.pos = md.floor;
        md.init_done = 1;
    }
}

void* mp_alloc(uint16_t length)
{
    void *tmp, *ret;

    // Adjust length depending on alignmentnforcement
    length = (length + MP_ALIGN_MASK) & ~MP_ALIGN_MASK;

    // Check there is enough room
    tmp = length + md.pos;
    if (tmp >= MP_POOL_END)
        return NULL;
    ret = md.pos;
    md.pos = tmp;

    return ret;
}

void* mp_calloc(uint16_t length)
{
    void* mem = mp_alloc(length);
    ;

    if (mem)
        memset(mem, 0, length);

    return mem;
}

void mp_free_to(void* pos)
{
    // Check pos looks valid, and set it if affirmative
    if ((pos >= (void*)md.floor) && (pos < (void*)md.pos)
        && (pos == MP_ALIGN_COMP(pos)))
        md.pos = pos;
}
