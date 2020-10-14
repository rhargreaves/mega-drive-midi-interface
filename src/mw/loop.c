/// \todo
/// * Frame number masking for triggering timers on different frames
///   (e.g. on even or odd frames).

#include <stdint.h>
#include <types.h>

#include <setjmp.h>

#include <memory.h>

#include "loop.h"
#include "mpool.h"
#include <setjmp.h>
#include <string.h>

/// VDP Control port address
#define VDP_CTRL_PORT_ADDR 0xC00004
/// VDP control port, WORD access.
#define VDP_CTRL_PORT_W (*((volatile uint16_t*)VDP_CTRL_PORT_ADDR))
/// Flag of the status register corresponging to the VBLANK interval.
#define VDP_STAT_VBLANK 0x0008

enum loop_check { LOOP_CHECK_FUNCS = 0, LOOP_CHECK_TIMERS };

struct pend_env {
    struct loop_func* f;
    struct loop_timer* t;
};

struct loop_data {
    struct loop_func** f;
    struct loop_timer** t;
    struct pend_env* env;
    jmp_buf* jmp;
    enum loop_check check;
    int8_t func_max;
    int8_t timer_max;
    int8_t idx;
    int8_t vblank;
    uint16_t frame;
    int exit;
};

static struct loop_data* d = NULL;

int loop_init(uint8_t max_func, uint8_t max_timer)
{
    if (d) {
        // Already initialized
        return 0;
    }

    mp_init(0);

    d = mp_alloc(sizeof(struct loop_data));
    memset(d, 0, sizeof(struct loop_data));
    // Allocate pointers for the func/timer structures
    d->f = mp_alloc(max_func * sizeof(struct loop_func*));
    d->t = mp_alloc(max_timer * sizeof(struct loop_timer*));
    memset(d->f, 0, max_func * sizeof(struct loop_func*));
    memset(d->t, 0, max_timer * sizeof(struct loop_timer*));
    d->func_max = max_func;
    d->timer_max = max_timer;
    d->vblank = VDP_CTRL_PORT_W & VDP_STAT_VBLANK;

    return 0;
}

int loop_func_add(struct loop_func* func)
{
    int i;

    for (i = 0; i < d->func_max && d->f[i]; i++) {
        // If function was marked to be deleted, just remove the flag
        if (d->f[i] == func && func->to_delete) {
            func->to_delete = 0;
            return 0;
        }
    }
    if (i == d->func_max) {
        return 1;
    }

    d->f[i] = func;
    return 0;
}

int loop_func_del(struct loop_func* func)
{
    struct loop_func* f;
    int i;

    for (i = 0; (f = d->f[i]); i++) {
        if (f == func) {
            // Mark function to be deleted during loop
            f->to_delete = 1;
            return 0;
        }
    }
    return 1;
}

int loop_timer_add(struct loop_timer* timer)
{
    int i;

    for (i = 0; i < d->timer_max && d->t[i]; i++) {
        // If timer was marked to be deleted, just remove the flag
        if (d->t[i] == timer && timer->to_delete) {
            timer->to_delete = 0;
            return 0;
        }
    }
    if (i == d->timer_max) {
        return 1;
    }

    d->t[i] = timer;
    return 0;
}

int loop_timer_del(struct loop_timer* timer)
{
    struct loop_timer* t;
    int i;

    for (i = 0; (t = d->t[i]); i++) {
        if (t == timer) {
            // Mark function to be deleted during loop
            t->to_delete = 1;
            return 0;
        }
    }
    return 1;
}

static int frame_update(void)
{
    int vblank = VDP_CTRL_PORT_W & VDP_STAT_VBLANK;
    int rc = 0;

    if (!d->vblank && vblank) {
        rc = 1;
        d->frame++;
    }
    d->vblank = vblank;
    return rc;
}

static void delete_func(int i)
{
    while (d->f[i + 1]) {
        d->f[i] = d->f[i + 1];
        i++;
    }
    d->f[i] = NULL;
}

static void run_funcs(void)
{
    struct loop_func* f;

    while (d->idx < d->func_max && (f = d->f[d->idx])) {
        if (f->to_delete) {
            f->to_delete = 0;
            delete_func(d->idx);
        } else {
            d->idx++;
            if (!f->disabled && !f->blocked) {
                f->func_cb(f);
                d->env->f = NULL;
            }
        }
    }
}

static void delete_timer(int i)
{
    while (d->t[i + 1]) {
        d->t[i] = d->t[i + 1];
        i++;
    }
    d->t[i] = NULL;
}

static void update_timer(struct loop_timer* t)
{
    if (t->frames && !t->blocked) {
        t->count++;
        if (t->count >= t->frames) {
            if (t->auto_reload) {
                t->count = 0;
            } else {
                t->frames = 0;
            }
            t->timer_cb(t);
            d->env->t = NULL;
        }
    }
}

static void check_timers(void)
{
    struct loop_timer* t;

    while (d->idx < d->timer_max && (t = d->t[d->idx])) {
        if (t->to_delete) {
            t->to_delete = 0;
            delete_timer(d->idx);
        } else {
            d->idx++;
            update_timer(t);
        }
    }
}

int loop(void)
{
    struct pend_env env = {};
    d->env = &env;

    while (!d->exit) {
        if (LOOP_CHECK_TIMERS == d->check || frame_update()) {
            d->check = LOOP_CHECK_TIMERS;
            check_timers();
            d->idx = 0;
            d->check = LOOP_CHECK_FUNCS;
        } else {
            run_funcs();
            d->idx = 0;
        }
    }

    return d->exit;
}

void loop_deinit(void)
{
    if (!d)
        return;

    mp_free_to(d);
    d = NULL;
}

static int jmp_set(void)
{
    jmp_buf jmp;
    int returned;
    struct pend_env* prev_env = d->env;
    jmp_buf* prev_jmp = d->jmp;

    d->jmp = &jmp;

    returned = setjmp(jmp);
    if (!returned) {
        loop();
    }
    d->env = prev_env;
    d->jmp = prev_jmp;

    return returned;
}

int loop_pend(void)
{
    int returned = 0;

    if (!d->env->f && !d->env->t) {
        if (LOOP_CHECK_FUNCS == d->check) {
            d->env->f = d->f[d->idx - 1];
        } else {
            d->env->t = d->t[d->idx - 1];
        }
    }
    if (d->env->f) {
        d->env->f->blocked = 1;
        returned = jmp_set();
        d->env->f->blocked = 0;
    } else if (d->env->t) {
        d->env->t->blocked = 1;
        returned = jmp_set();
        d->env->t->blocked = 0;
    }

    return returned;
}

void loop_post(int return_value)
{
    if (d->env) {
        longjmp(*d->jmp, return_value);
    }
}

void loop_end(int return_value)
{
    d->exit = return_value;
}
