/*
 *  EbmlWriter.h
 *  Embedded
 *
 *  Created by Serge on 27/01/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "util.h"
#include "EbmlTypes.h"
#if !__EMBEDDED__
#  include <string>
#endif

namespace ebml {
  /**
   * A class that enforces ebml binary writing rules
   */
  class EbmlWriter: NoCopy {
  public:
    typedef EbmlWriter this_type;
    
    EbmlWriter(util::Writer& writer): m_writer(writer) {
    }
        
    /**
     * Basic binary area writing
     * @return false if not all data could be written
     */
    bool write_binary(const uint8* bytes, util::Writer::size_type count) {
      const util::Writer::size_type bytes_written = m_writer.write_all(bytes, count);
      if (bytes_written != count) {
        return false;
      }
      return true;
    }
    
    /**
     * Writes an ebml id, accordingly to the rules
     */
    bool write_id(id_type id) {
      const uint32 id_size = get_id_size(id);
      if (id_size == 0) {
        return false;
      }
      id = util::to_network_order(id);
      // Get the actual beginning of the id
      const uint8* ptr = reinterpret_cast<const uint8*>(&id) + sizeof id - id_size;
      return write_binary(ptr, id_size);
    }
    
    /**
     * Writes the size of the data. The size is encoded with leading 1 at a position
     * that depends on its number of bytes.
     */
    bool write_size(size_type sz) {
      if (sz == 0) {
        write(uint8(0x80));
        return true;
      }
      const uint32 size_size = get_encoded_size(sz);
      const uint32 shift = size_size - 1;
      sz |= (0x80 << (shift << 3)) >> shift;
      return write(uint32(sz));
    }

    /**
     * Stream data, integral type
     */
    bool write(bool b) {
      if (!b)
        return true;
      const uint8 v = b;
      return write_binary(&v, sizeof v);
    }
    
    bool write(int8 d) {
      if (d == 0)
        return true;
      return write_binary(reinterpret_cast<const uint8*>(&d), sizeof d);
    }
    
    bool write(uint8 d) {
      if (d == 0)
        return true;
      return write_binary(&d, sizeof d);
    }
    
    bool write(uint16 d) {
      if (d < (1<<8)) {
        return write(static_cast<uint8>(d));
      }
      d = util::to_network_order(d);
      return write_binary(reinterpret_cast<const uint8*>(&d), sizeof d);
    }
    
    bool write(int16 d) {
      return write(static_cast<uint16>(d));
    }
    
    bool write(uint32 d) {
      if (d < (1<<16)) {
        return write(static_cast<uint16>(d));
      }
      if (d < (1<<24)) {
        // 3 bytes to write
        d = util::to_network_order(d);
        return write_binary(reinterpret_cast<const uint8*>(&d) + 1, sizeof d - 1);
      }
      d = util::to_network_order(d);
      return write_binary(reinterpret_cast<const uint8*>(&d), sizeof d);
    }
    
    bool write(int32 d) {
      return write(static_cast<uint32>(d));
    }    
    
    bool write(float d) {
      if (d == 0.f) 
        return true;
      d = util::to_network_order(d);
      return write_binary(reinterpret_cast<const uint8*>(&d), sizeof d);
    }
    
    bool write(const char* s) {
      return write_binary(reinterpret_cast<const uint8*>(s), util::strlen(s));
    }
    
#if !__EMBEDDED__
    bool write(const std::string& s) {
      return write_binary(reinterpret_cast<const uint8*>(s.data()), s.size());
    }
#endif
    
    template <uint32 N>
    bool write(const util::array<uint8,N>& d) {
      return write_binary(d.get(), d.size());
    }
    
    template <uint32 N>
    bool write(const util::array<int8,N>& d) {
      return write_binary(reinterpret_cast<const uint8*>(d.get()), d.size());
    }
    
    bool write(const util::Uuid& d) {
      return write_binary(d.get(), d.size());
    }
    
    bool write(const util::SHA1::result_type& d) {
      return write_binary(d.h, d.size());
    }

    template <typename T, int32 lo, int32 hi>
    bool write(util::bounded_value<T, lo, hi>& v) {
      return write(v.get_value());
    }
    
  private:
    util::Writer& m_writer;
  };  
}
