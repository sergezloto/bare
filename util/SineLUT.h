/*
 *  sine_table.h
 *  Embedded
 *
 *  Created by Serge on 21/3/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "BitOps.h"

namespace util {
  class SineLUT_1024;
  typedef SineLUT_1024 SineLUT;
  
  /**
   * This a table of 1024 signed 16 bit values for the sines from 0 to PI/2.
   * Values from PI/2 to 2*PI are obtained by symmetry.
   */
  class SineLUT_1024: NoInstance {
  public:
    static const uint32 NB_BITS = 12;
    static const uint32 NB_SAMPLES = 1 << NB_BITS;
    static const uint32 SIZE = NB_SAMPLES;
    static const uint32 SCALING = 1 << 15;
    
    /**
     * @param sample_index the angle value, mapped from [0,4096[ to [0, 2*PI[
     */
    static int16 get(uint32 sample_index) {
      // Put into our range
      sample_index &= MASK;
      
      const uint32 which_quarter_table = sample_index >> (NB_BITS - 2);
      switch (which_quarter_table) {
        case 0:  // sample_index [0..1023]
          return sine_table_1024[sample_index];
        case 1:  // sample_index [1024..2047]
          return sine_table_1024[HALF_INDEX - sample_index - 1];
        case 2: // sample_index [2048..3071]
          return -sine_table_1024[sample_index - HALF_INDEX];
        case 3:  // sample_index [3072..4095]
        default: // this one to make the compiler happy
          return -sine_table_1024[NB_SAMPLES - sample_index - 1];
      }
    }
    
    /**
     * We only store a quarter period as the rest can be obtained by symmetry
     */
    static const int16 sine_table_1024[NB_SAMPLES/4];
    
  private:
    static const uint32 HALF_INDEX = NB_SAMPLES >> 1;
    static const uint32 MASK = Mask<NB_BITS>::ones;
  };
}
