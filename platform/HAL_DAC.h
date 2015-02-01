/*
 *  HAL_DAC.h
 *  Embedded
 *
 *  Created by Serge on 6/17/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"
#include "util.h"
#include "HAL_Driver.h"
#include "HAL_PinSelect.h"

namespace lpc214x {
  
  /**
   * LPC214x DAC function.
   * sets the DAC to emit a 10-bit value.
   */
  class DAC: public hal::Driver, NoCopy, NoInstance {
  public:
    static void configure() {
      // Set up the initial value to 0
      set_value(int16(0));
      // Configure the device pin
      hal::PinSelect::enableDAC();
    }
    
    /**
     * @param value a 16 bit value to be fed to the DAC. The lower 6 bits are not significant.
     */
    static void set_value(uint16 value) {
      //dacr.value = value;
      dacr.raw_value = value | (DAC_BIAS << 16);  // value and bias 
    }
    
    /**
     * @param value a 16 bit signed value to be fed to the DAC. The lower 6 bits are not significant.
     */
    static void set_value(int16 value) {
      // The DC offset is at 1<<15
      // We also round to the nearest (1<<6) and saturate if necessary
      const uint32 vdc = util::min(0x8000u + value + (1<<5), 0xffffu);
      dacr.raw_value = vdc | (DAC_BIAS << 16);  // value and bias 
    }
                                          
  private:
    static const uint32 DACR_addr = 0xe006c000;
    static const uint32 DAC_BIAS = 0;  // 0: The settling time of the DAC is 1 μs max, and the maximum current is 700 μA.
    
    union regDACR {
      struct {
        unsigned : 6;  // ignored
        unsigned value: 10;
        unsigned bias: 1;
      };
      uint32 raw_value;
    };
    static volatile regDACR& dacr;
  };
  
}
