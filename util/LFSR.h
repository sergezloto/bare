/*
 *  LFSR.h
 *  Embedded
 *
 *  Created by Serge on 2/16/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  /**
   * A random number generator.
   * @see http://www.lomont.org/Math/Papers/2008/Lomont_PRNG_2008.pdf
   */
  class LFSR113 {
  public:
    /**
     * Constructor
     * @note the seed MUST satisfy z1 > 1, z2 > 7, z3 > 15, and z4 > 127 
     */ 
    LFSR113(uint32 s1 = 2, uint32 s2 = 8, uint32 s3 = 16, uint32 s4 = 128) {
      seed(s1, s2, s3, s4);
    }
    
    /**
     * @note the seed MUST satisfy z1 > 1, z2 > 7, z3 > 15, and z4 > 127 
     */ 
    void seed(uint32 s1, uint32 s2, uint32 s3, uint32 s4) {
      z1 = s1;
      z2 = s2;
      z3 = s3;
      z4 = s4;
    }

    /**
     * @return the next number
     */
    uint32 next() {
      uint32 b  = (((z1 << 6) ^ z1)   >> 13);  
      z1 = (((z1 & 4294967294u) << 18) ^ b);  
      b  = (((z2 << 2) ^ z2)   >> 27);  
      z2 = (((z2 & 4294967288u) <<  2) ^ b);  
      b  = (((z3 << 13) ^ z3)  >> 21);  
      z3 = (((z3 & 4294967280u) <<  7) ^ b);  
      b  = (((z4 << 3) ^ z4)   >> 12);  
      z4 = (((z4 & 4294967168u) << 13) ^ b);  
      return (z1 ^ z2 ^ z3 ^ z4);
    }
    
  private:
    uint32 z1;
    uint32 z2;
    uint32 z3;
    uint32 z4;
  };
}
