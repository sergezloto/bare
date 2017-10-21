/*
 *  PacketParser.h
 *  Embedded
 *
 *  Created by Serge on 1/19/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "util.h" // For array<T,N>

#if !defined(USE_EMBEDDED)
#  include <stdexcept>
#  include <string>
#endif

namespace protocol {
  /**
   * Helps in parsing packets received from the device
   */
  class PacketParser {
  public:
    PacketParser(uint8* payload, uint32 payload_size)
    : m_payload(payload), m_payload_size(payload_size), m_offset(0) {
    }
    
    /**
     * Gets data from the payload
     */
    template <typename T>
    T  get();

#if 0 && !defined(USE_EMBEDDED)
    /**
     * Gets a string (uint32 + variable number of bytes)
     */
    std::string get_string();
#endif

    /**
     * Reads data into its parameter
     */
    template <typename T>
    PacketParser& read(T& value);
    
    /**
     * Read an enum
     */
    template <typename T>
    PacketParser& read_enum(T& value) {
      uint32 v;
      read(v);
      value = static_cast<T>(v);
      return *this;
    }
#if 1
    /**
     * bool specific reader, expect 4 bytes, 'vrai' or 'faux'
     */
    //template <>
    PacketParser& read(bool& b);
    
    /**
     * Reads a uuid
     */
    //template<>
    PacketParser& read(util::Uuid& uuid);
    
    /**
     * Reads a sha1 result
     */
    //template<>
    PacketParser& read(util::SHA1::result_type& result);
#endif
    
    /**
     * Read a templated value array
     */
    template <typename T, uint32 SIZE>
    PacketParser& read(util::array<T,SIZE>& array) {
      for (uint32 i = 0; i < array.size(); ++i) {
        read(array[i]);
      }
      return *this;
    }
    
    /**
     * Reads remaining of packet as pointer/size
     */
    PacketParser& read(uint8* &buffer, uint32& buffer_size) {
      buffer = &m_payload[m_offset];
      buffer_size = m_payload_size - m_offset;
      m_offset = m_payload_size;
      return *this;
    }
    
    
    uint32 size() const {
      return m_payload_size;
    }
    
    bool at_eof() const {
      return m_offset >= m_payload_size;
    }
    
    void rewind() {
      m_offset = 0;
    }
    
  private:
    uint8* const m_payload;
    const uint32 m_payload_size;
    uint32 m_offset;
  };
  
  /**
   * Gets data from the payload
   */
  template <typename T>
  T  PacketParser::get() {
#if !defined(USE_EMBEDDED)
    if (m_offset + sizeof(T) > m_payload_size)
      throw std::runtime_error("Cannot deserialize packet");
#endif
    T value;
    read(value);
    return value;
  }

#if !defined(USE_EMBEDDED)
  /**
   * Specialization for strings
   */
  template<>
  inline
  std::string PacketParser::get<std::string>() {
    // The string length is a 4 bytes value
    const uint32 string_size = get<uint32>();
    if (m_offset + string_size > m_payload_size)
      throw std::runtime_error("Cannot deserialize string from packet");
    
    const std::string value = std::string((char*)(m_payload + m_offset), string_size);
    m_offset += string_size;
    
    return value;
  }
#endif

  /**
   * Reads data into its parameter
   */
  template <typename T>
  PacketParser& PacketParser::read(T& value) {
#if !defined(USE_EMBEDDED)
    if (m_offset + sizeof(T) > m_payload_size)
      throw std::runtime_error("Cannot deserialize packet");
#endif
    value = util::to_little_endian(*((T*)&m_payload[m_offset]));
    m_offset += sizeof value;
    return *this;
  }
  
  /**
   * bool specific reader, expect 4 bytes, 'vrai' or 'faux'
   */
  inline
  PacketParser& PacketParser::read(bool& b) {
    uint32 v;
    read(v);
    b = v != 'faux';
    return *this;
  }

  
  /**
   * Reads a uuid
   */
  inline
  PacketParser& PacketParser::read(util::Uuid& uuid) {
    for (uint32 i = 0; i < uuid.size(); ++i) {
      read(uuid[i]);
    }
    return *this;
  }
  
  /**
   * Reads a sha1 result
   */
  inline
  PacketParser& PacketParser::read(util::SHA1::result_type& result) {
    read(result.h0);
    read(result.h1);
    read(result.h2);
    read(result.h3);
    read(result.h4);
    return *this;
  }
  
  
}
