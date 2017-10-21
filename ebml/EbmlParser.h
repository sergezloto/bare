/*
 *  EbmlParser.h
 *  Embedded
 *
 *  Created by Serge on 27/01/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once
#include "EbmlTypes.h"
#include "EbmlElement.h"
#include "util.h"

namespace ebml {
  enum parse_result_type {
    RES_ELEMENT,
    RES_NO_MORE,
    RES_ERROR_FORMAT,
    RES_ERROR_OVERRUN
  };
  
  /**
   * A streaming, callback based parser for ebml
   */
  class EbmlParser {
    static const size_type INVALID_SIZE = ~size_type(0);
    
  public:
    typedef util::Reader reader_type;
    
    EbmlParser(reader_type& reader)
    : m_upper_parser(0), m_reader(reader), m_bytes_read(0), m_outer_size(0) {
    }
        
    EbmlParser(reader_type& reader, size_type outer_size)
    : m_upper_parser(0), m_reader(reader), m_bytes_read(0), m_outer_size(outer_size) {
    }
    
    /**
     * Implicit use of generated copy constructor, used to obtain sub-parsers
     */
    // EbmlParser(const EbmlParser& parser);
    
    /**
     * Resets the parser. Parsing, if resumed, will start from the current stream position.
     * Calling reset() from within process_element() is not advised!
     */
    void reset() {
      m_bytes_read = 0;
    }
    
    /**
     * Parses an element. The stream is assumed to lie at element start.
     * @return true if parse was successful
     */
    parse_result_type parse() {
      // Detect if we have overrun the outermost (first) element
      if (m_outer_size > 0) {
        // We have a limit
        if (m_bytes_read == m_outer_size) {
          return RES_NO_MORE;
        } else if (m_bytes_read >= m_outer_size) {
          return RES_ERROR_OVERRUN;
        }
      }
      m_element_id = parse_id();
      if (m_element_id == 0) {
        return RES_ERROR_FORMAT;
      }
      m_element_size = parse_size();
      if (m_element_size == INVALID_SIZE) {
        return RES_ERROR_FORMAT;
      }
      // TODO: If we have a CRC32 element we process it and check, then return it
      return RES_ELEMENT;
    }
    
    /**
     * Allows to recurse EBML tags
     * @return a new parser that parses from the current reader position
     */
    EbmlParser get_sub_parser() {
      // the element size is the sub elements outer size
      EbmlParser sub_parser(this, m_reader, m_element_size);
      return sub_parser;
    }
    
    /**
     * Accessor to the element id just parsed
     */
    id_type get_element_id() const {
      return m_element_id;
    }
    
    /**
     * Accessor to the element size just parsed
     */
    size_type get_element_size() const {
      return m_element_size;
    }

    /**
     * Skips the current valid element data
     * @return true if the skip was successful
     */
    bool skip_element() {
      uint8 buf;
      for (size_type i = 0; i < m_element_size; ++i) {
        // Perform 1 byte dummy reads
        if (read(&buf, sizeof buf) < sizeof buf)
          return false;
      }
      return true;
    }
    
    /**
     * Parses an element
     * @return true if the element was successfully parsed
     */
    template <typename T>
    bool parse(Element<T>& el) {
      T value;
      if (!parse_value(value))
        return false;
      el.set_value(value);
      return true;
    }
    
    /**
     * Parses a float
     * @return true if the value was successfully parsed
     */
    bool parse_value(float& f) {
      switch (m_element_size) {
        case 4:
        {
          union {
            float f;
            uint8 b[4];
          } v;
          if (!parse_value_binary(v.b, static_cast<size_type>(sizeof v.b)))
            return false;
          f = util::to_host_order(v.f);
          return true;
        }
          break;
        default:
          // Invalid EBML or unexpected format
          return false;
      }
    }
    
    /**
     * Parses a bounded value
     */
    template <typename T, int32 lo, int32 hi>
    bool parse_value(util::bounded_value<T, lo, hi>& v) {
      T t;
      if (!parse_value(t))
        return false;
      // t has been parsed, assign it (with check)
      v = t;
      return true;
    }
    
    /**
     * Parses an integral type
     * @return true if the value was successfully parsed
     */
    bool parse_value(uint32& v) {
      return parse_value(v, m_element_size);
    }
    
    /**
     * Parses a signed integral type
     * @return true if the value was successfully parsed
     */
    bool parse_value(int32& v) {
      return parse_value(v, m_element_size);
    }
    
    /**
     * Parses an integral type
     * @return true if the value was successfully parsed
     */
    bool parse_value(uint16& v) {
      return parse_value(v, m_element_size);
    }
    
    /**
     * Parses an integral type
     * @return true if the value was successfully parsed
     */
    bool parse_value(uint8& v) {
      return parse_value(v, m_element_size);
    }
    
    /**
     * Parses a boolean
     */
    bool parse_value(bool& b) {
      uint32 v;
      if (!parse_value(v))
        return false;
      b = v;
      return true;
    }
    
    /**
     * Parses a string (bounded character array)
     */
    bool parse_value(char* s, uint32 len) {
      if (len == 0)
        return false;
      if (len > m_element_size) {
        // Consume everything
        if (!parse_value_binary((uint8*)s, m_element_size))
          return false;
        s[m_element_size] = '\0';
        return true;
      }
      // Not enough space in the passed buffer. Truncate...
      if (!parse_value_binary((uint8*)s, len)) {
        return false;
      }
      s[len - 1] = '\0';
      // ...and consume the extra bytes
      for (uint32 i = 0; i < m_element_size - len; ++i) {
        uint8 b;
        if (!parse_value(b, sizeof b)) {
          return false;
        }
      }
      return true;
    }
    
#if !__EMBEDDED__
    /**
     * Parses an STL string
     */
    bool parse_value(std::string& s) {
      // Ensure there is enough capacity in the string
      s.resize(static_cast<std::string::size_type>(m_element_size));
      return parse_value_binary((uint8*)s.data(), m_element_size);
    }
#endif
    
  protected:
    EbmlParser(EbmlParser* upper_parser, reader_type& reader, size_type outer_size)
    : m_upper_parser(upper_parser), m_reader(reader), m_bytes_read(0), m_outer_size(outer_size) {
    }
    
    /**
     * Reads bytes
     */
    reader_type::size_type read(uint8* b, reader_type::size_type n) {
      const reader_type::size_type nr = m_reader.read_all(b, n);
      set_bytes_consumed(nr);
      return nr;
    }
    
    void set_bytes_consumed(reader_type::size_type nr) {
      m_bytes_read += nr;
      if (m_upper_parser) {
        m_upper_parser->set_bytes_consumed(nr);
      }
    }
    
    /**
     * @return the number of bytes read so far
     */
    size_type get_bytes_read() const {
      return m_bytes_read;
    }
    
    /**
     * Parses a buffer from the stream
     */
    bool parse_value_binary(uint8* ptr, size_type len) {
      const size_type nr = read(ptr, len);
      if (nr < len)
        return false;
      return true;
    }
    
    /**
     */
    bool parse_value(uint8& v, size_type len) {
      uint32 v32 = 0;
      if (!parse_value(v32, len)) {
        return false;
      }
      v = static_cast<uint8>(v32);
      return true;
    }
    bool parse_value(uint16& v, size_type len) {
      uint32 v32 = 0;
      if (!parse_value(v32, len)) {
        return false;
      }
      v = static_cast<uint16>(v32);
      return true;
    }
    
    /**
     * Parses an integer value of len bytes from the stream
     */
    bool parse_value(uint32& v, size_type len) {
      if (len == 0) {
        v = 0;
        return true;
      }
      uint8 b[len];
      const size_type nr = read(b, len);
      if (nr < len)
        return false;
      v = b[0];
      for (uint32 i = 1; i < len; ++i) {
        v = (v << 8) | b[i];
      }
      return true;
    }
    
    /**
     * Parses a signed integer value of len bytes from the stream
     */
    bool parse_value(int32& v, size_type len) {
      uint32 t;
      if (parse_value(t, len)) {
        v = static_cast<int32> (t);
        return true;
      }
      return false;
    }
    
    /**
     * Parses the input and returns an ebml id, complete with leading 1.
     * @return 0 if the id is invalid
     */
    id_type parse_id() {
      uint8 id1;
      const uint32 nr1 = read(&id1, sizeof id1);
      if (nr1 == 0) {
        // Read error
        return 0;
      }
      if (id1 & 0x80) {
        return id1;
      }
      
      // We need to read more bytes!
      uint32 sz = 0;
      
      if (id1 & 0x40) {
        sz = 1;
      } else if (id1 & 0x20) {
        sz = 2;
      } else if (id1 & 0x10) {
        sz = 3;
      } else {
        // Illegal id
        return 0;
      }
      uint8 b[sz];
      const uint32 n = read(b, sz);
      if (n < sz) {
        // Read error
        return 0;
      }
      // Assemble the id from its big endian format just read
      id_type d;
      switch (sz) {
        case 1:
          d = (id1 << 8) | b[0];
          break;
        case 2:
          d = (id1 << 15) | (b[0] << 8) | b[1];
          break;
        case 3:
          d = (id1 << 24) | (b[0] << 16) | (b[1] << 8) | b[2];
          break;
        default:
          // Logic error, we shouldn't get here at all
          d = 0;
          break;
      }
      
      return d;
    }
    
    /**
     * Parses the input and returns an ebml size, without leading 1
     */
    size_type parse_size() {
      uint8 sz1;
      const uint32 nr1 = read(&sz1, sizeof sz1);
      if (nr1 == 0) {
        return INVALID_SIZE;
      }
      if (sz1 & 0x80) {
        // Single byte size
        return sz1 & 0x7f;
      }
      uint32 nb;
      if (sz1 & 0x40) {
        nb = 1;
        sz1 &= 0x3f;
      } else if (sz1 & 0x20) {
        nb = 2;
        sz1 &= 0x1f;
      } else if (sz1 & 0x10) {
        nb = 3;
        sz1 &= 0xf;
      } else if ((sizeof(size_type) > 4) && (sz1 & 0x08)) {
        nb = 4;
        sz1 &= 0x07;
      } else if ((sizeof(size_type) > 4) && (sz1 & 0x04)) {
        nb = 5;
        sz1 &= 0x03;
      } else if ((sizeof(size_type) > 4) && (sz1 & 0x02)) {
        nb = 6;
        sz1 &= 0x01;
      } else if ((sizeof(size_type) > 4) && (sz1 & 0x01)) {
        nb = 7;
        sz1 = 0;
      } else {
        return INVALID_SIZE;      
      }
      uint8 buf[nb];
      const uint32 nr = read(buf, nb);
      if (nr < nb) {
        return INVALID_SIZE;
      }
      size_type sz = sz1;
      for (uint32 i = 0; i < nb; ++i) {
        sz = (sz << 8) | buf[i];
      }
      return sz;
    }
    
  private:
    EbmlParser* m_upper_parser;
    reader_type& m_reader;
    size_type m_bytes_read;
    const size_type m_outer_size;
    id_type m_element_id;
    size_type m_element_size;
    
    /**
     * Forbidden operation
     */
    void operator=(const EbmlParser&);
  };
}
