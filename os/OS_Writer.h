/*
 *  Writer.h
 *  Embedded
 *
 *  Created by Serge on 11/10/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "util.h" // For Q<N>
#include "OS_os.h"

namespace os {
  /**
   * This class implements a blocking writer.
   * The template must support this function
   * bool put(char& c).
   * The write() function can be non blocking.
   */
  template <class IO>
  class Writer : NoCopy {
  public:
    /**
     */
    enum BlockingMode {
      BLOCKING,
      NON_BLOCKING
    };
    
    Writer(IO& io, BlockingMode blocking_mode = BLOCKING) : m_io(io), m_blocking_mode(blocking_mode) {
    }
    
    /**
     * "standard" write function.
     * If the mode is set to non-blocking, the \fn write function may write fewer bytes than requested.
     * @return the number of bytes written
     */
    uint32 write(const uint8 *bytes, uint32 count) {
      uint32 bytes_sent = 0;
      while (bytes_sent < count) {
        const uint32 bytes_sent_this_turn = m_io.write(bytes + bytes_sent, count - bytes_sent);
        if (bytes_sent_this_turn == 0) {
          if (m_blocking_mode == BLOCKING)
            os::OS::yield();
          else 
            break;
        }
        bytes_sent += bytes_sent_this_turn;
      }
      return bytes_sent;
    }
    
    /**
     * Always blocking
     */
    void put(char c) {
      while (m_io.write(reinterpret_cast<const uint8*> (&c), 1) != 1)
        os::OS::yield();
    }
    
    /**
     * Write a string, always blocking
     */
    void put(const char v[]) {

      const uint32 count = util::strlen(v);
      uint32 bytes_sent = 0;
      while (bytes_sent < count) {
        const uint32 bytes_sent_this_turn = m_io.write(reinterpret_cast<const uint8*>(v) + bytes_sent, count - bytes_sent);
        if (bytes_sent_this_turn == 0)
          OS::yield();
        bytes_sent += bytes_sent_this_turn;
      }
    }
    
    void put(int32 v) {
      if (v < 0) {
        put('-');
        v = -v;
      }
      put(static_cast<uint32>(v));
    }
    
    void put(uint32 v) {
      if (v < 10) {
        put(static_cast<char>(v + '0'));
      } else {
        put( v / 10 );
        put(static_cast<char>(v % 10 + '0'));
      }
    }
    
    void put(uint32 v, uint32 digits) {
      char buf[digits+1];
      
      // Terminate char array
      buf[digits] = '\0';
      
      // process backwards
      do {
        -- digits;
        buf[digits] = '0' + v % 10;
        v = v / 10;
      } while (digits != 0);
      
      // Output as a string
      put(buf);
    }
    
    void put(bool b) {
      put( b ? "true" : "false" );
    }
    
    Writer& operator<<(char c) {
      put(c);
      return *this;
    }

    Writer& operator<<(const char v[]) {
      put(v);
      return *this;
    }
    
    Writer& operator<<(int32 v) {
      put(v);
      return *this;
    }

    Writer& operator<<(uint32 v) {
      put(v);
      return *this;
    }

    Writer& operator<<(bool b) {
      put(b);
      return *this;
    }
    
    Writer& operator<<(float v) {
      if (v < 0) {
        put('-');
        v = -v;
      }
      const uint32 vi = (uint32) v;
      const uint32 vf = (uint32) ((v-vi)*1000);
      put(vi);
      put('.');
      put(vf, 3);  // with 3 zeros
      
      return *this;
    }
    
    Writer& operator<<(double v) {
      if (v < 0) {
        put('-');
        v = -v;
      }
      const uint32 vi = (uint32) v;
      const uint32 vf = (uint32) ((v-vi)*100000);
      put(vi);
      put('.');
      put(vf, 5);  // 5 zeros
      
      return *this;
    }
    
    template<uint32 N>
    Writer& operator<<(const util::Q<N>& q) {
      const uint32 divisor = 1000;
      const int32 f = (q.frac() * divisor).to_int32();
      
      *this << q.to_int32() << "+" << f << "/" << divisor;
      return *this;
    }
    
  private:    
    IO& m_io;
    BlockingMode m_blocking_mode;
  };
}
