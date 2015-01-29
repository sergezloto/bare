/*
 *  ostream.h
 *  Embedded
 *
 *  Created by Serge on 03/07/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */
#ifdef __cplusplus
#pragma once

#include "base.h"
#include "ios.h"

namespace util {

  /**
   * A class that does not print anything
   */
  class  dummy_ostream: NoCopy {
  public:    
    dummy_ostream() {
    }
    
    dummy_ostream& operator<<(char) {
      return *this;
    }
    dummy_ostream& operator<<(const char*) {
      return *this;
    }
    
    dummy_ostream& operator<<(int16) {
      return *this;
    }
    
    dummy_ostream& operator<<(uint16) {
      return *this;
    }
    
    dummy_ostream& operator<<(int32) {
      return *this;
    }
    
    dummy_ostream& operator<<(uint32) {
      return *this;
    }
    
    dummy_ostream& operator<<(int64) {
      return *this;
    }
    
    dummy_ostream& operator<<(uint64) {
      return *this;
    }
    
    dummy_ostream& operator<<(bool) {
      return *this;
    }
    
    //ostream& operator<<(void* p);
    
  private:
    dummy_ostream(const dummy_ostream&);
    void operator=(const dummy_ostream&);
  };
  
  
  class  ostream: NoCopy {
public:
    typedef bool (*Putchar)(char c);
    
    ostream(Putchar putchar);
    
    ostream& operator<<(char c);
    ostream& operator<<(const char* str);
    ostream& operator<<(int16 d);
    ostream& operator<<(uint16 u);
    ostream& operator<<(int32 d);
    ostream& operator<<(uint32 u);
    ostream& operator<<(int64 d);
    ostream& operator<<(uint64 u);
    ostream& operator<<(bool b);
    
    ostream& operator<<(ios::base base);
    //ostream& operator<<(void* p);
    
private:
    void out_string(const char* s);
    void out_uint64(uint64 u);
    void out_int64(int64 i);
    void out_uint32(uint32 u);
    void out_int32(int32 i);
    
    const Putchar putchar;
    
    ios::base m_base;
  };
  
  inline
  ostream::ostream(Putchar putcharI)
  : putchar(putcharI), m_base(ios::dec) {
  }
  
  inline
  void ostream::out_string(const char* s) {
    while (*s) {
      putchar(*s);
      ++s;
    };    
  }
  
  inline
  ostream& ostream::operator<<(const char* str) {
    if (str == 0)
      out_string( "null");
    else
      out_string(str);
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(char c) {
    putchar(c);
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(bool b) {
    if (b)
      out_string("true");
    else
      out_string("false");
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(uint64 u) {
    out_uint64(u);
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(uint32 u) {
    out_uint32(u);
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(int16 d) {
    out_int32(d);
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(uint16 u) {
    out_uint32(u);
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(int64 d) {
    out_int64(d);
    return *this;
  }
  
  inline
  ostream& ostream::operator<<(int32 d) {
    out_int32(d);
    return *this;
  }
 
  inline
  ostream& ostream::operator<<(ios::base base) {
    m_base = base;
    return *this;
  }
}
#endif
