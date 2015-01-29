/*
 *  Math.h
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
   * @return the smallest of its arguments
   */
  template <typename T>
  T min(T a, T b) throw() {
    if (a < b)
      return a;
    return b;
  }

  /**
   * @return the largest of its arguments
   */
  template <typename T>
  T max(T a, T b) throw() {
    if (a > b)
      return a;
    return b;
  }
  
  /**
   * @return the absolute value of its argument
   */
  template <typename T>
  T abs(T a) throw() {
    if (a >= 0)
      return a;
    return -a;
  }
  
  /**
   * @return 1 if the argument is positive, -1 if negative, and 0 if null
   */
  template <typename T>
  int32 sign(T a) throw() {
    return a > T(0) ? 1 : (a < T(0) ? -1 : 0);
  }
  
  /**
   * @return the square of its argument
   */
  template <typename T>
  T sqr(T a) throw() {
    return a * a;
  }
  
  /**
   * @return true if v is between, inclusively, teh following arguments
   */
  template <typename T, typename C>
  bool between(T v, C a, C b) {
    return v >= a && v <= b;
  }
  
  /**
   * @return true if a and b are within epsilon of each other
   * @note it is understood that epsilon is positive
   */
  template <typename T, typename E>
  bool within(T a, T b, E epsilon) throw() {
    return (a >= b) ? (a - b < epsilon) : (b - a < epsilon);
  }
  
  /**
   * @return a value constrained within the given bounds
   */
  template <typename T, typename B>
  T clamp(const T value, const B lower, const B higher) {
    if (value < lower)
      return lower;
    if (value > higher)
      return higher;
    return value;
  }
  
  /**
   * @return the value quantized as per quantum
   */
  inline
  float quantize(float value, float quantum) {
    value += quantum / 2;
    const int32 d = static_cast<int32>(value / quantum);
    const float res = d * quantum;
    return res;
  }
  
  /**
   * A value with enforced integral boundaries
   */
  template <typename T>
  struct bound_type {
    typedef T type;
  };
  template <>
  struct bound_type<float> {
    typedef int32 type;
  };
  
  template <typename T, int32 lo, int32 hi>
  class bounded_value {
  public:
    typedef bounded_value<T, lo, hi> this_type;
    typedef T value_type;
    typedef typename bound_type<T>::type limit_type;
    
    static const limit_type LO = static_cast<limit_type>(lo);
    static const limit_type HI = static_cast<limit_type>(hi);

    bounded_value()
    : m_value(static_cast<T>(lo)) {
    }
    
    bounded_value(T v)
    : m_value(clamp(v, LO, HI)) {
    }
    
    bounded_value(const this_type& v) 
    : m_value(v.m_value) {
    }
    
    T get() const {
      return m_value;
    }
    
    operator T() const {
      return m_value;
    }
    
    T operator() () const{
      return m_value;
    }
    
    this_type& operator=(const this_type& v) {
      m_value = v.m_value;
      return *this;
    }

    this_type& operator=(T v) {
      m_value = clamp(v, LO, HI);
      return *this;
    }
  private:
    T m_value;
  };  
}
