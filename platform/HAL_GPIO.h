/*
 *  HAL_GPIO.h
 *  Embedded
 *
 *  Created by Serge on 17/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

#include "lpc214x.h"

#include "HAL_PinSelect.h"

#include "util.h"  // For mask

namespace hal {
  /*
   * Control for GPIO.
   * At reset all IO pins are set to GPIO function, IN.
   */
  class GPIO: public NoInstance, NoCopy {
  public:
    /*
     * The port we're talking to
     */
    enum Port {
      PORT_0 = 0,
      PORT_1 = 1
    };
    
    /*
     * Is the pin an IN pin or is it OUT
     */
    enum Direction {
      IN = 0,
      OUT = 1
    };
    
    /*
     * Sets up a pin for input or output
     */
    static void configure(Port port, uint8 pin, Direction dir);
    
    /*
     * For an OUT pin, sets it low or high
     * @param value true for high, false for low
     */
    static void set(Port port, uint8 pin, bool value);
    
    /*
     * acts upon a set of OUT pins at the same time on the given port
     * @param value true for high, false for low
     */
    static void set_mask(Port port, uint32 pin_mask, bool value);
    
    /*
     * For an IN pin, read its value
     */
    static bool get(Port port, uint8 pin);

    /*
     * @return the value of all pins of the given port
     */
    static uint32 get_all(Port port);
  };
  
  inline
  void GPIO::configure(Port port, uint8 pin, Direction dir) {
    // Set the function for the requested pin
    PinSelect::enableGPIO(port, pin);
    
    switch (port) {
      case PORT_0:
        if (dir == IN) 
          IODIR0 &= ~(1 << pin);
        else
          IODIR0 |= 1 << pin;
        break;
        case PORT_1:
        if (dir == IN) 
          IODIR1 &= ~(1 << pin);
        else
          IODIR1 |= 1 << pin;
        break;
    }
  }
  
  inline
  void GPIO::set_mask(Port port, uint32 pin_mask, bool value) {
    switch (port) {
      case PORT_0:
        if (value)
          IO0SET = pin_mask;
        else
          IO0CLR = pin_mask;
        break;
        case PORT_1:
        if (value)
          IO1SET = pin_mask;
        else
          IO1CLR = pin_mask;
        break;
    }
  }
  
  inline
  void GPIO::set(Port port, uint8 pin, bool value) {
    set_mask(port, 1 << pin, value);
  }
  
  inline
  bool GPIO::get(Port port, uint8 pin) {
    switch (port) {
      case PORT_0:
        return IO0PIN & (1 << pin);
      case PORT_1:
        return IO1PIN & (1 << pin);
    }
    return false;  // To make GCC happy
  }
  
  inline
  uint32 GPIO::get_all(Port port) {
    switch (port) {
      case PORT_0:
        return IO0PIN;
      case PORT_1:
        return IO1PIN;
    }
  }
}

