/*
 *  AES256_wrapper.h
 *  Embedded
 *
 *  Created by Serge on 26/11/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "aes256.h"

namespace util {
  /**
   * Implements the AES-256 cipher.
   * Each block is encrypted in-place.
   * It always operates in CBC mode
   */
  class AES256: NoCopy {
  public:
    /**
     * Block cipher block size
     */
    enum {
      BYTE_KEY_SIZE = 32,
      BYTE_BLOCK_SIZE = 16
    };
    
    typedef AES256 this_type;
    typedef uint8 value_type;
    typedef uint32 size_type;
    typedef value_type block_type[BYTE_BLOCK_SIZE];
    typedef value_type key_type[BYTE_KEY_SIZE];
    
    this_type& reset() {
      for (size_type i = 0 ; i < BYTE_BLOCK_SIZE; ++i)
        m_init_vector[i] = 0;
      return *this;
    }
    
    /**
     * Sets the key used to encrypt the data. The key size must be 256 bits, that is, 32 bytes long.
     */
    this_type& set_key(const key_type& key) {
      reset();
      aes256_init(&m_ctx, key);
      return *this;
    }
    
    /**
     * Encrypts one block of data.
     * The data must be passed in 16 bytes chunks.
     */
    this_type& encrypt(value_type* block) {
      for (size_type i = 0 ; i < BYTE_BLOCK_SIZE; ++i)
        block[i] ^= m_init_vector[i];

      aes256_encrypt_ecb(&m_ctx, block);

      for (size_type i = 0 ; i < BYTE_BLOCK_SIZE; ++i)
        m_init_vector[i] = block[i];
      return *this;
    }
    
    /**
     * Decrypts one block of data.
     * The data must be passed in 16 bytes chunks.
     */
    this_type& decrypt(value_type* block) {
      value_type current_ciphertext[BYTE_BLOCK_SIZE];
      for (size_type i = 0 ; i < BYTE_BLOCK_SIZE; ++i) {
        // Save ciphertext. We'll use it in the next block decrypt operation
        current_ciphertext[i] = block[i];
      }

      aes256_decrypt_ecb(&m_ctx, block);
            
      for (size_type i = 0 ; i < BYTE_BLOCK_SIZE; ++i) {
        // Undo block chaining
        block[i] ^= m_init_vector[i];
        // Current ciphertext used for next block chaining
        m_init_vector[i] = current_ciphertext[i];
      }
      return *this;
    }
        
    /**
     * Encrypts several blocks
     */
    this_type& encrypt(value_type* blocks, size_type nblocks) {
      while (nblocks) {
        encrypt(blocks);
        blocks += BYTE_BLOCK_SIZE;
        --nblocks;
      }
      return *this;
    }

    /**
     * Decrypts several blocks
     */
    this_type& decrypt(value_type* blocks, size_type nblocks) {
      while (nblocks) {
        decrypt(blocks);
        blocks += BYTE_BLOCK_SIZE;
        --nblocks;
      }
      return *this;
    }

  private:
    aes256_context m_ctx;
    block_type m_init_vector;
  };
}
