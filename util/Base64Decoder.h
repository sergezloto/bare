/*
 *  Base64Decoder.h
 *  Embedded
 *
 *  Created by Serge on 29/10/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  class Base64Decoder: NoCopy {
  public:
    enum Status {
      OK_BYTE,
      OK_NO_BYTE,
      DONE,
      ERROR_NOT_BASE64
    };
    
    typedef uint32 size_type;
    typedef uint8 value_type;
    //typedef Buffer<value_type, 32> buffer_type;

    Base64Decoder();
    
    void reset();

    /**
     * Process a single byte
     */
    Status process(value_type b64, value_type &b);

    /**
     */
    // Status process(buffer_type& b64, buffer_type& dest);
    
  private:
    size_type m_b64_index;
    value_type m_byte;
  };
  
  inline Base64Decoder::Base64Decoder() : m_b64_index(0) {
  }
  
  inline void Base64Decoder::reset() {
    m_b64_index = 0;
  }
}
