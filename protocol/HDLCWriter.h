/*
 *  HDLCWriter.h
 *  Embedded
 *
 *  Created by Serge on 27/11/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "HDLCLike.h"
#include "FrameSender.h"

namespace protocol {
  /**
   * A frame writer class
   */
  template <class STREAM_WRITER, uint32 CAPACITY>
  class HDLCWriter: protected HDLCLike<CAPACITY>, public FrameSender {
    typedef HDLCLike<CAPACITY> hdlc_type;
    
    enum State {
      START,
      TRANSMIT,
      END
    };
    
  public:
    HDLCWriter(STREAM_WRITER& writer): m_writer(writer), m_state(START) {
    }
    
    /**
     * Appends a byte to the frame to send
     */
    bool put(uint8 b) {
      if (/*b < hdlc_type::XOR ||*/ b == hdlc_type::FLAG || b == hdlc_type::ESC) {
        // Do we have space for escaping this byte?
        if (hdlc_type::frame_size() + 2 > hdlc_type::capacity())
          return false;
        hdlc_type::add_byte(hdlc_type::ESC);
        hdlc_type::add_byte( b ^ hdlc_type::XOR);
      } else {
        if (!hdlc_type::add_byte(b))
          return false;
      }
      //
      // Add to the hash calculator the new value
      //
      m_crc32.process(&b, sizeof(b));
      return true;
    }
        
    /**
     * Adds a byte string to the frame buffer
     * @return the number of bytes written
     */
    uint32 write(const uint8* bytes, uint32 size_i) {
      uint32 i;
      for (i = 0; i < size_i; ++i) {
        const uint8 b = put(*bytes++);
        if (!b)
          break;
      }
      return i;
    }
    
    /**
     * Sends the current frame. If the frame is empty two flags are sent.
     * @return true if there is no pending send operation or whole frame was sent
     */
    bool write_end() {
      switch (m_state) {
        case START:
        {
          util::CRC32::result_type crc32 = m_crc32.get_result();
          crc32 = util::to_little_endian(crc32);
          uint8 b0 = crc32 & 0xff;
          uint8 b1 = (crc32 >> 8) & 0xff;
          uint8 b2 = (crc32 >> 16) & 0xff;
          uint8 b3 = (crc32 >> 24) & 0xff;
          if (! (put(b0) && put(b1) && put(b2) && put(b3))) {
            // No space for hash!
            // TODO: Mark an error
            
            // The operation failed -silently for now-. Signal it's over
            return true;
          }
        }
        {
          // Start sending
          const uint8 flag = hdlc_type::FLAG;
          if (!m_writer.write(&flag, 1))
            goto not_done;
          
          m_state = TRANSMIT;
          m_send_index = 0;
        }
          /* FALLTHROUGH */
          
        case TRANSMIT:
          while (m_send_index < hdlc_type::frame_size()) {
            const uint32 bytes_written = m_writer.write(get_frame() + m_send_index, 
                                                        hdlc_type::frame_size() - m_send_index);
            if (bytes_written == 0)
              goto not_done;
            m_send_index += bytes_written;
          }
          
          m_state = END;
          /* FALLTHROUGH */
          
        case END:
        {
          const uint8 flag = hdlc_type::FLAG;
          if (!m_writer.write(&flag, 1))
            goto not_done;
          
          m_state = START;
          
          // Frame was sent, discard now
          hdlc_type::reset();
          
          // Reset the hash too
          m_crc32.reset();
          
          // Tell the caller we are done
          return true;
        }          
      }
      
    not_done:
      return false;
    }
    
    /**
     * Sends the FLAG byte
     */
    void send_flag() {
      const uint8 flag = hdlc_type::FLAG;
      m_writer.write(&flag, sizeof flag);
    }
    
    /**
     * Cancels the pending frame or send operation
     */
    void write_cancel() {
      m_send_index = 0;
      hdlc_type::reset();
      m_crc32.reset();
    }
    
    /**
     * Access the underlying frame buffer payload
     */
    virtual const uint8* get_frame() {
      return hdlc_type::get_frame();
    };
    
    /**
     * Access the underlying frame buffer payload
     */
    virtual uint32 get_frame_size() {
      return hdlc_type::size();
    }
    
    
  private:
    STREAM_WRITER& m_writer;
    State m_state;
    uint32 m_send_index;
    util::CRC32 m_crc32;
  };  
}
