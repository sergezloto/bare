/*
 *  DFT.h
 *  Embedded
 *
 *  Created by Serge on 29/6/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "SineLUT.h"

namespace util {
  /**
   * Fourier transforms
   */
  template <typename T>
  class Fourier: NoInstance {
  public:
    /**
     * Performs a DFT on a real input vector to a real and imaginary output vector.
     * The vectors must be preallocated.
     * @param x input vector
     * @param y output vector
     * @param N vector size
     */
    static void dft(const T* x, T* re, T* im, uint32 N) {
      T t_cos;
      T t_sin;
      for (uint32 k = 0; k < N; ++ k) {
        T t_re = T(0);
        T t_im = T(0);
        for (uint32 n = 0; n < N; ++n) {
          twiddle(k, N, t_cos, t_sin);
          t_re += x[n] * t_cos;
          t_im += x[n] * t_sin;
        } 
        re[k] = t_re;
        im[k] = t_im;
      }
    }
    
  private:
    /**
     * Calculates e^(-kj/n)
     * @return the sin and cos twiddle factors
     */
// TODO: FIXME: #warning Check the 2*PI factor in the twiddle
    static void twiddle(uint32 k, uint32 n, T& re, T& im) {
      // Compute the index to look for in the sine lookup table.
      const uint32 sine_index = k * SineLUT::SIZE / n;
      re = static_cast<T>(SineLUT::get(sine_index)) / SineLUT::SCALING;
      const uint32 cos_index = SineLUT::SIZE + SineLUT::SIZE / 8 - sine_index;
      im = static_cast<T>(SineLUT::get(cos_index)) / SineLUT::SCALING;
    }
  };
}
