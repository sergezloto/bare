/*
 *  Interpolator.h
 *  Embedded
 *
 *  Created by Serge on 20/10/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  
  /**
   * Interpolator that does nothing at all
   */
  template <typename TYPE_IN>
  class NullInterpolator: NoCopy {
  public:
    static const uint32 OVERSAMPLING_FACTOR = 1;
    
    /**
     * For now the type out is the same a the type out
     */
    typedef TYPE_IN TYPE_OUT;
    
    /**
     * Revert to an inital state
     */
    void reset() {
    }
    
    /**
     * Perform the interpolation
     * @return true if the sample in was consumed
     */
    bool interpolate(const TYPE_IN& sample_in, TYPE_OUT& sample_out) {
      sample_out = sample_in;
      return true;
    }
  };
  
  /**
   * Instances of this type perform a linear interpolation, resulting in twice as many samples.
   * It is assumed that the samples are equally spaced.
   */
  template <typename TYPE_IN>
  class LinearInterpolator: NoCopy {
  public:
    static const uint32 OVERSAMPLING_FACTOR = 2;
    
    /**
     * For now the type out is the same a the type out
     */
    typedef TYPE_IN TYPE_OUT;
    
    /**
     */
    LinearInterpolator() : m_interpolate_sample(false) {
    }
    
    /**
     * Revert to an inital state
     */
    void reset() {
      m_interpolate_sample = false;
    }
    
    /**
     * Perform the interpolation
     * @return true if the sample in was consumed
     */
    bool interpolate(const TYPE_IN& sample_in, TYPE_OUT& sample_out) {      
      if (m_interpolate_sample) {
        // We have a previous sample. Interpolate
        sample_out = (sample_in + m_previous) / 2;
        // Next time we do not interpolate
        m_interpolate_sample = false;
        return false;
      } else {
        sample_out = sample_in;
        // remember this sample.
        m_previous = sample_in;
        // We did not interpolate that sample, so it was consumed
        m_interpolate_sample = true;  // Next time we interpolate
        return true;
      }
    }
    
  private:
    bool m_interpolate_sample;
    TYPE_IN m_previous;
  };  
}
