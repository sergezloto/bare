/*
 *  Variance.h
 *  Embedded
 *
 *  Created by Serge on 24/4/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"

namespace util {
  
  /**
   * Determines the variance of data.
   * Algo taken from http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
   */
  template <typename T>
  class Variance {
  public:
    Variance() : n(0), mean(0), m2(0) {      
    }
    
    void reset() {
      n = 0;
      mean = m2 = T(0);
    }
    
    void add_data(T x) {
      ++n;
      const T delta = x - mean;
      mean += delta / n;
      m2 += delta * (x - mean);
    }
    
    T get_mean() const {
      return mean;
    }
    
    T get_variance() const {
      //const T variance = m2 / n;
      const T variance = m2 / (n - 1);
      return variance;
    }
    
    uint32 get_sample_size() const {
      return n;
    }
    
  private:
    uint32 n;
    T mean;
    T m2;
  };
}
