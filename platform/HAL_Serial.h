/*
 *  HAL_Serial.h
 *  Embedded
 *
 *  Created by Serge on 09/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "util.h"
#include "HAL_Driver.h"
#include "HAL_InterruptHandler.h"

//#warning Remove SER_DEBUG before release!
#if DEBUG
# define SER_DEBUG 1
#endif

namespace hal {
  
  /**
   * Abstracts a serial port device
   */
  class Serial: public Driver, public InterruptHandler, NoCopy {
    
public:
    enum Port{
      PORT_0,
      PORT_1
    };
    
    enum Baud {
      BAUD75 = 75,
      BAUD150 = 150,
      BAUD300 = 300,
      BAUD600 = 600,
      BAUD1200 = 1200,
      BAUD2400 = 2400,
      BAUD4800 = 4800,
      BAUD9600 = 9600,
      BAUD19200 = 19200,
      BAUD38400 = 38400,
      BAUD57600 = 57600,
      BAUD115200 = 115200,
      BAUD230400 = 230400,
      BAUD460800 = 460800,
      BAUD921600 = 921600
    };
    
    enum FlowControl {
      FLOW_CONTROL_NONE,
      FLOW_CONTROL_XON_OFF
    };
    
    static const uint8 XON = 0x11, XOFF = 0x13;  // Also known as DC1, DC3

    enum InterruptMode {
      INTERRUPT_DISABLED,
      INTERRUPT_ENABLED
    };
    
    /**
     * Constructor
     */
    Serial(Port port);
    
    /**
     * Sets up the requested device with the desired baud.
     * Any function that shares resources (pins, interrupts) with this serial
     * port is disabled.
     * The port is set for 8 bit, no parity, 1 stop bit => 8N1
     */
    void configure(Baud baud);
    
    /**
     * Stops the uart
     */
    void stop();
    
    /**
     * Stops and return to reset state
     */
    void reset();
    
    /**
     * Sets the flow control mode
     */
    void set_flow_control(FlowControl flow_control);
    
    /**
     * Prepares for auto-bauding, i.e determine the incoming bit rate.
     * The auto bauding is set for auto retry, and synchronize on the duration of the start bit
     */
    void set_auto_bauding(bool on);
    
    /**
     * @return true if the port is in auto bauding mode, that is, it has not yet synchronized.
     */
    bool get_auto_bauding() const;

    /**
     * Enables or disables interrupt mode.
     * When switching to non-interrupt mode any pending transmit may be
     * "forgotten"
     */
    void set_interrupt_mode(InterruptMode mode);
    
    /**
     * send a byte string
     * @return the number of bytes written
     */
    uint32 write(const uint8* bytes, uint32 size);
    
    /**
     * Ensures any pending transmit is sent on the wire.
     * This is a blocking call.
     */
    void flush();
    
    /**
     * Reads a string of bytes from the device
     * @return the number of bytes read
     */
    uint32 read(uint8 *bytes, uint32 count);
    
    
private:
    static const uint32 TX_FIFO_SIZE = 16U;
    
    void handle_irq();
#if 0   
#  if !ZO_HAL_SERIAL_SEND_BUFFER_SIZE || !ZO_HAL_SERIAL_RECEIVE_BUFFER_SIZE
#    error Define ZO_HAL_SERIAL_SEND_BUFFER_SIZE and ZO_HAL_SERIAL_RECEIVE_BUFFER_SIZE to use hal::Serial
#  endif
    typedef util::Buffer<uint8, ZO_HAL_SERIAL_SEND_BUFFER_SIZE> send_buffer_type;
    typedef util::Buffer<uint8, ZO_HAL_SERIAL_RECEIVE_BUFFER_SIZE> receive_buffer_type;
#else
#  if DEBUG
    typedef util::Buffer<uint8, 1023> send_buffer_type;
    typedef util::Buffer<uint8, 127> receive_buffer_type;
#  else
    typedef util::Buffer<uint8, 127> send_buffer_type;
    typedef util::Buffer<uint8, 127> receive_buffer_type;
#  endif
#endif
   
    const Port m_port;
    
    FlowControl m_flow_control;
    InterruptMode m_interrupt_mode;
    
    volatile send_buffer_type m_send_buffer;
    volatile receive_buffer_type m_receive_buffer;
    
#if SER_DEBUG
  public:
    uint32 m_nb_read_interrupts;
    uint32 m_nb_write_interrupts;
    uint32 m_nb_framing_errors;
    uint32 m_nb_overrun_errors;
    uint32 m_nb_parity_errors;
    uint32 m_nb_error_on_rx_fifo;
#endif
  };
  
  inline Serial::Serial(Port port)
  : m_port(port), m_flow_control(FLOW_CONTROL_NONE), m_interrupt_mode(INTERRUPT_DISABLED)
#if SER_DEBUG
  , m_nb_read_interrupts(0)
  , m_nb_write_interrupts(0)
  , m_nb_framing_errors(0)
  , m_nb_overrun_errors(0)
  , m_nb_parity_errors(0)
  , m_nb_error_on_rx_fifo(0)
#endif
  {
  }
}
