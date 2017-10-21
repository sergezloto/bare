/*
 *  Packet.h
 *  Embedded
 *
 *  Created by Serge on 7/11/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "HDLCLike.h"
#include "FrameSender.h"

/**
 * Packets follow this coding:
 * -integral types are sent in little endian format, on 4 bytes
 * -strings have a fixed size, dictated by the packet type
 * 
 * The packet protocol assumes an underlying framing and reliable transport.
 */
namespace protocol {
  
  /**
   * Forward declaration of the parser type
   */
  class PacketParser;
  
  /**
   * Groups functions common to all packet types
   */
  class Packet {
  public:
    /**
     * Sends the packet using the given sender
     */
    bool send(FrameSender& sender) {
      if (!write(sender))
        return false;
      return sender.write_end();
    }
    
  protected:
    /**
     * This class can't be directly instantiated
     */
    Packet() {
    }
    
    /**
     * It has virtual function so make the destructor virtual as well
     */
    virtual ~Packet() {
    }
    
    /**
     * writes the data part of the packet
     *
     * there is no data to send at this level, hence it is pure virtual
     */
    virtual bool write(FrameSender& sender) = 0;
    
    /**
     * Marks the end of the packet
     */
    bool write_end(FrameSender& sender) {
      return sender.write_end();
    }
    
    /**
     * Appends a 16 bit to the frame. The 16 bit is converted to little endian order.
     */
    bool write(FrameSender& sender, uint16 u) {
      u = util::to_little_endian(u);
      
      const uint32 bytes_written = sender.write(reinterpret_cast<const uint8*>(&u), sizeof u);
      return bytes_written == sizeof u;
    }
    
    /**
     * Appends a 32 bit to the frame. The 32 bit is converted to little endian order.
     */
    bool write(FrameSender& sender, uint32 u) {
      u = util::to_little_endian(u);
      
      const uint32 bytes_written = sender.write(reinterpret_cast<const uint8*>(&u), sizeof u);
      return bytes_written == sizeof u;
    }
    
    /**
     * Appends a float to the frame. In little endian.
     */
    bool write(FrameSender& sender, float f) {
      f = util::to_little_endian(f);
      return write(sender, reinterpret_cast<uint8*>(&f), sizeof(f));
    }
    
    /**
     * Appends a truth value
     */
    bool write(FrameSender& sender, bool b) {
      const uint32 u = b ? 'vrai' : 'faux';
      return write(sender, u);
    }
    
    /**
     * Appends a string to the frame.
     * The string size, a 32 bit integer, is written, then the string itself
     * without the ending '\0'
     */
    bool write(FrameSender& sender, const char* s) {
      const uint32 count = util::strlen(s);
      
      if (!write(sender, count)) {
        return false;
      }
      const uint32 bytes_written =  sender.write(reinterpret_cast<const uint8*>(s), count);
      
      return bytes_written == count;
    }
    
    /**
     * Appends a byte array to the frame
     */
    bool write(FrameSender& sender, const uint8* bytes, uint32 count) {
      const uint32 bytes_written =  sender.write(bytes, count);
      
      return bytes_written == count;
    }
    
    bool write(FrameSender& sender, const char* s, uint32 count) {
      const uint32 bytes_written =  sender.write(reinterpret_cast<const uint8*>(s), count);
      
      return bytes_written == count;
    }
    
#if !__EMBEDDED__
    /**
     * Appends an STL string
     */
    bool write(FrameSender& sender, const std::string& str) {
      return write(sender, str.c_str(), str.size());
    }
#endif
    
    /**
     * Appends an enumerated type or any that is convertible to uint32
     */
    template <typename T>
    bool write_enum(FrameSender& sender, T value) {
      return write(sender, static_cast<uint32>(value));
    }
    
    /**
     * Appends a templated array
     */
    template <typename T, uint32 N>
    bool write(FrameSender& sender, const util::array<T, N>& data) {
      return write(sender, data.get(), data.size());
    }

    /**
     * Appends an uuid
     */
    bool write(FrameSender& sender, const util::Uuid& data) {
      return write(sender, data.get(), data.size());
    }

    /**
     * Appends a sha1 result
     */
    bool write(FrameSender& sender, const util::SHA1::result_type& data) {
      return write(sender, data.h0)
      && write(sender, data.h1)
      && write(sender, data.h2)
      && write(sender, data.h3)
      && write(sender, data.h4);
    }
  };
  
  /**
   * Templated no-payload packet
   */
  template <uint32 TYPE>
  class NoPayload: public Packet {
    typedef Packet super;
  public:
    static const uint32 type = TYPE;
    
    /**
     * Constructor
     */
    NoPayload() {
      // Nothing to do
    }
    
    /**
     * Parsing a no payload packet: nothing to do!
     * The presence of this function makes payload packets identical to other type
     * when it comes to receiving them
     */
    NoPayload(PacketParser& /*packet_parser*/) {
      // Nothing to do
    }
    
  protected:
    virtual bool write(FrameSender& sender) {
      // We don't need to call the super class write here
      
      // Just write the type
      return super::write(sender, type);
    }
  };
}
