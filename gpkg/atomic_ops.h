#ifndef GPKG_ATOMIC_H
#define GPKG_ATOMIC_H

#include <stdint.h>

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__MINGW32__)

#include <Windows.h>

static inline long atomic_inc_long(volatile long *value) {
  return InterlockedIncrement(value);
}

static inline long atomic_dec_long(volatile long *value) {
  return InterlockedDecrement(value);
}

#elif defined(__MACH__) || defined(__APPLE__)

#include <libkern/OSAtomic.h>
#include <limits.h>

#if ULONG_MAX == 0xffffffff
static inline long atomic_inc_long(volatile long *value) {
  return OSAtomicIncrement32((volatile int32_t *)value);
}

static inline long atomic_dec_long(volatile long *value) {
  return OSAtomicDecrement32((volatile int32_t *)value);
}
#elif ULONG_MAX == 0xffffffffffffffff
static inline long atomic_inc_long(volatile long *value) {
  return OSAtomicIncrement64((volatile int64_t *)value);
}

static inline long atomic_dec_long(volatile long *value) {
  return OSAtomicDecrement64((volatile int64_t *)value);
}
#else

#error "Unsupported long size"

#endif

#elif defined(__GNUC__) || (defined(__has_builtin) && __has_builtin(__sync_fetch_and_add) && __has_builtin(__sync_fetch_and_sub))

static inline long atomic_inc_long(volatile long *value) {
  return __sync_add_and_fetch(value, 1);
}

static inline long atomic_dec_long(volatile long *value) {
  return __sync_sub_and_fetch(value, 1);
}

#elif defined(__sun)

#include <atomic.h>

static inline long atomic_inc_long(volatile long *value) {
  return (long)atomic_inc_ulong_nv((volatile ulong_t *)value);
}

static inline long atomic_dec_long(volatile long *value) {
  return (long)atomic_dec_ulong_nv((volatile ulong_t *)value);
}

#else

#error "Atomic operations not supported"

static inline long atomic_inc_long(volatile long *value) {
  *value += 1;
  return *value;
}

static inline long atomic_dec_long(volatile long *value) {
  *value -= 1;
  return *value;
}

#endif

#endif
