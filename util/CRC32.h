/*
 *  CRC32.h
 *  Embedded
 *
 *  Created by Serge on 6/02/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once
#include "base.h"

#define CRC32_NO_TABLE

namespace util {
  class CRC32 {
  public:
    typedef uint32 size_type;
    typedef uint8 value_type;
    
    typedef uint32 result_type;
    
    CRC32() {
      reset();
    }
    
#if defined(CRC32_NO_TABLE)
    CRC32& process(const value_type * data, size_type data_size)
    {
      while (data_size--) 
      {
        m_temp = (uint32)((m_crc & 0xff) ^ *data++);
        for (uint32 j = 0; j < 8; j++) 
        {
          if (m_temp & 0x1)
            m_temp = (m_temp >> 1) ^ 0xedb88320;
          else
            m_temp >>= 1;
        }
        m_crc = (m_crc >> 8) ^ m_temp;
      }
      return *this;
    }
    
#else
    /**
     * Process an additional block of data
     */
    CRC32& process(const value_type * data, size_type data_size) {
      for (uint32 i = 0; i < data_size; ++i) {
        m_crc = (m_crc >> 8) ^ m_lut[(m_crc & 0xff) ^ data[i]];
      }
      return *this;
    }
#endif

    /**
     * @return the result for the hash
     */
    result_type get_result() {
      return ~m_crc;
    }
    
    /**
     * Resets the state of the crc calculator
     */
    void reset()
    {
#if defined(CRC32_NO_TABLE)
      m_temp = 0;
#endif
      m_crc = 0xffffffff;
    }
    
  private:
    result_type m_crc;
    
#if defined(CRC32_NO_TABLE)
    uint32 m_temp;
#else
    static const uint32 m_lut[256];
#endif
  };
}
