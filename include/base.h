/*
 *  base.h
 *  Embedded
 *
 *  Created by Serge on 06/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

/*
 * Fixed size data types, suitable for ARM
 */
#if defined(__WXMAC__) || defined(__WXOSX_COCOA__) || defined(__WXOSX_CARBON__) || defined(__WX_COCOA__) || defined(__WXMSW__) || defined(__WXGTK__)
//
// If compiling with wxWidgets, use their definitions
//
#include <wx/defs.h>
typedef wxInt8 int8;
typedef wxByte uint8;
typedef wxInt16 int16;
typedef wxUint16 uint16;
typedef wxInt32 int32;
typedef wxUint32 uint32;
typedef wxInt64 int64;
typedef wxUint64 uint64;
#else
//
// Embedded case
//
typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int int32;
typedef unsigned long int uint32;
typedef long long int64;
typedef unsigned long long uint64;

#define USE_EMBEDDED 1

#endif


/*
 * Our basic data type for calculations.
 * A float may suffice, but if necessity arises, we'll turn it into fixed-point
 */
typedef float real_type;

#if defined(__cplusplus)
/**
 * Inheritors cannot be copied
 */
class NoCopy {
public:
  NoCopy() {
  }
private:
  NoCopy(const NoCopy&);
  void operator=(const NoCopy&);
};

/**
 * Inheritors cannot be instantiated
 */
class NoInstance {
private:
  NoInstance();
  ~NoInstance();
};

/**
 * Prevents instances from being allocated on the heap
 */
class NoHeap {
private:
#if !defined(_SIZE_T)
  typedef  unsigned int size_t;
#endif
  void* operator new(size_t size);
  void* operator new[](size_t size);
  void* operator new(size_t size, void* p);
  void operator delete(void* p);
  void operator delete[](void* p);
};

/**
 * Packed structure
 */
#if defined(__GNUC__)
#  define ZOROBO_PACKED __attribute__((packed))
#  define ZOROBO_ALIGNED(X) __attribute__((aligned(X)))
#else
#  error Packing only defined for gcc for now
#endif

/**
 * Gcc version check
 */
#if defined(__GNUC__)
#  define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#  define ZOROBO_CHECK_GCC_VERSION(v) (GCC_VERSION >= (v))
#endif

/**
 * Static size check
 */
#define ZOROBO_PASTE1(X, Y) X##Y
#define ZOROBO_PASTE(X, Y) ZOROBO_PASTE1(X, Y)
#define ZOROBO_CHECK_SIZE(T, S) \
typedef uint8 ZOROBO_PASTE(check_no_greater, __LINE__) [S - sizeof(T)]; \
typedef uint8 ZOROBO_PASTE(check_no_smaller, __LINE__) [sizeof(T) - S];

#endif
