/*
 *  HAL_Flash.cpp
 *  Embedded
 *
 *  Created by Serge on 4/02/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */

#include "HAL_Flash.h"

using hal::Flash;

uint8 Flash::BUFFER[Flash::BUFFER_SIZE]  ZOROBO_ALIGNED(4);

#if DEBUG
hal::IAP::return_code Flash::m_last_code;
#endif
