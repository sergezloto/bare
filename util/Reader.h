/*
 *  Reader.h
 *  Embedded
 *
 *  Created by Serge on 28/01/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

#if !__EMBEDDED__
#  include <sys/types.h>
#endif

namespace util {
  /**
   * An abstract base class for reading byte strings.
   */
  class Reader {
  public:
#if __EMBEDDED__
    typedef uint32 size_type;
#else
    typedef size_t size_type;
#endif
    
    /**
     * This one is to make gcc shut up about non-virtual destructor
     */
    virtual ~Reader() {
    }
    
    /**
     * Receives bytes from the medium
     * @return the number of bytes effectively received
     */
    virtual size_type read(uint8 *bytes, size_type count) = 0;    

    /**
     * Attempts to read the whole data
     * @return the number of bytes read.
     */
    size_type read_all(uint8 *bytes, size_type count) {
      size_type tr = 0;
      while (tr < count) {
        const size_type r = read(bytes + tr, count - tr);
        if (r == 0) {
          break;
        }
        tr += r;
      }
      return tr;
    }
  };  
}
