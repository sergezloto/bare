/*
 *  OS_Reader.h
 *  Embedded
 *
 *  Created by Serge on 10/3/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "OS_os.h"
namespace os {
  /**
   * This class implements a blocking reader.
   * The template must support this function
   * bool get(char& c)
   * The read() function can be blocking or non blocking
   */
  template <class IO>
  class Reader : NoCopy {
    static const uint32 PEEK_MASK = 1<<31;
  public:
    enum BlockingMode {
      BLOCKING,
      NON_BLOCKING
    };
    
    Reader(IO& io) : m_io(io), m_peek_char(0), m_blocking_mode(BLOCKING) {
    }
    
    void set_mode(BlockingMode blocking_mode) {
      m_blocking_mode = blocking_mode;
    }
    
    /**
     * "standard" blocking read function
     */
    uint32 read(uint8 *bytes, uint32 count) {
      uint32 bytes_read = 0;
      
      while (bytes_read < count) {
        const uint32 bytes_read_this_turn = m_io.read(bytes + bytes_read, count - bytes_read);
        if (bytes_read_this_turn == 0) {
          if (m_blocking_mode == BLOCKING)
            os::OS::yield();
          else
            break;
        }
        
        bytes_read += bytes_read_this_turn;
      }
      return bytes_read;
    }
    
    /**
     * Peeks a single character
     */
    void peek(uint8& c) {
      if (m_peek_char & PEEK_MASK) {
        c = m_peek_char;
        return;
      }
      // Get it from the device
      while ( !m_io.get(c) )
        OS::yield();
      
      // Remember the character and mark it as peeked
      m_peek_char = c | PEEK_MASK;
    }
    
    /**
     * Reads a single character
     */
    void get(uint8& c) {
      if (m_peek_char & PEEK_MASK) {
        m_peek_char &= PEEK_MASK;
        c = m_peek_char;
        return;
      } 
      while ( !m_io.get(c) )
        OS::yield();
    }
    
    /**
     * Reads an integer. No overflow protection.
     * @return true if an integer was read
     */
    bool get(uint32& v) {
      bool got_it = false;
      uint32 t = 0;
      for (;;) {
        uint8 c;
        get(c);
        if ( c < '0' || c > '9' ) {
          break;
        }
        t = t * 10 + (c - '0');
        got_it = true;
      }
      return got_it;
    }
    
    bool get(int32& v) {
      enum Sign {
        Positive, Negative
      } sign = Positive;
      
      uint8 c;
      peek(c);
      
      if ( c == '-' ) {
        sign = Negative;
        // Dummy read to get rid of the peeked character
        get(c);
      } else if ( c < '0' || c > '9' ) {
        // Dummy read to get rid of the peeked character
        get(c);
        return false;
      }
      
      uint32 t;
      if ( !get(t) ) {
        return false;
      }
      
      if (sign == Positive) {
        v = t;
      } else {
        v = -t;
      }
      return true;
    }
    
  private:
    IO& m_io;
    /**
     * We remember a single peeked character here. It is marked with PEEK_MASK when peeked.
     * The mark is removed when the peeked character is get()'ed.
     */
    uint32 m_peek_char;
    
    BlockingMode m_blocking_mode;
  };  
}
