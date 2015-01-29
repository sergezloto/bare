/*
 *  MovingAverage.h
 *  Embedded
 *
 *  Created by Serge on 1/11/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  
  template <typename T, uint32 MAX_CAPACITY>
  class MovingAverage {
  public:
    explicit MovingAverage(uint32 capacity = MAX_CAPACITY)
    : m_capacity(capacity) {
      reset();
    }
    
    /**
     * @return the set capacity
     */
    uint32 get_capacity() const {
      return m_capacity;
    }
    
    /**
     * @return the number of stored values
     */
    uint32 get_size() const {
      return m_capacity - m_empty_slots;
    }
    
    /**
     * @return true if values have been loaded up to the capacity
     */
    bool is_full() const {
      return m_empty_slots == 0;
    }
    
    /**
     * @return true if no values have been loaded
     */
    bool is_empty() const {
      return m_empty_slots == m_capacity;
    }
    
    /**
     * Adds a value
     */
    void add_value(const T value) {
      m_values[m_put_index++] = value;
      
      if (m_put_index == m_capacity)
        m_put_index = 0;
      if (m_empty_slots > 0)
        --m_empty_slots;
    }
    
    /**
     * Obtains the average of the stored samples
     */
    template <typename V>
    V get_average() const {
      V temp = V(0);
      
      const uint32 size = get_size();
      for (uint32 i = 0; i < size; ++i) {
        temp += m_values[i];
      }
      return temp / size;
    }
    
    /**
     * reset state save the capacity
     */
    void reset() {
      m_empty_slots = m_capacity;
      m_put_index = 0;
    }
    
  private:
    uint32 m_capacity;
    uint32 m_empty_slots;
    uint32 m_put_index;
    T m_values[MAX_CAPACITY];
  };
}
