/*
 *  uuid.h
 *  Embedded
 *
 *  Created by Serge on 26/11/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "array.h"
#include "BinHex.h"

namespace util {
  class Uuid {
  public:
    enum {
      CAPACITY = 16
    };
    typedef Uuid this_type;
    typedef uint8 value_type;
    typedef uint32 size_type;
    typedef array<value_type, CAPACITY> storage_type;
    
    explicit Uuid() {
    }
    
    explicit Uuid(const array<value_type, CAPACITY>& bytes)
    : m_bytes(bytes) {
    }
    
    explicit Uuid(const value_type* bytes) {
      assign(bytes);
    }
    
    value_type operator[](size_type i) const {
      return m_bytes[i];
    }
    
    value_type& operator[](size_type i) {
      return m_bytes[i];
    }
    
    const value_type* get() const {
      return m_bytes.get();
    }
    
    size_type size() const {
      return m_bytes.size();
    }
    
    /**
     * @param bytes must be a pointer to 16 bytes
     */
    this_type& assign(const value_type* bytes) {
      for (size_type i = 0; i < CAPACITY; ++i) {
        m_bytes[i] = bytes[i];
      }
      return *this;
    }
    
    bool assign(const char* hex_string) {
      return hex_to_bytes(m_bytes.get(), hex_string, m_bytes.size()) == m_bytes.size();
    }

  private:
    storage_type m_bytes;
  };
  
  inline
  bool operator==(const Uuid& a, const Uuid& b) {
    for (uint32 i = 0; i < Uuid::CAPACITY; ++i) {
      if (a[i] != b[i])
        return false;
    }
    return true;
  }

  inline
  bool operator!=(const Uuid& a, const Uuid& b) {
    return ! (a == b);
  }
  
}
