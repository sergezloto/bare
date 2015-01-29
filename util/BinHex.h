/*
 *  BinHex.h
 *  Embedded
 *
 *  Created by Serge on 2/12/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */
#pragma once
#include "base.h"

namespace util {
  /**
   * Converts a hex string to binary, ignoring non-hex digits
   * @param bytes must point to an area big enough to hold max_bytes
   * @param input hex string, processed until max_bytes are returned or '\000' is reached
   * @return the number of bytes processed
   */
  inline
  uint32 hex_to_bytes(uint8* bytes, const char* input, uint32 max_bytes) {
    uint32 nbytes = 0;
    uint8 ch;
    while (nbytes < max_bytes) {
      uint8 value = 0;
      if (*input == 0 || *(input+1) == 0)
        break;
      switch (ch = *input++) {
        case '0' ... '9':
          value = ch - '0';
          break;
        case 'a' ... 'f':
          value = ch - 'a' + 10;
          break;
        case 'A' ... 'F':
          value = ch - 'A' + 10;
          break;
        default:
          continue;
      }
      value <<= 4;
      switch (ch = *input++) {
        case '0' ... '9':
          value |= ch - '0';
          break;
        case 'a' ... 'f':
          value |= ch - 'a' + 10;
          break;
        case 'A' ... 'F':
          value |= ch - 'A' + 10;
          break;
        default:
          continue;
      }
      
      bytes[nbytes++] = value;
    }
    return nbytes;
  }
}
