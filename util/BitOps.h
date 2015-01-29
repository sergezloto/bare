/*
 *  BitOps.h
 *  Embedded
 *
 *  Created by Serge on 9/30/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  //
  // Rotate functions that C/C++ is lacking
  //
#if defined(__arm__)
#  if defined(__thumb__)
  inline
  uint32 ror(uint32 v, uint32 r) {
    asm(
        "  ror %0, %2  \r\n"
        :  "=r" (v) /* output */
        :  "0"  (v), "r" (r) /* input */
        :  /* clobbered */
        );
    return v;
  }

  inline
  uint32 rol(uint32 v, uint32 r) {
    const uint32 rr = 32 - r;
    asm(
        "  ror %0, %2  \r\n"
        :  "=r" (v) /* output */
        :  "0"  (v), "r" (rr) /* input */
        :  /* clobbered */
        );
    return v;
  }

#  else /* ARM mode */

  inline
  uint32 ror(uint32 v, uint32 r) {
    asm(
        "  mov %0, %1, ror %2  \r\n"
        :  "=r" (v) /* output */
        :  "0"  (v), "r" (r) /* input */
        :  /* clobbered */
        );
    return v;
  }
  
  inline
  uint32 rol(uint32 v, uint32 r) {
    const uint32 rr = 32 - r;
    asm(
        "  mov %0, %1, ror %2  \r\n"
        :  "=r" (v) /* output */
        :  "0"  (v), "r" (rr) /* input */
        :  /* clobbered */
        );
    return v;
  }
  
#  endif
#else
  // Generic implementation
  inline
  uint32 ror(uint32 v, uint32 r) {
    return (v >> r) | (v << (32-r));
  }
  inline
  uint32 rol(uint32 v, uint32 r) {
    return (v << r) | (v >> (32-r));
  }
#endif

  //
  // GCC Trick to fold the rotate if the rotate amount is known at compile time
  //
#define rol(v, r) \
	(__builtin_constant_p(r) ? \
	( (v << r) | (v >> (32 - r) ) ) :  \
	util::rol(v, r))
#define ror(v, r) \
	(__builtin_constant_p(r) ? \
	( (v >> r) | (v << (32 - r) ) ) :  \
	util::ror(v, r))
  
  
  
  /**
   * An overengineered trick to obtain the mask for N contiguous bits
   */
  template<uint32 MSB, uint32 LSB = 0> class Mask {
  public:
    static const uint32 ones = (MSB >= LSB) ? ((1 << MSB) | Mask<MSB-1, LSB>::ones) : 0;
    
    static const uint32 msb = 1 << MSB;
  };
  
  template<uint32 LSB> class Mask<0, LSB> {
  public:
    static const uint32 ones = (LSB == 0) ? 1 : 0;
  };
}
