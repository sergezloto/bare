/*
 *  Base64Encoder.h
 *  Embedded
 *
 *  Created by Serge on 18/12/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "array.h"

namespace util {
  class Base64Encoder: NoCopy {
  public:
    typedef uint32 size_type;
    typedef uint8 in_value_type;
    typedef char out_value_type;
        
    Base64Encoder();
    
    void reset();
    
    /**
     * @return the number of bytes written
     */
    size_type write(const in_value_type* b, size_type size);
    
    /**
     * Reads the end of input
     * @return the next characters, including padding, of the encoding, or
     *          '\000' if no more to come
     */
    out_value_type read_end();
    
    /**
     * @return the nuber of bytes read
     */
    size_type read(out_value_type* c, size_type size);
        
    /**
     * Evaluates the out size for a given in_size
     */
    static size_type get_out_size(size_type in_size);

    /**
     * Encodes a whole area in one go.
     * The destination area *must* be big enough to accomodate the result plus a final '\0'.
     */
    static void process(const in_value_type* area, size_type in_size, out_value_type* dest);
        
  private:    
    static const out_value_type t64[];
    
    uint8 m_mod3;
    
    union {
      uint32 m_c_all;
      uint8 m_c[4];      
    };

    union {
      uint32 m_b_all;
      uint8 m_b[2];
    };
  };
  
  inline 
  Base64Encoder::Base64Encoder() 
  : m_mod3(0), m_c_all(0), m_b_all(0) {
  }
  
  inline void Base64Encoder::reset() {
    m_mod3 = 0;
    m_c_all = 0;
    m_b_all = 0;
  }
  
  
  inline
  Base64Encoder::size_type Base64Encoder::get_out_size(size_type in_size) {
    // We pack 3 bytes into 4 bytes
    // and the result is padded to 4 bytes
    return (2 + in_size - ((in_size + 2) % 3)) * 4 / 3;
  }
}
