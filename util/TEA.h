/*
 *  TEA.h
 *  Embedded
 *
 *  Created by Serge on 2/18/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"

namespace util {  
  /**
   * XXTEA as picked from http://en.wikipedia.org/wiki/XXTEA
   * This implementation does not operate in block-chaining mode
   */
  class XXTEA: NoInstance {
  public:
    
    /**
     * Encrypts a data block of variable length, using a 4 32-bit key
     */
    static void encrypt(int32 *block, uint32 size, const int32 key[4]) {
      btea(block, size, key);
    }

    /**
     * Decrypts a data block of variable length, using a 4 32-bit key
     */
    static void decrypt(int32 *block, uint32 size, const int32 key[4]) {
      btea(block, -size, key);
    }

    /**
     * Encrypts a data block of variable length, using a 4 32-bit key
     */
    static void encrypt(uint8 *block, uint32 size, const uint8 key[16]) {
      btea(reinterpret_cast<int32*>(block), size / sizeof(int32), reinterpret_cast<const int32*>(key));
    }
    
    /**
     * Decrypts a data block of variable length, using a 4 32-bit key
     */
    static void decrypt(uint8 *block, uint32 size, const uint8 key[16]) {
      btea(reinterpret_cast<int32*>(block), -(size / sizeof(int32)), reinterpret_cast<const int32*>(key));
    }

  private:
    /**
     * According to Needham and Wheeler:
     * teab will encode or decode n words as a single block where n > 1
     * v is the n word data vector
     * k is the 4 word key
     * n is negative for decoding
     * if n is zero result is 1 and no coding or decoding takes place, otherwise the result is zero
     * assumes 32 bit ‘long’ and same endian coding and decoding
     */
    static int32 btea(int32* v, int32 n, const int32* k) {
#define XXTEA_MX (z>>5^y<<2) + (y>>3^z<<4)^(sum^y) + (k[p&3^e]^z);
      uint32 z=v[n-1], y=v[0], sum=0, e;
      const uint32 DELTA=0x9e3779b9;
      long p, q ;
      if (n > 1) {          /* Coding Part */
        q = 6 + 52/n;
        while (q-- > 0) {
          sum += DELTA;
          e = (sum >> 2) & 3;
          for (p=0; p<n-1; p++) y = v[p+1], z = v[p] += XXTEA_MX;
          y = v[0];
          z = v[n-1] += XXTEA_MX;
        }
        return 0 ; 
      } else if (n < -1) {  /* Decoding Part */
        n = -n;
        q = 6 + 52/n;
        sum = q*DELTA ;
        while (sum != 0) {
          e = (sum >> 2) & 3;
          for (p=n-1; p>0; p--) z = v[p-1], y = v[p] -= XXTEA_MX;
          z = v[n-1];
          y = v[0] -= XXTEA_MX;
          sum -= DELTA;
        }
        return 0;
      }
      return 1;
    }
  };
}
