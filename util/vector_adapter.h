/*
 *  vector_adapter.h
 *  Embedded
 *
 *  Created by Serge on 6/20/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#include "base.h"

namespace util {
  template <typename T = uint8>
  class vector_adapter {
  public:
    typedef vector_adapter<T> this_type;
    vector_adapter(T* data, uint32 size_i)
    : m_data(data), m_size(size_i) {
    }
    ~vector_adapter() {
    }
    
    T operator[](uint32 i) const {
      return m_data[i];
    }
    
    const T* get_ptr() const {
      return m_data;
    }
    
    uint32 size() const {
      return m_size;
    }
    
    this_type& operator=(const this_type& v) {
      m_data = v.m_data;
      m_size = v.m_size;
      return *this;
    }
  private:
    T* m_data;
    uint32 m_size;
  };
}
