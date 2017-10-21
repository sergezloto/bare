/*
 *  FrameSender.h
 *  Embedded
 *
 *  Created by Serge on 27/11/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace protocol {
  /**
   * An base class for sending bytes string.
   * It enables decoupling between a protocol packet encoder and a transmission mechanism.
   * Derive from this class and implement the write() and send() functions.
   */
  class FrameSender {
  public:
    /**
     * This one is to make gcc shut up about non-virtual destructor
     */
    virtual ~FrameSender() {
    }
    
    /**
     * Write bytes to the medium
     * @return the number of bytes effectively written
     */
    virtual uint32 write(const uint8 *bytes, uint32 count) = 0;
    
    /**
     * Signals that the frame is complete.
     * Framing protocol can use this to end their frame.
     * The frame may be sent only then.
     * @return true when the frame is sent
     */
    virtual bool write_end() = 0;
    
    /**
     * Cancels an ongoing frame sending or writing
     */
    virtual void write_cancel() = 0;
    
  };  
}
