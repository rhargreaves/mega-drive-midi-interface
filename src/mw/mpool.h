/************************************************************************//**
 * \file
 *
 * \brief Memory Pool
 *
 * \defgroup mpool mpool
 * \{
 *
 * \brief Memory Pool
 *
 * Implements a memory pool for dynamic memory allocation. This memory pool
 * gets memory from the unused region between the end of the .bss section
 * and the stack top. The implementation is pretty simple: an internal
 * pointer grows when memory is requested, and is reset to the specified
 * position to free memory. This restricts the usage of the module to
 * scenarios that free memory in exactly the reverse order in which they
 * requested it (it does not allow generic allocate/free such as malloc()
 * does).
 *
 * \author doragasu
 * \date   2017
 ****************************************************************************/


#ifndef _MPOOL_H_
#define _MPOOL_H_

#include <stdint.h>

/// Memory alignment enforcement (in bytes). Must be a power of 2
#define MP_ALIGN			4

/************************************************************************//**
 * \brief Pool initialization.
 *
 * Call this function before using any other in this module
 *
 * \param[in] force_init Force initialization (and memory reset) even if the
 *            module was previously initialized.
 ****************************************************************************/
void mp_init(int force_init);

/************************************************************************//**
 * \brief Allocates data from the pool.
 *
 * Obtains a memory chunck with enforced alignment and requested length.
 * Obtained chunck is contiguous.
 *
 * \param[in] length Length of the contiguous section to allocate.
 *
 * \return Pointer to the allocated memory zone of the requested length, or
 * NULL if the allocation could not succeed.
 *
 * \todo Maybe allocation should be tested against the stack pointer
 ****************************************************************************/
void *mp_alloc(uint16_t length) __attribute__((malloc));

/************************************************************************//**
 * \brief Allocates and zero fills data from the pool.
 *
 * Obtains a memory chunck with enforced alignment and requested length.
 * Obtained chunck is contiguous and initialized to zeros.
 *
 * \param[in] length Length of the contiguous section to allocate.
 *
 * \return Pointer to the allocated memory zone of the requested length, or
 * NULL if the allocation could not succeed.
 *
 * \todo Maybe allocation should be tested against the stack pointer
 ****************************************************************************/
void *mp_calloc(uint16_t length) __attribute__((malloc));

/************************************************************************//**
 * \brief Free memory up to the one pointed by pos.
 *
 * Memory previously allocated is freed up to pos position. The usual (and
 * recommended) way of using this function is calling it with pos set to the
 * value of the last mp_alloc() call.
 *
 * \warning This function will free memory requested by several mp_alloc()
 * calls if the input pos pointer is not the last returned by mp_alloc(). E.g.
 * if mp_alloc() is called consecutively three times, and mp_free_to() is called
 * with the pointer returned by the second call, the memory granted by the
 * second and third calls to mp_alloc() will be deallocated. Although this is
 * usually undesired, sometimes it is useful. In any case it is important
 * taking it into account when using this module.
 *
 * \warning Function fails (and does nothing) if input position is NOT
 * aligned as required by MP_ALIGN parameter. As mp_alloc() always returns
 * aligned pointers, this should not be a problem when using the function as
 * intended (passing pointers obtained by mp_alloc() calls).
 ****************************************************************************/
void mp_free_to(void *pos);

/************************************************************************//**
 * \brief Frees all the memory previously requested. 
 *
 * Deallocates all the memory previously reserved from the memory pool.
 *
 * \note In current implementation, this call is just an alias of mp_init().
 ****************************************************************************/
#define mp_free_pool()		mp_init()

#endif /*_MPOOL_H_*/

/** \} */

