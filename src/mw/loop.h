/****************************************************************************
 * \file
 *
 * \brief Loop handling for single threaded Megadrive programs.
 *
 * \defgroup loop loop
 * \{
 *
 * \brief Loop handling for single threaded Megadrive programs.
 *
 * Allows easily adding and removing functions to be run on the main loop, as
 * well as timers based on the frame counter. It also provides a hacky
 * interface to perform pseudo syncrhonous calls (through the loop_pend() and
 * loop_post() semantics) without disturbing the loop execution.
 *
 * \note Timers will take more frames than expected if loop load is high
 * enough to take more than a frame to complete.
 * \warning Due to the crappy/hacky implementation of the syncrhonous API, when
 * nesting loop_pend() calls, loop_post() will restore control flow reversing
 * the order of the loop_pend() calls. This is probably not what you want, and
 * it is thus discouraged to nest loop_pend() calls unless you know what you
 * are doing.
 ****************************************************************************/

#include <stdint.h>

#ifndef _LOOP_H_
#define _LOOP_H_

/// Frames per second (60 on NTSC consoles, 50 on PAL machines)
#define FPS 60

/// Converts milliseconds to frames, rounding to the nearest.
#define MS_TO_FRAMES(ms) (((ms)*FPS / 500 + 1) / 2)

struct loop_func;

/// Loop function callback definition
typedef void (*loop_func_cb)(struct loop_func* f);

/// Loop function data structure
struct loop_func {
    /// Function callback to run on the loop
    loop_func_cb func_cb; ///< Function callback to run on the loop
    struct {
        // Do not manually modify these fields
        uint16_t to_delete : 1; ///< Delete function when 1
        uint16_t blocked : 1; ///< Blocked on a loop_pend()
        uint16_t disabled : 1; ///< Function disabled when 1
    };
};

struct loop_timer;

/// Loop timer callback definition
typedef void (*loop_timer_cb)(struct loop_timer* t);

/// Loop timer data structure
struct loop_timer {
    loop_timer_cb timer_cb; ///< Timer callback function
    uint16_t frames; ///< Timer duration in frames
    uint16_t count; ///< Timer counter (do not manually modify)
    struct {
        uint16_t auto_reload : 1; ///< Set for timer auto-reload
        /// Delete timer when 1 (do not manually modify)
        uint16_t to_delete : 1;
        uint16_t blocked : 1; ///< Blocked on a loop_pend()
    };
};

/************************************************************************/ /**
 * \brief Initialize loop
 *
 * \param[in] max\_func  Maximum number of loop functions to support.
 * \param[in] max\_timer Maximum number of loop timers to support.
 *
 * \return 0 on success, 1 on error.
 ****************************************************************************/
int loop_init(uint8_t max_func, uint8_t max_timer);

/************************************************************************/ /**
 * \brief Add a function to the loop.
 *
 * \param[in] func Pointer to the function data structure to add.
 *
 * \return 0 on success, 1 if maximun number of functions has been reached.
 *
 * \note Function will enabled and added to the end of the function list.
 ****************************************************************************/
int loop_func_add(struct loop_func* func);

/************************************************************************/ /**
 * \brief Delete a function from the loop.
 *
 * \param[in] func Pointer to the function data structure to delete.
 *
 * \return 0 on success, 1 if requested timer function was not found.
 ****************************************************************************/
int loop_func_del(struct loop_func* func);

/************************************************************************/ /**
 * \brief Prevent a previously added function from running.
 *
 * \param[in] func Pointer to the function to disable.
 ****************************************************************************/
static inline void loop_func_disable(struct loop_func* func)
{
    func->disabled = 1;
}

/************************************************************************/ /**
 * \brief Resume execution of a previously disabled function.
 *
 * \param[in] func Pointer to the function to re-enable.
 ****************************************************************************/
static inline void loop_func_enable(struct loop_func* func)
{
    func->disabled = 0;
}

/************************************************************************/ /**
 * \brief Add a timer to the loop.
 *
 * \param[in] timer Pointer to the timer data structure to add.
 *
 * \return 0 on success, 1 if maximun number of timers has been reached.
 ****************************************************************************/
int loop_timer_add(struct loop_timer* timer);

/************************************************************************/ /**
 * \brief Start a previously added timer.
 *
 * \param[in] timer  Pointer to the previously added timer to start.
 * \param[in] frames Number of frames after the timer will trigger.
 ****************************************************************************/
static inline void loop_timer_start(struct loop_timer* timer, int frames)
{
    timer->frames = frames;
    timer->count = 0;
}

/************************************************************************/ /**
 * \brief Stop a previously added timer.
 *
 * \param[in] timer Pointer to the previously added timer to stop.
 ****************************************************************************/
static inline void loop_timer_stop(struct loop_timer* timer)
{
    timer->frames = 0;
}

/************************************************************************/ /**
 * \brief Delete a timer from the loop.
 *
 * \param[in] timer Pointer to the timer data structure to delete.
 *
 * \return 0 on success, 1 if requested timer to delete was not found.
 * \warning Loop deletion is delayed a bit (in the worst case, until the next
 * frame is processed by the loop). Thus do not deallocate the timer structure
 * immediately when this function returns.
 ****************************************************************************/
int loop_timer_del(struct loop_timer* timer);

/************************************************************************/ /**
 * \brief Loop function.
 *
 * Once called, this function calls the added functions and timers, and does
 * not return unless loop_end() is called.
 *
 * \return Value specified in the loop_end() call.
 ****************************************************************************/
int loop(void);

/************************************************************************/ /**
 * \brief Exit a previously entered loop
 *
 * \param[in] return_value Value to be returned by the loop() call.
 *                         It must be nonzero.
 ****************************************************************************/
void loop_end(int return_value);

/************************************************************************/ /**
 * \brief De-initialize loop module, and free associated resources.
 *
 * \warning Memory allocated using MpAlloc() after a loop_init(), will be
 * freed when calling this function.
 * \warning Do not call this function from inside the loop. Call loop_end()
 * to exit the loop, and once loop() returns, call loop_deinit().
 ****************************************************************************/
void loop_deinit(void);

/************************************************************************/ /**
 * \brief This function does not return until a loop_post() is executed.
 *
 * While in this function, the loop continues to run, and other functions and
 * timers are normally run. Use with care, specially if you are low on stack.
 *
 * \return A non-zero value, passed to the loop_post() function causing this
 * function to return, or zero on error.
 *
 * \warning Do not call this function if there is already a previous call to
 * loop_pend() that has not yet returned.
 ****************************************************************************/
int loop_pend(void);

/************************************************************************/ /**
 * \brief Causes a previously invoked loop_pend() function to return.
 *
 * \param[in] return_value Number to be returned by loop_pend(). Must be
 *            non-zero, or otherwise loop_pend() will not return.
 *
 * \warning Do not call this function if there is not a pending loop_pend().
 * \warning Passing zero to return_value variable, will prevent loop_pend()
 * to return.
 ****************************************************************************/
void loop_post(int return_value);

#endif /*_LOOP_H_*/

/** \} */
