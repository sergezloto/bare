/*
 *  FlashWriter.h
 *  Embedded
 *
 *  Created by Serge on 5/9/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "util.h"
#include "nor_types.h"

namespace lpcnor {
  /**
   * A class to write data into lpcnor flash, templated
   * on the particular LPCNOR class instantiation
   */
  template <class T>
  class FlashWriter: public util::Writer {
  public:
    /**
     * Constructor, allocates the given byte size in the flash
     */
    FlashWriter(size_type size)
    : m_temp_ptr(T::data_new(size)) {
    }
    
    /**
     * Write bytes to the medium
     * @return the number of bytes written
     */
    size_type write(const uint8 *bytes, size_type count) {
      const bool ok = T::data_write(m_temp_ptr, bytes, count);
      return ok ? count : 0;
    }
    
    /**
     * Commits the write. The data is not considered written until
     * commit is called and returns a valid flash pointer.
     */
    typename T::ptr_type commit() {
      return T::data_commit(m_temp_ptr);
    }
    
    /**
     * @return true if the writer does not have any failure condition and an operation is ongoing
     */
    bool good() const {
      return !m_temp_ptr.is_blank();
    }
    
#if DEBUG
    const typename T::temp_ptr_type get_ptr() const { return m_temp_ptr; }
    
    typedef typename T::return_code return_code;
    return_code get_last_code() {
      return T::get_last_code();
    }
#endif
  private:
    typename T::temp_ptr_type m_temp_ptr;
  };
}
