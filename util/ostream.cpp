/*
 *  ostream.cpp
 *  Embedded
 *
 *  Created by Serge on 03/07/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "ostream.h"

namespace util {
  
  
  void ostream::out_uint64(uint64 u) {
    // Recursive version
    switch (m_base) {
      case ios::dec:
        if (u < 10) {
          putchar(ios::digit(u));
        } else {
          out_uint64(u / 10);
          putchar(ios::digit(u % 10));
        }
        break;
        
      case ios::hex:
        if (u < 0x10) {
          putchar(ios::digit(u));
        } else {
          out_uint64(u / 0x10);
          putchar(ios::digit(u % 0x10));
        }        
        break;
    }
  }
    
  void ostream::out_uint32(uint32 u) {
    switch (m_base) {
      case ios::dec:
        if (u < 10) {
          putchar(ios::digit(u));
        } else {
          out_uint32(u / 10);
          putchar(ios::digit(u % 10));
        }
        break;
        
      case ios::hex:
        if (u < 0x10) {
          putchar(ios::digit(u));
        } else {
          out_uint32(u / 0x10);
          putchar(ios::digit(u % 0x10));
        }        
        break;
    }
  }
  
  void ostream::out_int64(int64 i) {
    if (i < 0) {
      putchar('-');
      i = -i;
    }
    out_uint64((uint64) i);
  }
  
  void ostream::out_int32(int32 i) {
    if (i < 0) {
      putchar('-');
      i = -i;
    }
    out_uint32((uint32) i);
  }
  
}
