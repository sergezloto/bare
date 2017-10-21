/*
 *  Framereceiver.h
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
   * An base class for receiving bytes string.
   * It enables decoupling between a protocol stream decoder and a transmission mechanism.
   * Derive from this class and implement the read() function.
   */
  class FrameReceiver {
  public:
    /**
     * This one is to make gcc shut up about non-virtual destructor
     */
    virtual ~FrameReceiver() {
    }
    
    /**
     * Receives bytes from the medium
     * @return the number of bytes effectively received
     */
    virtual bool has_frame() = 0;
    
    /**
     * Access the underlying frame buffer payload
     */
    virtual const uint8* get_frame() const = 0;
    virtual uint8* get_frame() = 0;

    /**
     * Access the underlying frame buffer payload
     */
    virtual uint32 get_frame_size() const = 0;
  };  
}
