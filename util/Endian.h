/*
 *  Endian.h
 *  Embedded
 *
 *  Created by Serge on 25/10/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once
/**
 * CAREFUL! We are using LITTLE ENDIAN as our communication protocol order.
 * Most of our target hardware is little endian so this saves lots of byte movements.
 */

#include "base.h"

namespace util {

#if defined(__GNUC__)
#  if defined(__arm__)
#    if defined(__ARMEL__)
#      define __LITTLE_ENDIAN__ 1
#    elif defined(__ARMEB__)
#      define __BIG_ENDIAN__ 1
#    endif
#  elif defined(__i386__) || defined(__x86_64__)
#    define __LITTLE_ENDIAN__ 1
#  elif defined(__ppc__)
#    define __BIG_ENDIAN__ 1
#  endif
#endif


#if 0 && defined(__arm__) && !defined(__thumb__)
  inline
  uint32 reverse(uint32 v) {
    uint32 t;
    asm(
        "  eor	%[t], %[n], %[n], ror #16	\r\n"
        "  mov	%[t], %[t], lsr #8		\r\n"
        "  bic	%[t], %[t], #0xff00		\r\n"
        "  eor	%[n], %[t], %[n], ror #8		\r\n"
        : [n] "=r" (v), [t] "=r" (t)     /* output */
        : "0" (v)     /*input */
        : /* clobber */
        );
    return t;
  }
#else
  inline
  uint16 reverse(uint16 v) {
    int16 t;
    t = (v >> 8) | (v << 8);
    return t;
  }
  
  inline
  uint32 reverse(uint32 v) {
    int32 t;
    
    t = (v >> 24)
    | (v << 24)
    | (v & 0x00ff0000) >> 8
    | (v & 0x0000ff00) << 8;
    return t;
  }
  
  inline
  uint64 reverse(uint64 v) {
    uint32 l = static_cast<uint32>(v & 0xffffffff);  // The bitwise and is probably overkill
    uint32 h = static_cast<uint32>(v >> 32);
    const uint64 t =  (static_cast<uint64>(reverse(l)) << 32) | reverse(h);
    return t;
  }
  
  inline
  float reverse(float v) {
    union {
      float f;
      uint32 u;
    } t;
    t.f = v;
    reverse(t.u);
    return t.f;
  }
  
#endif
  
  //
  // A hack to fold byte order reversing for constant expressions
  //
#define zorobo_reverse_constant_16(x) \
((uint32)((((uint32)(x) & 0xff00) >> 8) | \
(((uint32)(x) & 0x00ff) <<  8)))  
  
#define zorobo_reverse_constant_32(x) \
  ((uint32)((((uint32)(x) & 0xff000000) >> 24) | \
  (((uint32)(x) & 0x00ff0000) >>  8) | \
  (((uint32)(x) & 0x0000ff00) <<  8) | \
  (((uint32)(x) & 0x000000ff) << 24)))

#define zorobo_reverse_constant_64(x) \
  ((uint64) (zorobo_reverse_constant_32((x>>32) & 0xffffffff))) | \
  (((uint64) (zorobo_reverse_constant_32((x) & 0xffffffff))) << 32)
  
#define zorobo_reverse_64(x) \
(__builtin_constant_p(x) ? zorobo_reverse_constant_64(x) : reverse(x))
  
#define zorobo_reverse_32(x) \
  (__builtin_constant_p(x) ? zorobo_reverse_constant_32(x) : reverse(x))
  
#define zorobo_reverse_16(x) \
  (__builtin_constant_p(x) ? zorobo_reverse_constant_16(x) : reverse(x))
  
  
  inline
  char to_big_endian(char d) {
    return d;
  }

  inline
  char to_little_endian(char d) {
    return d;
  }

  inline
  uint8 to_big_endian(uint8 d) {
    return d;
  }
  
  inline
  uint8 to_little_endian(uint8 d) {
    return d;
  }
  
#if defined(__BIG_ENDIAN__) && defined(__LITTLE_ENDIAN__)
#  error "Both __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are defined"
#elif defined(__BIG_ENDIAN__)
  inline
  uint64 to_big_endian(uint64 d) {
    return d;
  }
  
  inline
  uint32 to_big_endian(uint32 d) {
    return d;
  }
  
  inline
  uint16 to_big_endian(uint16 d) {
    return d;
  }
  
  inline
  uint64 to_little_endian(uint64 d) {
    return zorobo_reverse_64(d);
  }
  
  inline
  uint32 to_little_endian(uint32 d) {
    return zorobo_reverse_32(d);
  }

  inline
  uint16 to_little_endian(uint16 d) {
    return zorobo_reverse_16(d);
  }
  
  inline
  float to_little_endian(float f) {
    return reverse(f);
  }
  
  inline
  uint64 to_host_order(uint64 d) {
    return d;
  }
  
  inline
  uint32 to_host_order(uint32 d) {
    return d;
  }
  
  inline
  uint32 to_network_order(uint32 d) {
    return d;
  }
  
  inline
  uint16 to_host_order(uint16 d) {
    return d;
  }
  
  inline
  uint16 to_network_order(uint16 d) {
    return d;
  }

  inline
  float to_host_order(float f) {
    return f;
  }
  
  inline
  float to_network_order(float f) {
    return f;
  }
#elif defined(__LITTLE_ENDIAN__)
  inline
  uint64 to_big_endian(uint64 d) {
    return zorobo_reverse_64(d);
  }
  
  inline
  uint64 to_little_endian(uint64 d) {
    return d;
  }
  
  inline
  uint32 to_big_endian(uint32 d) {
    return zorobo_reverse_32(d);
  }
  
  inline
  uint32 to_little_endian(uint32 d) {
    return d;
  }
  
  inline
  uint16 to_big_endian(uint16 d) {
    return zorobo_reverse_16(d);
  }
  
  inline
  uint16 to_little_endian(uint16 d) {
    return d;
  }
  
  inline
  float to_little_endian(float f) {
    return f;
  }
  
  inline
  uint64 to_host_order(uint64 d) {
    return zorobo_reverse_64(d);
  }
  
  inline
  uint64 to_network_order(uint64 d) {
    return zorobo_reverse_64(d);
  }  
  
  inline
  uint32 to_host_order(uint32 d) {
    return zorobo_reverse_32(d);
  }
  
  inline
  uint32 to_network_order(uint32 d) {
    return zorobo_reverse_32(d);
  }  
  
  inline
  uint16 to_host_order(uint16 d) {
    return zorobo_reverse_16(d);
  }
  
  inline
  uint16 to_network_order(uint16 d) {
    return zorobo_reverse_16(d);
  }  
  
  inline
  float to_host_order(float f) {
    return reverse(f);
  }
  
  inline
  float to_network_order(float f) {
    return reverse(f);
  }
  
#else
#  error "neither __BIG_ENDIAN__ nor __LITTLE_ENDIAN__ is defined"
#endif

#if 0
  // Defined out because sizeof bool is implementation dependent
  inline
  bool to_big_endian(bool b) {
    return to_big_endian(uint32(b));
  }
  
  inline
  bool to_little_endian(bool b) {
    return to_little_endian(uint32(b));
  }
#endif
}
