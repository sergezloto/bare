/*
 *  HDLCReader.h
 *  Embedded
 *
 *  Created by Serge on 27/11/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "HDLCLike.h"
#include "FrameReceiver.h"

namespace protocol {
  /**
   * A frame reader class
   */
  template <class STREAM_READER, uint32 CAPACITY>
  class HDLCReader: protected HDLCLike<CAPACITY>, public FrameReceiver {
    typedef HDLCLike<CAPACITY> hdlc_type;
    
    /**
     * Our receiving state machine
     */
    enum State {
      SYNC,
      START,
      DATA,
      ESCAPE
    };
    
  public:
    HDLCReader(STREAM_READER& reader): m_stream_reader(reader), m_state(SYNC) {
    }
    
    /**
     * Gathers bytes and tries to assemble a full frame.
     * @return true if a full frame has been gathered
     */
    virtual bool has_frame() {
      while (true) {
        uint8 b;
        const uint32 got_byte = m_stream_reader.read(&b, sizeof b);
        
        if (!got_byte) {
          // We don't have a frame since we don't have a byte
          return false;
        }
        
        switch (m_state) {
          case SYNC:
            if (b == hdlc_type::FLAG)
              m_state = START;
            break;
            
          case START:
            if (b == hdlc_type::FLAG)
              break;
            
            // We have a byte for a new frame. Prepare the new frame
            hdlc_type::reset();
            
            if (b == hdlc_type::ESC) {
              m_state = ESCAPE;
              break;
            }
            if (hdlc_type::add_byte(b))
              m_state = DATA;
            else {
              // The byte couldn't be added to the frame. We have an overflow. TODO: Mark an error
              //  Resynchronize
              m_state = SYNC;
            }
            break;
            
          case DATA:
            if (b == hdlc_type::FLAG) {
              bool has_framep = false;
              // End of frame. Check integrity
              if (hdlc_type::frame_size() < hdlc_type::MIN_FRAME_SIZE) {
                // TODO: mark an error condition
              } else {
                // Compute the hash
                using util::CRC32;
                
                const uint32 hash_offset = hdlc_type::frame_size() - 4;
                CRC32 crc32;
                crc32.process(get_frame(), hash_offset);
                CRC32::result_type calc_h = crc32.get_result();
                // Extract the hash from the frame, which is located in the 4 end bytes
                // It is in little endian order
                CRC32::result_type frame_h = 
                (hdlc_type::get_frame(0 + hash_offset) << 0)
                | (hdlc_type::get_frame(1 + hash_offset) << 8)
                | (hdlc_type::get_frame(2 + hash_offset) << 16)
                | (hdlc_type::get_frame(3 + hash_offset) << 24);
                
                // Verify the CRC32 matches
                if (calc_h == frame_h) {
                  has_framep = true;
                }
              }
              // Get ready for a new frame
              m_state = START;
              
              // Tell the caller we have a frame!
              return has_framep;
            }
            if (b == hdlc_type::ESC) {
              m_state = ESCAPE;
              break;
            }
            if (hdlc_type::add_byte(b))
              m_state = DATA;
            else {
              // The byte couldn't be added to the frame. We have an overflow. TODO: Mark an error
              //  Resynchronize
              m_state = SYNC;
            }
            break;
            
          case ESCAPE:
            if (b == hdlc_type::FLAG) {
              // We have a framing error. cancel this frame. TODO: Mark an error condition
              m_state = SYNC;
            }
            if (hdlc_type::add_byte(b ^ hdlc_type::XOR))
              m_state = DATA;
            else {
              // The byte couldn't be added to the frame. We have an overflow. TODO: Mark an error condition
              //  Resynchronize
              m_state = SYNC;
            }
            break;
            
          default:
            break;
        }
      }
    }

  /**
   * Access to the payload buffer
   */
  virtual const uint8* get_frame() const {
    return hdlc_type::get_frame();
  }
  
  virtual uint8* get_frame() {
    return hdlc_type::get_frame();
  }

    /**
     * @return the size of the frame payload
     */
  virtual uint32 get_frame_size() const {
    return hdlc_type::size();
  }
    
  private:
    STREAM_READER & m_stream_reader;
    
    State m_state;
  };
}
