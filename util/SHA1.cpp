/*
 *  SHA1.cpp
 *  Embedded
 *
 *  Created by Serge on 25/10/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#include "SHA1.h"
#include "BitOps.h"
#include "Endian.h"
#include "mem.h" // For memset()

namespace util {
  
  SHA1 & SHA1::process(const value_type* data, size_type dataSize)
  {
    size_type dataOffset = 0;
    
    while (dataOffset < dataSize)
    {
      m_chunk_8[m_chunk_offset++] = data[dataOffset++];
      m_input_size ++;
      
      if (m_chunk_offset == CHUNK_SIZE_8)
      {
        process_chunk();
        m_chunk_offset = 0;
      }
    }
    
    return *this;
  }
  
  void SHA1::process_chunk()
  {
    convert_chunk_to_host_endian();
    extend_chunk_from_16_to_80_values();
    calculate_chunk_hash();
  }
  
  SHA1::Result SHA1::get_result()
  {
    //
    // Remember the complete input size (IN BITS)
    //
    const uint64 original_input_size = m_input_size * 8;
    
    /*
     * Here we have to complete the last chunk according to the 
     * SHA1 algorithm, that is,
     * - Add 1 bit (in effect the 0x80 byte)
     * - complete the chunk so that it is congruent with 56 octets (448 bits)
     * - write the original message length as big endian 64-bit quantity
     *
     */
    m_chunk_8[m_chunk_offset++] = 0x80;
    
    /*
     * Pad with zeroes until congruent to 448 bits (56 octets)
     * We may have to add another chunk if we are more than
     * 56 octets into this chunk.
     */
    if (m_chunk_offset > 56)
    {
      //
      // No space to write the input size.
      // Pad with zeroes, calculate hash and then add another full chunk
      //
      while (m_chunk_offset < CHUNK_SIZE_8)
      {
        m_chunk_8[m_chunk_offset++] = 0;
      }	
      
      process_chunk();
      m_chunk_offset = 0;
    }
    
    //
    // Terminal chunk, zeroed and appended with original input size
    //
    const int32 zero_padding = CHUNK_SIZE_8 - m_chunk_offset - 8;
    
    memset(m_chunk_8 + m_chunk_offset, 
           0x0, 
           zero_padding); 
    m_chunk_offset += zero_padding; //  Result is 56.
    
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 56) & 0xff;
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 48) & 0xff;
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 40) & 0xff;
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 32) & 0xff;
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 24) & 0xff;
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 16) & 0xff;
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 8) & 0xff;
    m_chunk_8[m_chunk_offset++] = (original_input_size >> 0) & 0xff;
    
    process_chunk();
    
    return m_state;
  }
  
  void SHA1::convert_chunk_to_host_endian()
  {
    for (uint32 i = 0; i < CHUNK_SIZE_32; ++i)
      m_chunk_32[i] = to_host_order(m_chunk_32[i]);
  }
  
  void SHA1::extend_chunk_from_16_to_80_values()
  {
    //
    // We assume a complete chunk here
    //
    for (uint32 i = CHUNK_SIZE_32; i < EXTENDED_CHUNK_SIZE_32; ++i)
    {
      uint32 tmp = m_chunk_32[i-3] ^ m_chunk_32[i-8] ^ m_chunk_32[i-14] ^ m_chunk_32[i-16];
      m_chunk_32[i] = rol(tmp, 1);
    }
  }
  
  void SHA1::calculate_chunk_hash()
  {
    uint32 a = m_state.h0, 
    b = m_state.h1, 
    c = m_state.h2, 
    d = m_state.h3, 
    e = m_state.h4;
        
    uint32 temp;
    
    for (uint32 i = 0; i < EXTENDED_CHUNK_SIZE_32; ++i)
    {
      uint32 k, f;
      
      if (i < 20)
      {
        f = (b & c) | ((~b) & d);
        k = K0;
      }			
      else if (i < 40)
      {
        f = b ^ c ^ d;
        k = K1;
      }			
      else if (i < 60)
      {
        f = (b & c) | (b & d) | (c & d);
        k = K2;
      }
      else
      {
        f = b ^ c ^ d;
        k = K3;
      }
      
      temp = rol(a, 5);
      temp += f + e + k + m_chunk_32[i];
      e = d;
      d = c;
      c = rol(b, 30);
      b = a;
      a = temp;
    }
    
    m_state.h0 += a;
    m_state.h1 += b;
    m_state.h2 += c;
    m_state.h3 += d;
    m_state.h4 += e;
  }  
}

