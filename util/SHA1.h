/*
 *  Sha1.h
 *  Embedded
 *
 *  Created by Serge on 25/10/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "Endian.h"
#include "BinHex.h"

namespace util {
  /**
   * A class whose objects calculate a Sha1 hash
   */
  class SHA1: NoCopy {
  public:
    typedef uint32 size_type;
    typedef uint8 value_type;

    enum {
      BIT_BLOCK_SIZE = 512,
      BYTE_BLOCK_SIZE = BIT_BLOCK_SIZE / 8,
      BIT_DIGEST_SIZE = 160,
      BYTE_DIGEST_SIZE = BIT_DIGEST_SIZE / 8
    };
    
    /**
     * The result, in host endian order
     */
    union Result
    {
      typedef SHA1::size_type size_type;
      typedef SHA1::value_type value_type;
      Result() {
      }
      
      explicit Result(const uint8* bytes_20) {
        for (uint32 i = 0; i < BYTE_DIGEST_SIZE; ++i) {
          h[i] = bytes_20[i];
        }
      }
      
      bool assign(const char* hex_string) {
        return hex_to_bytes(h, hex_string, sizeof h) == sizeof h;
      }
      
      bool matches(const Result& o) const {
        return h0 == o.h0 && h1 == o.h1 && h2 == o.h2 && h3 == o.h3 && h4 == o.h4; 
      }
      
      void to_host_order() {
        h0 = util::to_host_order(h0);
        h1 = util::to_host_order(h1);
        h2 = util::to_host_order(h2);
        h3 = util::to_host_order(h3);
        h4 = util::to_host_order(h4);
      }
      
      void to_network_order() {
        h0 = util::to_network_order(h0);
        h1 = util::to_network_order(h1);
        h2 = util::to_network_order(h2);
        h3 = util::to_network_order(h3);
        h4 = util::to_network_order(h4);
      }
      
      const uint8* get() const {
        return h;
      }
      
      uint8 operator[](uint32 i) const {
        return h[i];
      }
      
      uint8* get() {
        return h;
      }
      
      uint8& operator[](uint32 i) {
        return h[i];
      }
      
      size_type size() const {
        return BYTE_DIGEST_SIZE;
      }
      
      struct {
        uint32 h0;
        uint32 h1;
        uint32 h2;
        uint32 h3;
        uint32 h4;
      };
      uint8 h[BYTE_DIGEST_SIZE];
    };
    
    typedef Result result_type;
    
    SHA1() {
      reset();
    }
    
    /**
     * Process an additional block of data
     */
    SHA1& process(const value_type * data, size_type data_size);
    
    /**
     * @return the result for the hash
     */
    Result get_result();
    
    /**
     * Resets the state of the Sha1 calculator
     */
    void reset()
    {
      m_input_size = 0;
      m_chunk_offset = 0;
      
      m_state.h0 = H0;
      m_state.h1 = H1;
      m_state.h2 = H2;
      m_state.h3 = H3;
      m_state.h4 = H4;
    }
    
    /**
     * @returns the number of bytes processed so far
     */
    size_type get_input_size() const {
      return m_input_size;
    }
    
  private:
    static const uint32 
    H0 = 0x67452301, 
    H1 = 0xEFCDAB89, 
    H2 = 0x98BADCFE, 
    H3 = 0x10325476, 
    H4 = 0xC3D2E1F0;
    
    static const uint32 K0 = 0x5A827999, 
    K1 = 0x6ED9EBA1,
    K2 = 0x8F1BBCDC,
    K3 = 0xCA62C1D6;
    
    static const uint32 CHUNK_SIZE_32 = 16;
    static const uint32 EXTENDED_CHUNK_SIZE_32 = 80;
    static const uint32 CHUNK_SIZE_8 = CHUNK_SIZE_32 * 4;
    
    /**
     * process_chunk assumes we have a complete chunk in m_chunk
     */
    void process_chunk();
    void convert_chunk_to_host_endian();
    void extend_chunk_from_16_to_80_values();
    void calculate_chunk_hash();
    
    size_type m_input_size;
    uint32 m_chunk_offset;
    
    Result m_state;
    
    union
    {
      uint32 m_chunk_32[EXTENDED_CHUNK_SIZE_32];
      value_type m_chunk_8[CHUNK_SIZE_8];
    };
  };
}
