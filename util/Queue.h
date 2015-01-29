/*
 *  Queue.h
 *  Embedded
 *
 *  Created by Serge on 03/07/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  
  /**
   * Fixed capacity queue.
   * The implementation uses a circular buffer, which wastes one position for its
   * empty marker. No synchronization is necessary when there is one producer and 
   * one consumer thread.
   */
  template <typename T = uint8, uint32 CAPACITY = 64 -1>
  class  Queue {
public:
    typedef T value_type;
    static const uint32 capacity = CAPACITY + 1;
    
    Queue()
      : m_put(0), m_get(0) {}
    
    bool put(T value) {
      if (isFull())
        return false;
      
      // We enqueue at the tail
      m_content[m_put++] = value;
      if (m_put == capacity)
        m_put = 0;
						
      return true;
    }
    
    bool get(T& value) {
      if (isEmpty())
        return false;
      
      // We get from the head
      value = m_content[m_get++];
      if (m_get == capacity)
        m_get = 0;
      return true;
    }
    
    bool isEmpty() const {
      return m_put == m_get;
    }
    
    bool isFull() const {
      return size() == capacity - 1;
    }
    
    uint32 size() const {
      if (m_put >= m_get)
        return m_put - m_get;
      return m_get - m_put - 1;
    }
    
private:
      uint32 m_put;
    uint32 m_get;
    T m_content[capacity];
  };
}
