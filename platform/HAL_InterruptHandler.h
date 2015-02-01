/*
 *  HAL_InterruptHandler.h
 *  Embedded
 *
 *  Created by Serge on 6/4/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

extern "C" {
  void oo_irq_handler(void*); 
}


namespace hal {
  class InterruptHandler {
  public:
    
  protected:
    /**
     * object oriented irq handler. Override in derived classes to 
     * do something useful
     */
    virtual void handle_irq() = 0;
    
    virtual ~InterruptHandler() {};  // Avoid warnings about non-virtual destructor
    
  private:
    friend void ::oo_irq_handler(void*);
    
    static void handle_irq(InterruptHandler& handler);
  };
}
