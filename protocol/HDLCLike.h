/*
 *  HDLCLike.h
 *  WX
 *
 *  Created by Serge on 5/11/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#define DEBUG_THIS_FILE 0

#include "base.h"
#include "util.h"

#if DEBUG && DEBUG_THIS_FILE
#  include <iostream>
#  include <iomanip>
#endif

namespace protocol {
  
  /**
   * A base class for HDLC-like framing (see http://tools.ietf.org/html/rfc1662 )
   */
  template <uint32 CAPACITY> 
  class HDLCLike: NoCopy {
  
public:
  static const uint32 MIN_FRAME_SIZE = 4;  // 4 bytes for the CRC32
  
protected:
  static uint32 capacity() {
    return CAPACITY;
  }
  
  uint32 frame_size() const {
    return m_size;
  }
  
  uint32 size() const {
    if (frame_size() <= MIN_FRAME_SIZE)
      return 0;
    return frame_size() - MIN_FRAME_SIZE;
  }

  const uint8* get_frame() const {
    return m_frame;
  }
  
  uint8* get_frame() {
    return m_frame;
  }
  
  uint8 get_frame(uint32 i) const {
    return m_frame[i];
  }
  
  /**
   * The FLAG delimits frames, and never appears in the stream.
   * An ESC byte make the next byte in the stream be xored with 0x20 as the received byte value.
   * Unlike the spec, we use 'A' as our flag, because it also is the auto bauding synchronization.
   */
  static const uint8 FLAG = 'A', ESC = 0x7d, XOR = 0x20;
  
  HDLCLike(): m_size(0) {
  }
  
  bool add_byte(uint8 b) {
#if DEBUG && DEBUG_THIS_FILE
    std::cerr << "Adding frame byte:" << std::hex << (uint32)b << " '" << (char)b << "'" << std::endl;
#endif
    
    if (m_size >= CAPACITY)
      return false;
    m_frame[m_size++] = b;
    return true;
  }
  
  void reset() {
    m_size = 0;
  }
  
private:
  uint8 m_frame[CAPACITY];
  uint32 m_size;
  
  // Make sure CAPACITY is at least one!
  typedef uint8 xxyyxxz[CAPACITY - MIN_FRAME_SIZE];
};

}
