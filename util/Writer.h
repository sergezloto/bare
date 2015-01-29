/*
 *  Writer.h
 *  Embedded
 *
 *  Created by Serge on 28/01/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"

namespace util {
  /**
   * An abstract base class for sending byte strings
   */
  class Writer {
  public:
    typedef uint32 size_type;

    /**
     * This one is to make gcc shut up about non-virtual destructor
     */
    virtual ~Writer() {
    }
    
    /**
     * Write bytes to the medium
     * @return the number of bytes effectively sent
     */
    virtual size_type write(const uint8 *bytes, size_type count) = 0;
    
    /**
     * Attempts to write the whole data
     * @return false if unsuccessfull
     */
    size_type write_all(const uint8 *bytes, size_type count) {
      size_type b = 0;
      while (b < count) {
        const size_type w = write(bytes, count);
        if (w == 0) {
          return b;
        }
        bytes += w;
        b += w;
      }
      return b;
    }
  };
}

