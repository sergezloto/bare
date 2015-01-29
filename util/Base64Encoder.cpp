/*
 *  Base64Encoder.cpp
 *  Embedded
 *
 *  Created by Serge on 18/12/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#include "Base64Encoder.h"

using util::Base64Encoder;

const Base64Encoder::out_value_type Base64Encoder::t64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

Base64Encoder::size_type Base64Encoder::write(const in_value_type* b, size_type size) {
  size_type nb = 0;
  while (size--) {
    // We hold no more than 3 values
    if (m_mod3 >= 3) {
      return nb;
    }

    switch (m_mod3) {
      case 0:
      case 1:
        m_b[m_mod3] = b[nb];
        break;
      case 2:
      {
        m_c[0] = t64[m_b[0] >> 2];
        m_c[1] = t64[(m_b[0] & 0x3) << 4 | (m_b[1] >> 4)];
        m_c[2] = t64[(m_b[1] & 0xf) << 2 | (b[nb] >> 6)];
        m_c[3] = t64[b[nb] & 0x3f];
      }
        break;
    }
    // Next input byte
    ++m_mod3;
    // Next data
    ++nb;
  }
  return nb;
}

Base64Encoder::size_type Base64Encoder::read(out_value_type* c, size_type size) {
  // Limit the number of chars we output
  if (size > 4)
    size = 4;
  
  size_type nb = 0;
  
  while (size--) {
    switch (m_mod3) {
      case 0 ... 2:
        // Nothing to read when we don't have a full buffer
        return 0;

      case 3 ... 6:
        // We have at most 4 bytes we can read
        c[nb++] = m_c[m_mod3++ - 3];
        break;
        
      default:
        // We have written all our characters out, reset input counter
        m_mod3 = 0;
        goto done;
    }
  }
done:
  return nb;
}

Base64Encoder::out_value_type Base64Encoder::read_end() {
  out_value_type ret;
  switch (m_mod3) {
    case 0:
      // We are sweet
      ret = '\x00';
      break;
    case 1:
      ret = t64[m_b[0] >> 2];
      m_c[1] = t64[(m_b[0] & 0x3) << 4];
      m_c[2] = '=';
      m_c[3] = '=';
      m_mod3 = 4;
      break;
    case 2:
      ret = t64[m_b[0] >> 2];
      m_c[1] = t64[(m_b[0] & 0x3) << 4 | (m_b[1] >> 4)];
      m_c[2] = t64[(m_b[1] & 0xf) << 2];
      m_c[3] = '=';
      m_mod3 = 4;
      break;
      
    case 3 ... 6:
      ret = m_c[m_mod3++ - 3];
      break;
      
    default:
      ret = '\x00';
      break;
  }
  
  return ret;
}

void Base64Encoder::process(const in_value_type* area, size_type in_size, out_value_type* dest) {
  Base64Encoder e64;
  const uint32 NR = 4;
  uint32 ti = 0;
  uint32 to = 0;
  
  while (in_size) {
    const uint32 i = e64.write(area + ti, in_size);
    ti += i;
    in_size -= i;
    
    const uint32 o = e64.read(dest + to, NR);
    to += o;
  }
  // TODO: GET REMAINING BYTES AND PADDING
  out_value_type c;
  while ((c = e64.read_end()) != '\x00') {
    dest[to++] = c;
  }
}
