/*
 *  Q.h
 *  Embedded
 *
 *  Created by Serge on 5/6/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"

namespace util {
  
  /**
   * Type promotion to next size
   */
  template <typename> struct extend {};
  template <> struct extend<int8> {typedef int16 type;};
  template <> struct extend<int16> {typedef int32 type;};
  template <> struct extend<int32> {typedef int64 type;};
  template <> struct extend<int64> {typedef int64 type;};  // no extension
  template <> struct extend<uint8> {typedef uint16 type;};
  template <> struct extend<uint16> {typedef uint32 type;};
  template <> struct extend<uint32> {typedef uint64 type;};
  template <> struct extend<uint64> {typedef uint64 type;};  // no extension
  
  
  /**
   * Fixed point class that stores 32 bits numbers, with N fractional bits.
   * There is no check for overflow, and wraparoud occurs in this case
   */
  template<uint32 N, typename T = int32>  class Q {
  public:
    typedef T storage_type;
    
    static const uint32 bits = sizeof(T) << 3;
    static const uint32 bits_integral = bits - N;
    static const uint32 bits_fractional = N;
    
    static const Q zero;
    static const Q one;
    static const Q min_value;
    static const Q max_value;
    
    explicit Q() 
    : m_value(0) {
    }
    
    explicit Q(int32 value) 
    : m_value(value << N) {
    }
    
    explicit Q(uint32 value) 
    : m_value(value << N) {
    }
    
#if DEPRECATED
    explicit Q(float value)
    : m_value(static_cast<storage_type>(value * (1<<N))){
    }    
#else
    /**
     * @brief Constructor from float
     * Does not use any floating point function
     */
    explicit Q(float value) {
      union {
        float f;
        uint32 i;
        struct {
          unsigned int mantissa:23;
          unsigned int exponent:8;
          unsigned int negative:1;
        };
      } f;
      f.f = value;
      if (f.i == 0) {
        m_value = 0;
        return;
      }
      
      const uint8 exponent_bias = 0x7f;
      const int8 exp = f.exponent - exponent_bias;
      const uint32 mant =  f.mantissa | (1<<23);
      
      const int32 shift = 23 - N - exp;

      if (shift >= 0)
        m_value = mant >> shift;
      else
        m_value = mant << -shift;
      
      if (f.negative)
        m_value = -m_value;
    }
#endif
    
    Q(const Q<N,T>& v)
    : m_value(v.m_value) {
    }
    
    Q<N,T>& operator=(const Q<N, T>& v) {
      m_value = v.m_value;
      return *this;
    }  
    
    Q<N,T>& operator=(int32 i) {
      m_value = i << N;
      return *this;
    }
    
    int32 to_int32() const {
      return static_cast<int32>(m_value >> N);
    }
    
    float to_float() const {
      return static_cast<float>(m_value) / (1<<N);
    }

    Q<N, T> frac() const {
      return Q<N, T>(m_value & Mask<N-1, 0>::ones, false);
    }

    Q<N, T> floor() const {
      return Q<N, T>(m_value >> N);
    }
    
    Q<N, T> round() const {
      return Q<N, T>(m_value + (1 << (N-1)) >> N);
    }
    
    static const Q<N, T>& min() {
      return min_value;
    }
    static const Q<N, T>& max() {
      return max_value;
    }
    
    /**
     * @return the signed number of leading zeroes in value left of the binary point
     */
    int32 magnitude() const {
      if (*this == zero)
        return -bits_fractional;
      return bits_integral - __builtin_clz(m_value);
    }
    
  private:
    storage_type m_value;
    
    Q(storage_type raw_value, bool /*dummy*/)
    : m_value(raw_value) {
    }
    
    template<uint32 M, typename U> friend Q<M, U> operator -(const Q<M, U>& a);

    template<uint32 M, typename U> friend Q<M, U> operator +(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U> operator -(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U> operator *(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U> operator /(const Q<M, U>& a, const Q<M, U>& b);
    
    template<uint32 M, typename U> friend Q<M, U>& operator +=(Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U>& operator -=(Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U>& operator *=(Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U>* operator /=(Q<M, U>& a, const Q<M, U>& b);
    
    template<uint32 M, typename U> friend Q<M, U> operator +(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend Q<M, U> operator -(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend Q<M, U> operator *(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend Q<M, U> operator /(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend Q<M, U> operator +(int32 a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U> operator -(int32 a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U> operator *(int32 a, const Q<M, U>& b);
    template<uint32 M, typename U> friend Q<M, U> operator /(int32 a, const Q<M, U>& b);
    
    
    template<uint32 M, typename U> friend Q<M, U> operator <<(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend Q<M, U> operator >>(const Q<M, U>& a, int32 b);
    
    template<uint32 M, typename U> friend bool operator ==(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend bool operator !=(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend bool operator <(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend bool operator <=(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend bool operator >(const Q<M, U>& a, const Q<M, U>& b);
    template<uint32 M, typename U> friend bool operator >=(const Q<M, U>& a, const Q<M, U>& b);

    template<uint32 M, typename U> friend bool operator <(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend bool operator <=(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend bool operator >(const Q<M, U>& a, int32 b);
    template<uint32 M, typename U> friend bool operator >=(const Q<M, U>& a, int32 b);

    /**
     * @brief Square root function by Newton's method
     * @param x a positive value
     * @return the positive square root of the parameter
     */
    template<uint32 M, typename U> friend Q<M, U> sqrt(const Q<M, U>& x);
    /**
     * @brief Cosine function by Taylor expansion to the 5th order
     * @param x  an angle in radians. Valid input is [-pi/2, pi/2]
     * @return the cosine of an angle in radians
     */
    template<uint32 M, typename U> friend Q<M, U> sin(const Q<M, U>& x);
    
  };

  
  template<uint32 M, typename U>
  const Q<M,U> Q<M,U>::zero;
  template<uint32 M, typename U>
  const Q<M,U> Q<M,U>::one(1<<M,false);
  template<uint32 M, typename U>
  const Q<M,U> Q<M,U>::min_value(util::limits<U>::min_value,false);
  template<uint32 M, typename U>
  const Q<M,U> Q<M,U>::max_value(util::limits<U>::max_value,false);



  /**
   * Unary neg
   */
  template<uint32 N, typename T> Q<N, T> operator -(const Q<N, T>& a) {
    return Q<N, T>(-a.m_value, false);
  }
  
  /**
   * Addition of Qn numbers
   */
  template<uint32 N, typename T>
  Q<N, T> operator +(const Q<N, T>& a, const Q<N, T>& b) {
    return Q<N, T>(a.m_value + b.m_value, false);
  }
  
  /**
   * Difference of Qn numbers
   */
  template<uint32 N, typename T>
  Q<N, T> operator -(const Q<N, T>& a, const Q<N, T>& b) {
    return Q<N, T>(a.m_value - b.m_value, false);
  }
  
  /**
   * Product of Qn numbers
   */
  template<uint32 N, typename T>
  Q<N, T> operator *(const Q<N, T>& a, const Q<N, T>& b) {
    // Multiply and round from 0.5 up
    typedef typename extend<T>::type promo_t;
    const promo_t tmp = static_cast<promo_t>(a.m_value) * b.m_value + (1 << (N-1));
    return Q<N, T>(static_cast<T>(tmp >> N), false);
  }
  
  /**
   * Division of Qn numbers
   */
  template<uint32 N, typename T>
  Q<N, T> operator /(const Q<N, T>& a, const Q<N, T>& b) {
    typedef typename extend<T>::type promo_t;
    // Scale and round up
    const promo_t tmp = (static_cast<promo_t>(a.m_value) << N) + (b.m_value / 2);
    return Q<N, T>(static_cast<T>(tmp / b.m_value), false);
  }
  
  /**
   */
  template<uint32 N, typename T>
  Q<N, T>& operator +=(Q<N, T>& a, const Q<N, T>& b) {
    a.m_value += b.m_value;
    return a;
  }
  
  /**
   */
  template<uint32 N, typename T>
  Q<N, T>& operator -=(Q<N, T>& a, const Q<N, T>& b) {
    a.m_value -= b.m_value;
    return a;
  }

  /**
   */
  template<uint32 N, typename T>
  Q<N, T>& operator *=(Q<N, T>& a, const Q<N, T>& b) {
    typedef typename extend<T>::type promo_t;
    // Multiply and round from 0.5 up
    const promo_t tmp = static_cast<promo_t>(a.m_value) * b.m_value + (1 << (N-1));
    a.m_value = static_cast<T>(tmp >> N);
    return a;
  }
  
  /**
   */
  template<uint32 N, typename T>
  Q<N, T>& operator /=(Q<N, T>& a, const Q<N, T>& b) {
    typedef typename extend<T>::type promo_t;
    // Scale and round up
    const promo_t tmp = static_cast<promo_t>(a.m_value << N) + (b.m_value / 2);
    a.m_value = static_cast<T>(tmp / b.m_value);
    return a;
  }
  
  /**
   * Addition of a Qn and an int
   */
  template<uint32 N, typename T>
  Q<N, T> operator +(const Q<N, T>& a, int32 b) {
    return Q<N, T>( a.m_value + (b << N), false);
  }
  template<uint32 N, typename T>
  Q<N, T> operator +(int32 a, const Q<N, T>& b) {
    return Q<N, T>( (b << N) + b.m_value , false);
  }
  
  /**
   * Difference of a Qn and an int
   */
  template<uint32 N, typename T>
  Q<N, T> operator -(const Q<N, T>& a, int32 b) {
    return Q<N, T>( a.m_value - (b << N), false);
  }
  template<uint32 N, typename T>
  Q<N, T> operator -(int32 a, const Q<N, T>& b) {
    return Q<N, T>( (a << N) - b.m_value, false);
  }
  
  /**
   * Product of a Qn and an int
   */
  template<uint32 N, typename T>
  Q<N, T> operator *(const Q<N, T>& a, int32 b) {
    return Q<N, T>( a.m_value * b, false);
  }
  template<uint32 N, typename T>
  Q<N, T> operator *(int32 a, const Q<N, T>& b) {
    return Q<N, T>( a * b.m_value, false);
  }
  
  /**
   * Division of a Qn and an int
   */
  template<uint32 N, typename T>
  Q<N, T> operator /(const Q<N, T>& a, int32 b) {
    return Q<N, T>( a.m_value / b, false);
  }
  template<uint32 N, typename T>
  Q<N, T> operator /(int32 a, const Q<N, T>& b) {
    return Q<N, T>(a) / b;
  }
  
  /**
   * Powers of two
   */
  template<uint32 N, typename T>
  Q<N, T> operator >>(const Q<N, T>& a, int32 b) {
    return Q<N, T>(a.m_value >> b, false);
  }
  
  template<uint32 N, typename T>
  Q<N, T> operator <<(const Q<N, T>& a, int32 b) {
    return Q<N, T>(a.m_value << b, false);
  }
  
  /**
   * Comparison
   */
  template<uint32 N, typename T>
  int32 compare(const Q<N, T>& a, const Q<N, T>& b) {
    return a.m_value - b.m_value;
  }
  
  template<uint32 N, typename T>
  bool operator ==(const Q<N, T>& a, const Q<N, T>& b) {
    return a.m_value == b.m_value;
  }
  
  template<uint32 N, typename T>
  bool operator !=(const Q<N, T>& a, const Q<N, T>& b) {
    return a.m_value != b.m_value;
  }
  
  template<uint32 N, typename T>
  bool operator <=(const Q<N, T>& a, const Q<N, T>& b) {
    return a.m_value <= b.m_value;
  }
  
  template<uint32 N, typename T>
  bool operator >=(const Q<N, T>& a, const Q<N, T>& b) {
    return a.m_value >= b.m_value;
  }
  
  
  template<uint32 N, typename T>
  bool operator <(const Q<N, T>& a, const Q<N, T>& b) {
    return a.m_value < b.m_value;
  }
  
  template<uint32 N, typename T>
  bool operator >(const Q<N, T>& a, const Q<N, T>& b) {
    return a.m_value > b.m_value;
  }
  
  template<uint32 N, typename T>
  bool operator <(const Q<N, T>& a, int32 b) {
    return (a.m_value >> N) < b;
  }
  
  template<uint32 N, typename T>
  bool operator <=(const Q<N, T>& a, int32 b) {
    return (a.m_value >> N) <= b;
  }
  
  template<uint32 N, typename T>
  bool operator >(const Q<N, T>& a, int32 b) {
    return (a.m_value >> N) > b;
  }
  
  template<uint32 N, typename T>
  bool operator >=(const Q<N, T>& a, int32 b) {
    return (a.m_value >> N) >= b;
  }

#if 1 // This one is faster for now
  template<uint32 N, typename T>
  Q<N, T> sqrt(const Q<N, T>& y) {
    typedef Q<N,T> Q;
    if (y == Q::zero || y == Q::one)
      return y;
    
    const int32 magnitude = y.magnitude();
    // An estimate square root will have about half magnitude
    Q xn = y >> (magnitude / 2);

    Q x;  // Defaulted to 0
    
    for (uint32 i = 10; i > 0 && x != xn; --i) {
      // Newton iteration
      x = xn;
      const Q yn = x * x - y;
      const Q dy =  x << 1;  // 2x, derivative of x^2-y
      
      xn = x - yn / dy;
    }
    return xn;
  }
#else
  template<uint32 N, typename T>
  Q<N, T> sqrt(const Q<N, T>& y) {
    typedef Q<N,T> Q;
    if (y == Q::zero || y == Q::one)
      return y;
    
    const int32 magnitude = y.magnitude();
    // An estimate square root will have about half magnitude
    Q xn = y >> (magnitude / 2);

    Q x;  // Defaulted to 0

    for (uint32 i = 10; i > 0 && x != xn; --i) {
      // Newton iteration
      x = xn;
      
      xn -=  (x - y/x) >> 1;
      //xn = x - yn / dy;
    }
    return xn;
  }
#endif
  
  template<uint32 N, typename T>
  Q<N, T> sin(const Q<N, T>& x) {
    typedef Q<N,T> Q;
    const Q x2(x * x);
    const Q x3(x2 * x);
    const Q x5(x3 * x2);
    return x - x3 / 6 + x5 / 120;
  }

  template<uint32 N, typename T>
  Q<N, T> inverse(const Q<N, T>& y) {
    typedef Q<N,T> Q;
    
    if (y == Q::zero)  // pathological case
      return Q::max();

    if (y == Q::one)
      return y;
    
    const int32 magnitude = y.magnitude();
    // An estimate inverse will have its magnitude reversed
    Q xn;
    if (magnitude >= 0)
      xn = y >> (2 * magnitude);
    else
      xn = y << (-2 * magnitude);      
    
    Q x;  // Defaulted to 0
    
    for (uint32 i = 10; i > 0 && x != xn; --i) {
      // Newton iteration
      x = xn;      
      xn += x - x * x * y;
    }
    return xn;
  }
  
}
