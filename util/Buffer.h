/*
 *  Buffer.h
 *  Embedded
 *
 *  Created by Serge on 6/2/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  
  /**
   * A circular buffer, templated on the object type and capacity.
   * It allocates capacity + 1 elements to distinguish between the empty
   * and full conditions.
   * The user is expected to check that the buffer is not empty or not full
   * before calling get() or put(), respectively.
   */
  template <typename T, uint32 N>
  class Buffer {
  public:
    typedef Buffer<T, N> type;
    typedef T value_type;
    typedef uint32 size_type;
    
    static const uint32 capacity = N;
    
    Buffer() : m_put(0), m_get(0) {
    }

    explicit Buffer(const type& buffer);

    uint32 size() volatile const {
      if (m_put >= m_get)
        return m_put - m_get;
      else
        return capacity + m_put - m_get + 1;
    }
    
    uint32 available() volatile const {
      if (m_put >= m_get)
        return capacity - m_put + m_get;
      else
        return m_get - m_put - 1;
    }
    
    bool is_full() volatile const {
      return (m_get == 1 + m_put) || (m_get == 0 && m_put == N);
    }
    
    bool is_empty() volatile const {
      return m_put == m_get;
    }
    
    void put(T value) volatile {
      uint32 p = m_put;
      m_buf[p++] = value;
      if (p == N+1)
        p = 0;
      m_put = p;
    }
    
    T peek() volatile const {
      const T value = m_buf[m_get];
      return value;      
    }
    
    T get() volatile {
      uint32 g = m_get;
      const T value = m_buf[g++];
      if (g == N+1)
        g = 0;
      m_get = g;
      return value;
    }
    
    void reset() volatile {
      m_put = m_get = 0;
    }
    
  private:
    T m_buf[N + 1];
    size_type m_put, m_get;
    
  };
}
