/*
 *  HAL_I2C.h
 *  Embedded
 *
 *  Created by Serge on 12/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "HAL_Driver.h"
#include "HAL_InterruptHandler.h"

namespace hal {
  
  /**
   * Abstracts an I2C port device, with buffering tx and rx bytes to two queues
   * 
   */
  class I2C: public Driver, InterruptHandler, NoCopy {
  public:
    
    enum Direction {
      RECEIVE = 0,
      SEND = 1
    };
    
    enum Port{
      PORT_0,
      PORT_1
    };
    
    /**
     * An i2c address is even values between 2 and 254, inclusive. 
     * 0 is used for broadcast to all devices on the bus
     */
    typedef uint8 Address;
    
    /**
     * This is the type of the data carried over by i2c (just bytes)
     */
    typedef uint8 Data;
    
    /**
     * Constructor
     * @return an instance of this I2C driver for the given port
     */
    I2C(Port port);
    
    /**
     * Sets up the requested device with the desired baud.
     * Any function that shares resources (pins, interrupts) with this I2C
     * port is disabled.
     * The i2c port is configured as a master.
     * The baud should not be higher than 400 000 bit per sec for i2c
     */
    void configure(uint32 baud);
    
    /**
     * Sets up the bus speed
     */
    void setBaud(uint32 baud);
    
    /**
     * Writes multiple bytes to the chosen address
     * @param address a 7 bit address as per i2c specs. The lsb of the address
     *                will be set to 1 for write by this function
     * @return the number of bytes sent
     */
    uint8 send(Address address, const Data *data, uint8 dataLen) const;
    
    /**
     * Reads bytes from the i2c port.
     * @param address a 7 bit address as per i2c specs. The lsb of the address
     *                will be set to 0 for read by this function
     * @return The number of bytes received
     */
    uint8 receive(Address address, Data* data, uint8 bufferLen) const;
    
    /**
     * Query the status of the last transaction
     */
    bool isError() const;
    
    /**
     * For debugging ISR. // TODO: REMOVE
     */
    static uint32 getUnknownInterrupts();
    
    /**
     * For debugging ISR. // TODO: REMOVE
     */
    static uint32 getSpuriousInterrupts();
    
    /**
     * For debugging ISR. // TODO: REMOVE
     */
    static uint32 getInterrupts();
    
  private:    
    const Port m_port;
    
    static uint8 transfer(Port port, Direction dir, Address address, const Data *data, uint8 dataLen);
    
    void handle_irq();
  };
  
  inline
  I2C::I2C(Port port)
  : m_port(port) {
  }
  
  inline 
  uint8 I2C::send(Address address, const Data *data, uint8 dataLen) const {
    return transfer(m_port, SEND, address, data, dataLen);
  }
  
  inline 
  uint8 I2C::receive(Address address, Data *data, uint8 dataLen) const {
    return transfer(m_port, RECEIVE, address, data, dataLen);
  }
}
