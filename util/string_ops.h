/*
 *  string.h
 *  Embedded
 *
 *  Created by Serge on 8/10/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace util {
  template <typename CHARD, typename CHARS>
  void strcpy(CHARD dst, const CHARS src) {
    int i = 0;
    while ( src[i] ) {
      dst[i] = src[i];
      ++i;
    }
    dst[i] = 0;
  }
  
  template <typename CHAR>
  uint32 strlen(CHAR s[]) {
    uint32 l = 0;
    while (s[l] != '\0')
      l++;
    return l;
  }
}
