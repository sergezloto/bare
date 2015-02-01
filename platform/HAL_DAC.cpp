/*
 *  HAL_DAC.cpp
 *  Embedded
 *
 *  Created by Serge on 6/17/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#include "HAL_DAC.h"

namespace lpc214x {
  volatile DAC::regDACR& DAC::dacr = *(volatile DAC::regDACR*) DAC::DACR_addr;
}
