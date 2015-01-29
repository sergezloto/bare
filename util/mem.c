/*
 *  mem.c
 *  Embedded
 *
 *  Created by Serge on 6/13/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */
#if __EMBEDDED__
#  include "mem.h"

#  if defined(__cplusplus)
extern "C" {
#  endif
    
  /*
   * memxxx functions
   */
  void *memset(void *b, int c, size_t n) {
    char *p = (char*)b;
    for (int i = 0; i < n; ++i) {
      p[i] = c;
    }
    return b;    
  }
  
  void *memcpy(void *c_restrict s1, const void *c_restrict s2, size_t n) {
    const uint8 *src = (uint8*) s2;
    uint8 *dest = (uint8*) s1;
    // TODO: Optimize!
    while (n-- > 0) {
      *dest = *src;
      dest++;
      src++;
    }
    return s1;
  }
  
  void *memmove(void *s1, const void *s2, size_t n) {
    if (s1 == s2)
      return s1;
    
    const uint8 *src = (uint8*) s2;
    uint8 *dest = (uint8*) s1;
    // TODO: Optimize!
    while (n-- > 0) {
      *dest = *src;
      dest++;
      src++;
    }    
    return s1;
  }
  
#  if defined(__cplusplus)
}
#  endif
#endif
