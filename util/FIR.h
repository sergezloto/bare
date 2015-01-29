/*
 *  FIR.h
 *  Embedded
 *
 *  Created by Serge on 1/21/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  
  /**
   * Implements a FIR filter with N taps
   */
  template <typename T, uint32 N>
  class FIR {
  public:
    FIR(const T coef[N]) {
      reset();
      for (uint32 i = 0 ; i < N; ++i)
        m_coef[i] = coef[i];
    }
    
    void reset() {
      m_primed = false;
      for (uint32 i = 0 ; i < N; ++i)
        m_taps[i] = T(0);
    }
    
    void add_value(const T& v) {
      if (!m_primed) {
        for (uint32 i = 0; i < N; ++i) {
          m_taps[i] = v;
        }
        m_primed = true;
      } else {
        for (uint32 i = N - 1 ; i > 0; --i)
          m_taps[i] = m_taps[i - 1];
        m_taps[0] = v;        
      }
    }
    
    T get_filtered_value() const {
      T temp(0);
      
      for (uint32 i = 0 ; i < N; ++i) {
        temp += m_coef[i] * m_taps[i];
      }
      return temp;
    }
    
  private:
    T m_taps[N];
    T m_coef[N];
    bool m_primed;
    
    // Ensure N is greater than 1
    typedef char ensure_fir_has_more_than_1_coef[N - 2];
  };
}
