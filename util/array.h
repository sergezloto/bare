/*
 *  array.h
 *  Embedded
 *
 *  Created by Serge on 2/19/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"
#include "Endian.h"

#if !__EMBEDDED__
#  include <string>
#endif

namespace util {
  /**
   * A generic, fixed size, array container. 
   * The underlying data must support assigment.
   */
  template <typename T, uint32 SIZE>
  class array {
  public:
    typedef array<T, SIZE> this_type;
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;
    
    static const uint32 capacity = SIZE;
    
    static const uint32 npos = ~0u;
    
    /**
     * default constructor
     */
    array() {
    }
    
    /**
     * Copy constructor
     */
    array(const this_type& other) {
      for (uint32 i = 0; i < capacity; ++i)
        m_data[i] = other.m_data[i];
    }
    
    /**
     * Constructor from a pointer.
     * At most capacity items are copied
     */
    explicit array(const T* ptr, uint32 nb_items) {
      uint32 i;
      for (i = 0; i < capacity && i < nb_items; ++i)
        m_data[i] = ptr[i];
    }
    
    /**
     * Constructor from a C string.
     * At most capacity items are copied
     */
    explicit array(const char* s) {
      uint32 i;
      for (i = 0; i < capacity && s[i] != 0 && i < npos; ++i)
        m_data[i] = static_cast<value_type>(s[i]);
      
      // Add a '\0' if there is space
      if (i < capacity)
        m_data[i] = static_cast<value_type>('\0');
    }
    
    /**
     * Assignment
     */
    this_type& operator=(const this_type& other) {
      for (uint32 i = 0; i < capacity; ++i)
        m_data[i] = other.m_data[i];
      return *this;
    }
    
    /**
     * Assignment from a string, if its type is convertible to T.
     * Copies up to capacity items, or until U(0) is reached.
     */
    this_type& operator=(const char* s) {
      uint32 i;
      for (i = 0; i < capacity && s[i] != 0; ++i) {
        m_data[i] = static_cast<value_type>(s[i]);
      }
      // Add a '\0' if there is space
      if (i < capacity)
        m_data[i] = static_cast<value_type>('\0');
    }
    
    /**
     * Subscript operator
     */
    const T& operator[](uint32 i) const {
      return m_data[i];
    }
    
    /**
     * Mutable subscript operator
     */
    T& operator[](uint32 i) {
      return m_data[i];
    }
    
#if !__EMBEDDED__
    /**
     * If value_type is convertible to char, returns
     * a standard string of size capacity
     */
    std::string str() const {
      return std::string(reinterpret_cast<const char*>(m_data), capacity * sizeof(value_type));
    }
#endif
    
    /**
     * @return the contents of the array as a C string pointer
     */
    const char* c_str() const {
      return reinterpret_cast<const char*> (m_data);
    }
    
    /**
     * Direct data access
     */
    const T* get() const {
      return m_data;
    }
    
    /**
     * Mutable direct access
     */
    T* get() {
      return m_data;
    }
    
    uint32 size() const {
      return capacity;
    }
    
    /**
     * STL style Iterator
     */
    iterator begin() {
      return &m_data[0];
    }
    
    iterator end() {
      return &m_data[capacity];
    }
    
    const_iterator begin() const {
      return &m_data[0];
    }
    
    const_iterator end() const {
      return &m_data[capacity];
    }
    
  private:
    T m_data[SIZE];
  };
  
  //
  // Endian conversions
  //
  template <uint32 SIZE>
  inline
  array<uint8,SIZE>& to_little_endian(array<uint8,SIZE>& b) {
    return b;
  }
  
  template <uint32 SIZE>
  inline
  array<uint8,SIZE>& to_big_endian(array<uint8,SIZE>& b) {
    return b;
  }
  
  template <uint32 SIZE>
  inline
  array<char,SIZE>& to_little_endian(array<char,SIZE>& b) {
    return b;
  }
  
  template <uint32 SIZE>
  inline
  array<char,SIZE>& to_big_endian(array<char,SIZE>& b) {
    return b;
  }

  template <typename T, uint32 SIZE>
  inline
  array<T,SIZE> to_little_endian(array<T,SIZE>& b) {
    array<T, SIZE> ret;
    for (uint32 i = 0; i < array<T, SIZE>::capacity; ++i) {
      ret[i] = to_little_endian(b[i]);
    }
    return ret;
  }
  
  template <typename T, uint32 SIZE>
  inline
  array<T,SIZE> to_big_endian(array<T,SIZE>& b) {
    array<T, SIZE> ret;
    for (uint32 i = 0; i < array<T, SIZE>::capacity; ++i) {
      ret[i] = to_big_endian(b[i]);
    }
    return ret;
  }
}

