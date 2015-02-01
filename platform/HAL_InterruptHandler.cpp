/*
 *  HAL_InterruptHandler.cpp
 *  Embedded
 *
 *  Created by Serge on 6/4/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#include "HAL_InterruptHandler.h"

extern "C" {
  void oo_irq_handler(void *c_handler) {
    hal::InterruptHandler* handler = static_cast<hal::InterruptHandler*>(c_handler);
    handler->handle_irq();
  }
}
