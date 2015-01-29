/*
 *  HMAC.h
 *  Embedded
 *
 *  Created by Serge on 25/11/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  
  /**
   * HMAC calculator, templated on a hash function
   */
  template <class H>
  class HMAC: NoCopy {
  public:
    typedef HMAC<H> this_type;
    typedef typename H::size_type size_type;
    typedef typename H::value_type value_type;
    typedef typename H::result_type result_type;
    
    enum {
      BIT_BLOCK_SIZE = H::BIT_BLOCK_SIZE,
      BYTE_BLOCK_SIZE = H::BYTE_BLOCK_SIZE,
      BIT_DIGEST_SIZE = H::BIT_DIGEST_SIZE,
      BYTE_DIGEST_SIZE = H::BYTE_DIGEST_SIZE
    };
    
    /**
     * Constructor
     */
    HMAC() {
    }
    
    /**
     * Sets the key for calculating the hmac.
     * This must be done for every new hmac calculation
     */
    this_type& set_key(const value_type* key, size_type key_size);
    
    /**
     * Process an additional block of data
     */
    this_type& process(const value_type * data, size_type data_size);
    
    /**
     * @return the result for the hmac
     */
    result_type get_result();

    /**
     * Forgets any ongoing operation
     */
    this_type& reset() {
      m_h.reset();
      return *this;
    }
    
  private:
    static const uint8 PAD_IN = 0x36;
    static const uint8 PAD_OUT = 0x5c;
    H m_h;
    value_type m_key[BYTE_BLOCK_SIZE];
  };

  template <class H>
  typename HMAC<H>::this_type& HMAC<H>::set_key(const value_type* key, size_type key_size) {
    if (key_size > BYTE_BLOCK_SIZE) {
      m_h.reset();
      m_h.process(key, key_size);
      const result_type result = m_h.get_result();
      for (size_type i = 0; i < BYTE_DIGEST_SIZE; ++i) {
        m_key[i] = result[i];
      }
      key_size = BYTE_DIGEST_SIZE;
      // Done with hashing the key
      m_h.reset();
    } else {
      for (size_type i = 0; i < key_size; ++i) {
        m_key[i] = key[i];
      }
    }
    
    // When the key does not fill a whole block, fill with zeroes
    for (size_type i = key_size; i < BYTE_BLOCK_SIZE; ++i) {
      m_key[i] = 0;
    }
    
    // Perform inner padding
    // We convert the key to inner padding
    for (size_type i = 0; i < BYTE_BLOCK_SIZE; ++i) {
      m_key[i] ^= PAD_IN;
    }
    m_h.process(m_key, BYTE_BLOCK_SIZE);
    
    // Prepare for outer padding, as it will be needed at the end of the message to hash
    for (size_type i = 0; i < BYTE_BLOCK_SIZE; ++i) {
      // Reverse inner padding xor, and do outer padding at the same time
      m_key[i] ^= PAD_IN ^ PAD_OUT;
    }
    
    return *this;
  }
  
  template <class H>
  typename HMAC<H>::this_type& HMAC<H>::process(const value_type * data, size_type data_size) {
    // Process this data chunk
    m_h.process(data, data_size);
    
    return *this;
  }
  
  template <class H>
  typename HMAC<H>::result_type HMAC<H>::get_result() {
    // Get inner result
    result_type inner_result = m_h.get_result();
    
    // Reuse the hasher to perform the outer hash
    m_h.reset();
    
    // Do the outer padding
    m_h.process(m_key, BYTE_BLOCK_SIZE);
    
    // Add the inner hash in big-endian
    inner_result.to_network_order();
    m_h.process(inner_result.get(), BYTE_DIGEST_SIZE);
    
    // We are done
    return m_h.get_result();
  }
}
