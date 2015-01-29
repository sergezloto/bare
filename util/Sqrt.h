/*
 *  Sqrt.h
 *  Embedded
 *
 *  Created by Serge on 2/5/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

namespace util {

  /**
   * A quick approximate square root function
   */
  inline
  float approx_sqrt(float val)  {
    union
    {
      int tmp;
      float val;
    } u;
    u.val = val;

    const int m = -0x4C000;
    u.tmp = (1<<29) + (u.tmp >> 1) - (1<<22) + m;

    // One extra step to improve accuracy, at the expense of 
    // a division
    u.val = 0.5f * (u.val + val/u.val);
    
    return u.val;
  }
  
  /**
   * @param x
   * @return 1/sqrt(x)
   */
  inline 
  float approx_inv_sqrt(const float x) {
    const float xhalf = 0.5f*x;
    const int SQRT_MAGIC_F = 0x5f3759df;
    
    union // get bits for floating value
    {
      float x;
      int i;
    } u;
    u.x = x;
    u.i = SQRT_MAGIC_F - (u.i >> 1);  // gives initial guess y0
    return u.x*(1.5f - xhalf*u.x*u.x);// Newton step, repeating increases accuracy
  }
}
