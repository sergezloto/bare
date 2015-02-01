/*
 *  HAL_Driver.h
 *  Embedded
 *
 *  Created by Serge on 24/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"

namespace hal {
  
  /**
   * A base class, which concrete drivers will inherit.
   * Some common driver definitions and utilities will find their
   * place here.
   */
  class Driver {
public:
    /**
     * The peripheral clock, hardcoded to 60 MHz
     */
    static const uint32 PCLK = CONFIG_CLOCK;
    
    static uint32 bit(uint32 n);
    
protected:
    Driver();
  };
  
  inline Driver::Driver() {
  }
  
  inline uint32 Driver::bit(uint32 n) {
    return 1 << n;
  }
}
