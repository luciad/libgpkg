#ifndef GPKG_ATOMIC_H
#define GPKG_ATOMIC_H

#include <stdint.h>

#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__MINGW32__)

static inline uint32_t atomic_inc_uint32(volatile uint32_t *value) {
  return InterlockedIncrement(value);
}

static inline uint32_t atomic_dec_uint32(volatile uint32_t *value) {
  return InterlockedDecrement(value);
}

#elif defined(__GNUC__) || (defined(__has_builtin) && __has_builtin(__sync_fetch_and_add) && __has_builtin(__sync_fetch_and_sub))

static inline uint32_t atomic_inc_uint32(volatile uint32_t *value) {
  return __sync_add_and_fetch(value, 1);
}

static inline uint32_t atomic_dec_uint32(volatile uint32_t *value) {
  return __sync_sub_and_fetch(value, 1);
}

#else

#error "Atomic operations not supported"

static inline uint32_t atomic_inc_uint32(volatile uint32_t *value) {
  *value += 1;
  return *value;
}

static inline uint32_t atomic_dec_uint32(volatile uint32_t *value) {
  *value -= 1;
  return *value;
}

#endif

#endif
