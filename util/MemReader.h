/*
 *  MemReader.h
 *  Embedded
 *
 *  Created by Serge on 5/7/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "Reader.h"

namespace util {
  /**
   * A memory reader.
   * As no copy of the memory is made, the memory lifetime must exceed
   * that of this object.
   */
  class MemReader: public Reader {
  public:
    /**
     * Constructor
     */
    MemReader(const uint8* memory, size_type size)
    : m_memory(memory), m_size(size), m_pos(0) {
    }
    
    /**
     * This one is to make gcc shut up about non-virtual destructor
     */
    ~MemReader() {
    }
    
    /**
     * Receives bytes from the medium
     * @return the number of bytes effectively received
     */
    size_type read(uint8 *bytes, size_type count) {
      uint32 nb = 0;
      while (m_pos < m_size && count > 0) {
        *bytes++ = m_memory[m_pos++];
        --count;
        ++nb;
      }
      return nb;
    }

  private:
    const uint8* const m_memory;
    const size_type m_size;
    size_type m_pos;
  };  
}

