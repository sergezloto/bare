/*
 *  HAL_Vectors.h
 *  Embedded
 *
 *  Created by Serge on 21/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */
#pragma once

namespace  hal {
  /*
   * This enumeration centralizes the definitions of interrupt priorities
   * for the devices we (may) use
   */
  enum Vectors {
    VEC_BOD	= 0,
    VEC_TIMER0,
    VEC_TIMER1,
    VEC_UART0,
    VEC_AD1,
    VEC_AD0,
    VEC_RTC,
    VEC_UART1,
    VEC_I2C0,
    VEC_I2C1,
    VEC_PWM0,
    VEC_LAST
  };
}
