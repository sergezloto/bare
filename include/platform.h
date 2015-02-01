/*
 *  platform.h
 *  Embedded
 *
 *  Created by Serge on 05/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "../platform/lpc214x.h"

#if defined(__cplusplus)

#include "../platform/HAL_Processor.h"
#include "../platform/HAL_SCB.h"
#include "../platform/HAL_VIC.h"
#include "../platform/HAL_Serial.h"
#include "../platform/HAL_Timer.h"
#include "../platform/HAL_GPIO.h"
#include "../platform/HAL_I2C.h"
#include "../platform/HAL_LIS3LV02DQ.h"
#include "../platform/HAL_HMC6352.h"
#include "../platform/HAL_ADC.h"
#include "../platform/HAL_PWM.h"
#include "../platform/HAL_DAC.h"

#include "../platform/HAL_AudioPlayer.h"
#include "../platform/HAL_IAP.h"
#include "../platform/HAL_Flash.h"

#include "../platform/DefaultInterruptHandler.h"

//#include "../platform/HAL_RecordStore.h"


namespace hal {
  // Make names easier
  typedef LIS3LV02DQ Accelerometer;
  typedef HMC6352 Magnetometer;
  
  typedef lpc214x::PWM PWM;
  typedef lpc214x::DAC DAC;
}

#endif
