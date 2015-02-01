/*
 *  HAL_PinSelect.cpp
 *  Embedded
 *
 *  Created by Serge on 17/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_PinSelect.h"

#include "lpc214x.h"

namespace hal {
  
  void PinSelect::enable(uint8 port, uint8 pin, uint8 function) {
    // Port 0, pin 0 to 15 are mapped to PINSEL0
    switch (port) {
      case 0:
        if (pin < 16) {
          const uint32 bits = (function) << (pin << 1);  // Address two bits in PINSEL0
          PINSEL0 &= ~(0x3 << (pin << 1)); // Clear all bits for that pin
          PINSEL0 |= bits;  // Sets bits that are set in the mask
        } else if (pin < 32) {
          const uint32 pin_bit_shift = 2 * (pin - 16);
          const uint32 bits = (function) << pin_bit_shift;  // Address two bits in PINSEL1
          PINSEL1 &= ~(0x3 << pin_bit_shift) ; // Clear bits for that pin
          PINSEL1 |= bits;  // Sets bits that are set in the mask
        }
        break;
      case 1:
        // Port 1 is special, that is, its pins cannot be enabled individually and
        // only pins from 16 to 31 exist at all
        // In practice we don't change the function for these pins because the 
        // alternate functions are TRACE and JTAGS, which we don't use
        // so do nothing!
        break;
      default:
        // This is an error for LPC21xx
        break;
    }		
  }
}
