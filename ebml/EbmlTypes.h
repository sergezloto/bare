/*
 *  EbmlTypes.h
 *  Embedded
 *
 *  Created by Serge on 28/01/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "util.h"
#if !__EMBEDDED__
#  include <sys/types.h>
#  include <string>
#endif
namespace ebml {
  /**
   * Ids are given with leading zeroes as 
   * as per http://ebml.sourceforge.net/specs/
   * Note: Matroska CRC as per http://www.matroska.org/technical/specs/rfc/index.html is labeled CRC32{Container,Value}
   */
  typedef uint32 id_type;
  
  /**
   * Well known ebml ids
   */
  enum {
    ID_EBML = 0x1A45DFA3,
    ID_EBMLVersion = 0x4286,
    ID_EBMLReadVersion = 0x42f7,
    ID_EBMLMaxIDLength = 0x42f2,
    ID_EBMLMaxSizeLength = 0x42f3,
    ID_DocType = 0x4282,
    ID_DocTypeVersion = 0x4287,
    ID_DocTypeReadVersion = 0x4285,
    ID_CRC32 = 0xbf,		// as per ebml spec, CRC32 for all elements so far, from last CRC32 or master
    ID_CRC32Container = 0xc3,	// as per Matroska spec, it is a master element
    ID_CRC32Value = 0x42fe,	// as per Matroska spec, it is the CRC32 value for all elements encountered so far in the master
    ID_Void = 0xec
  };
  
  /**
   * Size type used for ebml
   */
#if __EMBEDDED__
  typedef uint32 size_type;
#else
  typedef size_t size_type;
#endif
  
  /**
   * EBML id size encoding rule, in bytes.
   */
  inline
  uint32 get_id_size(id_type id) {
    if (id < (1 << 7))
      return 0;    // Error: Not an EBML id!
    if (id < (1 << 8))
      return 1;
    if (id < (1 << 15))
      return 2;
    if (id < (1 << 22))
      return 3;
    if (id < (1 << 29))
      return 4;
    // Error: Not an EBML id!
    return 0;
  }
  
  /**
   * EBML size field encoding
   * @return the number of bytes it will take to encode a size
   */
  inline
  uint32 get_encoded_size(size_type size) {
    if (size < (1<<7) - 1)
      return 1;
    if (size < (1<<14) - 1)
      return 2;
    if (size < (1<<21) - 1)
      return 3;
    if (size < (1<<28) - 1)
      return 4;
//    if (sizeof size > 4) {
//      const uint32 size32 = static_cast<uint32>(size >> 32);
//      if (size32 < (1<<7) - 1)
//        return 5;
//      if (size32 < (1<<14) - 1)
//        return 6;
//      if (size32 < (1<<21) - 1)
//        return 7;
//      if (size32 < (1<<28) - 1)
//        return 8;
//    }
    // We don't handle greater sizes
    return 0;
  }
  
  /**
   * A false bool will be encoded as having size 0.
   */
  inline
  size_type get_data_size(bool b) {
    return b ? 1 : 0;
  }
  
  /**
   * A zero float is encoded with size 0
   */
  inline
  size_type get_data_size(float f) {      
    return f == 0.f ? 0 : 4;
  }
  
  /**
   * A zero  is encoded with size 0
   */
  inline
  size_type get_data_size(uint64 d) {
    if (d == 0)
      return 0;
    
    if (d < (1<<8))
      return 1;
    if (d < (1<<16))
      return 2;
    if (d < (1<<24))
      return 3;
    
    const uint32 d32 = static_cast<uint32>(d >> 32);
    if (d32 == 0)
      return 4;
    
    if (d32 < (1<<8))
      return 5;
    
    if (d32 < (1<<16))
      return 6;
    if (d32 < (1<<24))
      return 7;
    
    return 8;
  }
  
  inline
  size_type get_data_size(int64 d) {
    return get_data_size(static_cast<uint64>(d));
  }
  
  inline
  size_type get_data_size(uint32 d) {
    if (d == 0)
      return 0;
    
    if (d < (1<<8))
      return 1;
    if (d < (1<<16))
      return 2;
    if (d < (1<<24))
      return 3;
    
    return 4;
  }
  
  inline
  size_type get_data_size(int32 d) {
    return get_data_size(static_cast<uint32>(d));
  }
  
  inline
  size_type get_data_size(uint16 d) {
    if (d == 0)
      return 0;
    
    if (d < (1<<8))
      return 1;
    return 2;
  }
  
  inline
  size_type get_data_size(int16 d) {
    return get_data_size(static_cast<uint16>(d));
  }
  
  inline
  size_type get_data_size(uint8 d) {
    if (d == 0)
      return 0;
    
    return 1;
  }
  
  inline
  size_type get_data_size(int8 d) {
    return get_data_size(static_cast<uint8>(d));
  }
  
  /**
   * EBML data size for C strings
   */
  inline
  size_type get_data_size(const char* s) {
    return util::strlen(s);
  }
  
  /**
   * EBML data size for templated array
   */
  template <typename T, uint32 N>
  size_type get_data_size(const util::array<T,N>&) {
    return N * sizeof(T);
  }
  
  /**
   * EBML data size for binary data in an adapter
   */
  template <typename T>
  size_type get_data_size(const util::vector_adapter<T>& v) {
    return sizeof(T) * v.size();
  }
  
#if !__EMBEDDED__
  inline
  size_type get_data_size(const std::string& s) {
    return s.size();
  }
#endif
  
  /**
   * EBML data size for UUIDS
   */
  inline
  size_type get_data_size(const util::Uuid& uuid) {
    return uuid.size();
  }
  
  /**
   * EBML data size for SHA1 result
   */
  inline
  size_type get_data_size(const util::SHA1::result_type& res) {
    return res.size();
  }  
}
