/*
 *  stringstream.h
 *  Embedded
 *
 *  Created by Serge on 2/24/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "ios.h"
#include "array.h"

namespace util {
  template <uint32 CAPACITY>
  class stringstream {
  public:
    stringstream() : m_put(0), m_base(ios::dec) {
    }
    
    uint32 size() const {
      return m_put;
    }
    
    const char* str() {
      m_buffer[m_put] = '\0';
      m_put = 0;
      return m_buffer.get();
    }
    
    stringstream& operator<<(char c) {
      putchar(c);
      return *this;
    }
    
    stringstream& operator<<(const char* s) {
      if (s == 0)
        out_string( "null");
      else
        out_string(s);
      return *this;
    }
    
    stringstream& operator<<(int16 d) {
      out_int32(d);
      return *this;
    }
    
    stringstream& operator<<(uint16 u) {
      out_uint32(u);
      return *this;
    }
    
    stringstream& operator<<(int32 d) {
      out_int32(d);
      return *this;
    }
    
    stringstream& operator<<(uint32 u) {
      out_uint32(u);
      return *this;
    }
    
#if 0
    stringstream& operator<<(int d) {
      out_int32(d);
      return *this;
    }
    stringstream& operator<<(unsigned int u) {
      out_uint32(u);
      return *this;
    }
#endif
    stringstream& operator<<(int64 d) {
      out_int64(d);
      return *this;
    }
    
    stringstream& operator<<(uint64 u) {
      out_uint64(u);
      return *this;
    }
    
    stringstream& operator<<(float v) {
      if (v < 0) {
        *this << '-';
        v = -v;
      }
      const uint32 vi = (uint32) v;
      const uint32 vf = (uint32) ((v-vi)*1000);
      *this << vi;
      *this << '.';
      out_uint32(vf, 3);  // with 3 zeros
      
      return *this;      
    }
    
    stringstream& operator<<(bool b) {
      if (b)
        out_string("true");
      else
        out_string("false");
      return *this;
    }
    
    stringstream& operator<<(ios::base base) {
      m_base = base;
      return *this;
    }
    
    template <typename T, uint32 N>
    stringstream& operator<<(const array<T, N>& a) {
      for (uint32 i = 0; i < N; ++i) {
        *this << a[i];
      }
      return *this;
    }
    
    stringstream& out_uint32(uint32 v, uint32 digits) {
      char buf[digits+1];
      
      // Terminate char array
      buf[digits] = '\0';
      
      // process backwards
      do {
        -- digits;
        buf[digits] = ios::digit(v % m_base);
        v = v / m_base;
      } while (digits != 0);
      
      // Output as a string
      out_string(buf);
      return *this;
    }
    
    
  protected:
    void putchar(char c) {
      if (m_put < CAPACITY) {
        m_buffer[m_put++] = c;
      }
    }
    
  private:
    void out_string(const char* s) {
      while (*s) {
        putchar(*s);
        ++s;
      };    
    }
    
    void out_uint64(uint64 u) {
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
    
    void out_uint32(uint32 u) {
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
    
    void out_int64(int64 i) {
      if (i < 0) {
        putchar('-');
        i = -i;
      }
      out_uint64((uint64) i);
    }
    
    void out_int32(int32 i) {
      if (i < 0) {
        putchar('-');
        i = -i;
      }
      out_uint32((uint32) i);
    }
    
    
    array<char, CAPACITY+1> m_buffer;
    uint32 m_put;

    ios::base m_base;
  };
}
