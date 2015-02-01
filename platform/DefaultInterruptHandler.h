/*
 *  DefaultInterruptHandler.h
 *  Embedded
 *
 *  Created by Serge on 3/23/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"
#include "HAL_Driver.h"
#include "HAL_InterruptHandler.h"
#include "HAL_VIC.h"

namespace hal {
  class DefaultInterruptHandler: public InterruptHandler, NoCopy {
  public:
    DefaultInterruptHandler()
    : m_interrupt_count(0) {
    }
    
    bool configure() {
      VIC::set_default_handler(*this);
      return true;
    }
    
    uint32 get_interrupt_count() const {
      return m_interrupt_count;
    }
    
  private:
    void handle_irq() {
      ++m_interrupt_count;
    }
    
    volatile uint32 m_interrupt_count;
  };
}
