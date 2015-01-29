/*
 *  type_ops.h
 *
 *  Copyright 2012 Zorobo Pte Ltd. All rights reserved.
 *
 */

#pragma once

#include "base.h"

#if defined __cplusplus

namespace util {
  /**
   * Integral constant
   */
  template <typename T, T v>
  struct integral_constant {
    typedef integral_constant<T, v> type; 
    typedef T value_type; 
    static const T value = v;
  };
  
  /**
   * Template boolean value types
   */
  typedef integral_constant<bool, false> false_type;
  typedef integral_constant<bool, true> true_type;
  
  /**
   * Constness type traits
   */
  template <typename>
  struct is_const: public false_type {};
  template <typename T>
  struct is_const<T const>: public true_type {};
  
  /**
   * Volatile-ness type traits
   */
  template <typename>
  struct is_volatile: public false_type {};
  template <typename T>
  struct is_volatile<T volatile>: public true_type {};
  
  /**
   * Rid a type of constness
   */
  template <typename T>
  struct remove_const {
    typedef T type;
  };
  template <typename T>
  struct remove_const<T const> {
    typedef T type;
  };
  
  /**
   * Rid a type of volatile ness
   */
  template <typename T>
  struct remove_volatile {
    typedef T type;
  };
  template <typename T>
  struct remove_volatile<T volatile> {
    typedef T type;
  };
  
  /**
   * Rid a type of volatile and constness
   */
  template <typename T>
  struct remove_volatile_const {
    typedef T type;
  };
  template <typename T>
  struct remove_volatile_const<T volatile const> {
    typedef T type;
  };
  template <typename T>
  struct remove_volatile_const<T volatile> {
    typedef T type;
  };
  template <typename T>
  struct remove_volatile_const<T const> {
    typedef T type;
  };
  
  /**
   * Is array type traits
   */
  template <typename>
  struct is_array: public false_type {};
  template <typename T>
  struct is_array<const T[]>: public true_type {};
  
  /**
   * Integral type predicates
   */
  template <typename>
  struct z_is_integral: public false_type {};
  template <>
  struct z_is_integral<uint8>: public true_type {};
  template <>
  struct z_is_integral<uint16>: public true_type {};
  template <>
  struct z_is_integral<uint32>: public true_type {};
  template <>
  struct z_is_integral<uint64>: public true_type {};
  template <>
  struct z_is_integral<int8>: public true_type {};
  template <>
  struct z_is_integral<int16>: public true_type {};
  template <>
  struct z_is_integral<int32>: public true_type {};
  template <>
  struct z_is_integral<int64>: public true_type {};
  template <>
  struct z_is_integral<bool>: public true_type {};  // bool is integral too
  
  template <typename T>
  struct is_integral: public z_is_integral<typename remove_volatile_const<T>::type> {};
  
  /**
   * Floating point type predicates
   */
  template <typename>
  struct z_is_floating_point: public false_type {};
  template <>
  struct z_is_floating_point<float>: public true_type {};
  template <>
  struct z_is_floating_point<double>: public true_type {};
  template <typename T>
  struct is_floating_point: public z_is_floating_point<typename remove_volatile_const<T>::type> {};
  
  /**
   * Signed type predicates
   */
  template <typename>
  struct z_is_signed: public false_type {};
  template <>
  struct z_is_signed<float>: public true_type {};
  template <>
  struct z_is_signed<double>: public true_type {};
  template <>
  struct z_is_signed<int8>: public true_type {};
  template <>
  struct z_is_signed<int16>: public true_type {};
  template <>
  struct z_is_signed<int32>: public true_type {};
  template <>
  struct z_is_signed<int64>: public true_type {};
  template <typename T>
  struct is_signed: public z_is_signed<typename remove_volatile_const<T>::type> {};
  
  /**
   * Min numeric type value
   */
  template <typename>
  struct z_min {};
  template <>
  struct z_min<float> {static const float value = 1.17549435e-38f;};
#if Z_ENABLE_DOUBLE_TYPE
  template <>
  struct z_min<double> {static const double value = 2.2250738585072014e-308;};
#endif
  template <>
  struct z_min<uint8> {static const uint8 value = 0;};
  template <>
  struct z_min<uint16> {static const uint16 value = 0;};
  template <>
  struct z_min<uint32> {static const uint32 value = 0;};
  template <>
  struct z_min<uint64> {static const uint64 value = 0;};
  template <>
  struct z_min<int8> {static const uint8 value = 0xff;};
  template <>
  struct z_min<int16> {static const uint16 value = 0xffff;};
  template <>
  struct z_min<int32> {static const uint32 value = 0xffffffffl;};
  template <>
  struct z_min<int64> {static const uint64 value = 0xffffffffffffffffll;};
  template <typename T>
  struct type_min: public z_min<typename remove_volatile_const<T>::type> {};
  
  /**
   * Max numeric type value
   */
  template <typename>
  struct z_max {};
  template <>
  struct z_max<float> {static const float value = 3.40282347e+38f;};
#if Z_ENABLE_DOUBLE_TYPE
  template <>
  struct z_max<double> {static const double value = 1.7976931348623157e+308;};
#endif
  template <>
  struct z_max<uint8> {static const uint8 value = 0xff;};
  template <>
  struct z_max<uint16> {static const uint16 value = 0xffffu;};
  template <>
  struct z_max<uint32> {static const uint32 value = 0xffffffffu;};
  template <>
  struct z_max<uint64> {static const uint64 value = 0xffffffffffffffffull;};
  template <>
  struct z_max<int8> {static const uint8 value = 0x7f;};
  template <>
  struct z_max<int16> {static const uint16 value = 0x7fff;};
  template <>
  struct z_max<int32> {static const uint32 value = 0x7fffffff;};
  template <>
  struct z_max<int64> {static const uint64 value = 0x7fffffffffffffffll;};
  template <typename T>
  struct type_max: public z_max<typename remove_volatile_const<T>::type> {};
  
  /**
   * Type numeric attributes
   */
  template <typename T>
  struct z_limits {
    static const bool is_integral = is_integral<T>::value;
    static const bool is_floating_point = is_floating_point<T>::value;
    static const bool is_signed = is_signed<T>::value;
    static const uint32 bytes = sizeof(T);
    static const uint32 bits = 8 * sizeof(T);
    static const T min_value = type_min<T>::value;
    static const T max_value = type_max<T>::value;
  };
  template <typename T>
  struct limits: public z_limits<typename remove_volatile_const<T>::type> {};
  
  /**
   * Number of elements in sized array, 
   */
  template<typename T, uint32 N> 
  inline uint32 array_size(const T(&)[N]) {
    return N;
  }
}

#endif
