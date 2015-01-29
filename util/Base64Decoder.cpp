/*
 *  Base64Decoder.cpp
 *  Embedded
 *
 *  Created by Serge on 29/10/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#include "Base64Decoder.h"

namespace util {

  namespace {
    enum {
      SKIP = 65,
      ERR = 66
    };
  }
#if TABLE_LOOKUP
  static const unsigned char char_to_six[256] =
  {
  /* ASCII table taken from Apache */
  /*       0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
  /* 0 */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* 1 */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* 2 */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
  /* 3 */ 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
  /* 4 */ 64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  /* 5 */ 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
  /* 6 */ 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  /* 7 */ 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
  /* 8 */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* 9 */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* A */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* B */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* C */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* D */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* E */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  /* F */ 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
  };
  
  static inline uint32 lookup(Base64Decoder::value_type b64) {
    return char_to_six[b64];
  }
#else
  static inline uint32 lookup(Base64Decoder::value_type b64) {
    switch(b64) {
      case 'A'...'Z':
        return b64 - 'A';
      case 'a'...'z':
        return b64 - 'a' + 26;
      case '0'...'9':
        return b64 - '0' + 26 * 2;
      case '+':
        return 10 + 26 * 2;
      case '/':
        return 1 + 10 + 26 * 2;
      case '=':
        return 64;
      case '\n':
      case '\r':
        return SKIP;
      default:
        return ERR;
    }
  }
#endif
  
  /**
   *
   */
  Base64Decoder::Status Base64Decoder::process(value_type b64, value_type& b) {
    bool got_byte = false;
    
    const value_type six_bits = lookup(b64);
    
    if (six_bits == SKIP) {
      // Do not take it into account
      return OK_NO_BYTE;
    } else if (six_bits == 64) {
      // We have reached the end of the base 64
      return DONE;
    }
    
    switch (m_b64_index & 0x03) {
      case 0:
        // First six bits, no output yet
        m_byte = six_bits << 2;  // get six bits
        break;
        
      case 1:
        // Get it out
        // We add two bits, and we assemble a byte
        b = m_byte | six_bits >> 4;  // get 2 high bits
        got_byte = true;
        // Prepare for the next step
        m_byte = six_bits << 4;  // get 4 low bits
        break;
        
      case 2:
        // Get it out
        b = m_byte | six_bits >> 2;  // get 2 high bits
        got_byte = true;
        // Prepare for next step
        m_byte = six_bits << 6;  // get 2 low bits
        break;
        
      case 3:
        // Get it out
        b = m_byte | six_bits;  // get all six bits
        got_byte = true;
        break;
    }
    // We have read one b64 byte. Remember that
    m_b64_index ++;

    
    return got_byte ? OK_BYTE : OK_NO_BYTE;
  }


#if 0
  Base64Decoder::Status Base64Decoder::process(buffer_type& src,
                                               buffer_type& dest) {    
    // Process the input
    while (! (src.is_empty() || dest.is_full())) {
      const value_type b64 = src.get();
      const value_type six_bits = lookup(b64);

      if (six_bits == 64) {
        // We have reached the end of the base 63
        // TODO: TERMINATION!
        
      }

      switch (m_b64_index & 0x03) {
        case 0:
          // First six bits, no output yet
          m_byte = six_bits << 2;  // get six bits
          break;
          
        case 1:
          // We add two bits, and we assemble a byte
          m_byte |= six_bits >> 4;  // get 2 high bits
          // Get it out!
          dest.put(m_byte);
          // Prepare for the next step
          m_byte = six_bits << 4;  // get 4 low bits
          break;
          
        case 2:
          m_byte |= six_bits >> 2;  // get 2 high bits
          // Get it out
          dest.put(m_byte);
          // Prepare for next step
          m_byte = six_bits << 6;  // get 2 low bits
          break;
          
        case 3:
          m_byte |= six_bits;  // get all six bits
          // Get it out
          dest.put(m_byte);
          break;
      }
      // We have read one b64 byte. Remember that
      m_b64_index ++;
    }
    return OK;
  }
#endif
}
