/*
 *  HAL_SCB.cpp
 *  Embedded
 *
 *  Created by Serge on 08/07/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_SCB.h"

namespace hal {
  volatile uint32& SCB::RSIR = (*(volatile uint32*) 0xe01fc180);
}
