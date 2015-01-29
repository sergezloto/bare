/*
 *  mem.h
 *  Embedded
 *
 *  Created by Serge on 6/13/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */
#pragma once

#if __EMBEDDED__
#include "base.h"

#if defined(__cplusplus)
extern "C" {
#  define c_restrict
#else
#  define c_restrict restrict
#endif

#if !defined(_SIZE_T)
  typedef long unsigned int size_t;  
#endif
  void *memset(void *b, int c, size_t n);
  void *memcpy(void *c_restrict s1, const void *c_restrict s2, size_t n);
  void *memmove(void *s1, const void *s2, size_t n);  
  
#if defined(__cplusplus)
}
#endif
#else
#  include <string.h>
#endif
