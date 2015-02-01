/*
 *  HAL_I2C.cpp
 *  Embedded
 *
 *  Created by Serge on 12bufferSize/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_I2C.h"
#include "HAL_VIC.h"
#include "HAL_PinSelect.h"
#include "HAL_Vectors.h"

#include "lpc214x.h"

namespace hal {
  
  /*
   * Interrupt service routines
   */
  
  /*
   * State space for an I2C transaction
   */
  struct I2CState {
private:
    enum State {
      IDLE,
      BUSY,
      DONE,
      ERROR
    };
public:
    uint32 interrupts;
    uint32 spuriousInterrupts;
    uint32 unknownInterrupts;
    State state;
    I2C::Address address;
    I2C::Data* buffer;
    uint8 byteIndex;
    uint8 bufferSize;
    
    I2CState() { 
      reset(); 
      interrupts = 0; 
      spuriousInterrupts = 0; 
      unknownInterrupts = 0;
    }
    void reset() volatile {
      state = IDLE;
      address = 0;
      buffer = 0;
      byteIndex = 0;
      bufferSize = 0;
    }
    
    void setIdle() volatile { state = IDLE; }
    void setBusy() volatile { state = BUSY; }
    void setDone() volatile { state = DONE; }
    void setError() volatile { state = ERROR;}
    bool isIdle() volatile { return state == IDLE; }
    bool isBusy() volatile { return state == BUSY; }
    bool isDone() volatile { return state == DONE; }
    bool isError() volatile { return state == ERROR; }
  };
  
  static volatile I2CState i2c0State;
  //static volatile I2CState i2c1State;
		
  /*
   * Use these definitions with
   * Port 0: I2C0CONCLR, I2C0CONSET
   * Port 1: I2C1CONCLR, I2C1CONSET
   */
  enum I2CControlRegisterBits {
    AA 		= 1 << 2,		// Assert acknowledge.
    SI 		= 1 << 3,		// Interrupt flag
    STO 	= 1 << 4,		// Stop flag
    STA 	= 1 << 5,		// Start flag
    I2EN 	= 1 << 6		// Enable flag
  };
  
  /*
   * Status bits for use with I2C0STAT and I2C1STAT
   */
  enum I2CStatusRegisterBits {
    START        = 0x08,
    RESTART      = 0x10,
    MT_SLA_ACK   = 0x18,
    MT_SLA_NACK  = 0x20,
    MT_DATA_ACK  = 0x28,
    MR_SLA_ACK   = 0x40,
    MR_SLA_NACK  = 0x48,
    MR_DATA_ACK  = 0x50,
    MR_DATA_NACK = 0x58
		};
  
  inline void sendStart0() {
    I2C0CONSET = STA;
  }
  inline void sendAck0() {
    I2C0CONSET = AA;
  }
  inline void sendNack0() {
    I2C0CONCLR = AA;
  }
  inline void sendByte0(I2C::Data b) {
    I2C0DAT = b;
  }
  inline I2C::Data getByte0() {
    return I2C0DAT;
  }
  inline void sendStop0() {
    I2C0CONSET = STO;
    // TODO: Should wakeup the waiting task if any!
  }
  inline void clearStart0() {
    I2C0CONCLR = STA;
  }
  inline void clearSi0() {
    I2C0CONCLR = SI;
  }
  
  
  uint32 I2C::getInterrupts() {
    return i2c0State.interrupts;
  }
  
  uint32 I2C::getSpuriousInterrupts() {
    return i2c0State.spuriousInterrupts;
  }
  
  uint32 I2C::getUnknownInterrupts() {
    return i2c0State.unknownInterrupts;
  }
  
  void I2C::configure(uint32 baud) {
    switch (m_port) {
      case PORT_0:
        i2c0State.reset();
        
        // PINSEL, set the GPIO to the I2C function
        PinSelect::enableI2C0();
        
        // Setup VIC for our interrupt callback
        VIC::enable(VIC::I2C0, VEC_I2C0, *this);
        
        // Clear all flags then enable device
        I2C0CONCLR = AA | SI | STA | I2EN;
        I2C0CONSET = I2EN;
        
        // Set up the bus speed
        setBaud(baud);
        break;				
        
      case PORT_1:
        // TODO: Implement
        break;
    }
  }
  
  void I2C::setBaud(uint32 baud) {
    // Set the bitrate, both high and low times.
    const uint32 divisor = PCLK / baud;
    
    switch (m_port) {
      case PORT_0:
        I2C0SCLL = divisor / 2;		// Low time is half the i2c clock
        I2C0SCLH = divisor / 2;		// High time is half the i2c clock
        break;
        
      case PORT_1:
        // TODO: Implement
        break;
    }
  }
  
  bool I2C::isError() const {
    switch (m_port) {
      case PORT_0:
        return i2c0State.isError();
        break;
      case PORT_1:
        return true;
        // TODO: Enable when port 1 is implemented: return i2c1State.isError();
        break;
    }
    return true;
  }
  
  uint8 I2C::transfer(Port port, Direction dir, Address address, const Data* data, uint8 dataLen) {
    uint8 count = 0;		// Return value for the function
    switch (port) {
      case PORT_0:
        // Check there is no transfer under way
        while (i2c0State.isBusy()) {
          // TODO: Wait until not busy. Maybe use a mutex!
        }
        
        // Now a transfer will be under way
        // Also remember the address if we have to resend in from the ISR
        i2c0State.setBusy();
        
        if (dir == RECEIVE)
          i2c0State.address = address | 0x1;		// LSB is 1 for read operation
        else
          i2c0State.address = address;
        
        i2c0State.buffer = const_cast<Data*>(data);
        i2c0State.byteIndex = 0;
        i2c0State.bufferSize = dataLen;
        
        // Clear conditions
        I2C0CONCLR = AA | SI | STA;
        // Enable and wait for the start conddition
        I2C0CONSET = STA;
        
        // Wait until the transmission is received and return the
        //       number of bytes written
        while (i2c0State.isBusy()) {
          // Wait until not busy. TODO: Maybe use a mutex!
        }
        if (i2c0State.isError()) {
          // TODO: Tell the upper layer there was an error
        }
          
        // Save the number of bytes in the transaction
        count = i2c0State.byteIndex;
        // Allow others in the function
        i2c0State.setIdle();
        return count;
        break;
        
      case PORT_1:
        // TODO: Implement
        return 0;
        break;
    }
    return 0;		// Just to please the compiler!
  }
  
  void I2C::handle_irq() {
    ++i2c0State.interrupts;
    if (i2c0State.isBusy()) {
      const uint32 status = I2C0STAT;
      switch (status) {
        case START:
        case RESTART:
          // We are clear to start
          // reset the start bit and transmit the slave address
          I2C0DAT = i2c0State.address;
          I2C0CONCLR = STA;
          break;
          
          // MT_ is Master Transmit
        case MT_SLA_ACK:
          // We have a transmit address ack from the slave. 
          // We can write the data
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            I2C0DAT = i2c0State.buffer[i2c0State.byteIndex];
            ++i2c0State.byteIndex;
          } else {
            // No data bytes. The whole message (0 byte) was then sent
            // Stop the transmission
            I2C0CONSET = STO;
            i2c0State.setDone();
          }
          break;
          
          case MT_SLA_NACK:
          // No acknowledgment from a slave at this address! // TODO: Check if true
          I2C0CONSET = STO;
          // Signal error
          i2c0State.setError();
          break;
          
          case MT_DATA_ACK:
          // Send next byte if we still have any to send
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            I2C0DAT = i2c0State.buffer[i2c0State.byteIndex];	
            ++i2c0State.byteIndex;
          } else {
            // No more data bytes. The whole message was then sent
            // Stop the transmission
            I2C0CONSET = STO;
            i2c0State.setDone();
          }				
          break;
          
          // MR_ is Master Receive
          case MR_SLA_ACK:
          // The slave is ok to transmit
          // Did we expect any data?
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {
            // Send ACK
            I2C0CONSET = AA;					
          } else {
            // Send NACK, no more space to receive data
            I2C0CONCLR = AA;
          }
          break;
          
          case MR_SLA_NACK:
          // The slave has transmitter a NACK.
          // Stop transfer
          I2C0CONSET = STO;
          // Signal the application
          i2c0State.setDone();
          break;
          
          case MR_DATA_ACK:
          // The slave has sent us a byte. Read it
          //  and stop unless we expect more
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            i2c0State.buffer[i2c0State.byteIndex] = I2C0DAT;
            ++i2c0State.byteIndex;
            
            if (i2c0State.byteIndex < i2c0State.bufferSize) {
              // Get ready to receive more. Send ACK
              I2C0CONSET = AA;
            } else {
              // Max bytes to receive reached. Send NACK
              I2C0CONCLR = AA;
            }
          } else {
            // We really should no reach in here since we had signalled
            // that we were done receiving!
            // Stop the transfer!
            I2C0CONCLR = STO;
            // Signal the application
            i2c0State.setError();					
          }			
          break;
          
          case MR_DATA_NACK:
          // The slave is done transmitting with this byte
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            i2c0State.buffer[i2c0State.byteIndex] = I2C0DAT;
            ++i2c0State.byteIndex;
          }
          // Send ACK and stop the transaction
          I2C0CONSET = STO;
          // Signal the application
          i2c0State.setDone();
          break;
          
          default:
          ++i2c0State.unknownInterrupts;
          // Should not get in here! Do something sensible?
          I2C0CONCLR = AA;
          I2C0CONSET = STO;
          // Signal the application
          i2c0State.setError();
          break;
      }
    } else {
      // Mmmm we got an unexpected interrupt. Log this!
      ++i2c0State.spuriousInterrupts;
    }
    
    // In any case clear the i2c interrupt flag
    I2C0CONCLR = SI;
  }

#if 0
  void I2C::I2C0_ISR() {  // TODO: REMOVE
    //IRQ_ENTRY();
    
    ++i2c0State.interrupts;
    if (i2c0State.isBusy()) {
      const uint32 status = I2C0STAT;
      switch (status) {
        case START:
        case RESTART:
          // We are clear to start
          // reset the start bit and transmit the slave address
          I2C0DAT = i2c0State.address;
          I2C0CONCLR = STA;
          break;
          
          // MT_ is Master Transmit
        case MT_SLA_ACK:
          // We have a transmit address ack from the slave. 
          // We can write the data
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            I2C0DAT = i2c0State.buffer[i2c0State.byteIndex];
            ++i2c0State.byteIndex;
          } else {
            // No data bytes. The whole message (0 byte) was then sent
            // Stop the transmission
            I2C0CONSET = STO;
            i2c0State.setDone();
          }
          break;
          
        case MT_SLA_NACK:
          // No acknowledgment from a slave at this address! // TODO: Check if true
          I2C0CONSET = STO;
          // Signal error
          i2c0State.setError();
          break;
          
        case MT_DATA_ACK:
          // Send next byte if we still have any to send
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            I2C0DAT = i2c0State.buffer[i2c0State.byteIndex];	
            ++i2c0State.byteIndex;
          } else {
            // No more data bytes. The whole message was then sent
            // Stop the transmission
            I2C0CONSET = STO;
            i2c0State.setDone();
          }				
          break;
          
          // MR_ is Master Receive
        case MR_SLA_ACK:
          // The slave is ok to transmit
          // Did we expect any data?
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {
            // Send ACK
            I2C0CONSET = AA;					
          } else {
            // Send NACK, no more space to receive data
            I2C0CONCLR = AA;
          }
          break;
          
        case MR_SLA_NACK:
          // The slave has transmitter a NACK.
          // Stop transfer
          I2C0CONSET = STO;
          // Signal the application
          i2c0State.setDone();
          break;
          
        case MR_DATA_ACK:
          // The slave has sent us a byte. Read it
          //  and stop unless we expect more
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            i2c0State.buffer[i2c0State.byteIndex] = I2C0DAT;
            ++i2c0State.byteIndex;
            
            if (i2c0State.byteIndex < i2c0State.bufferSize) {
              // Get ready to receive more. Send ACK
              I2C0CONSET = AA;
            } else {
              // Max bytes to receive reached. Send NACK
              I2C0CONCLR = AA;
            }
          } else {
            // We really should no reach in here since we had signalled
            // that we were done receiving!
            // Stop the transfer!
            I2C0CONCLR = STO;
            // Signal the application
            i2c0State.setError();					
          }			
          break;
          
        case MR_DATA_NACK:
          // The slave is done transmitting with this byte
          if ((i2c0State.bufferSize > 0) && (i2c0State.byteIndex < i2c0State.bufferSize)) {  
            i2c0State.buffer[i2c0State.byteIndex] = I2C0DAT;
            ++i2c0State.byteIndex;
          }
          // Send ACK and stop the transaction
          I2C0CONSET = STO;
          // Signal the application
          i2c0State.setDone();
          break;
          
        default:
          ++i2c0State.unknownInterrupts;
          // Should not get in here! Do something sensible?
          I2C0CONCLR = AA;
          I2C0CONSET = STO;
          // Signal the application
          i2c0State.setError();
          break;
      }
    } else {
      // Mmmm we got an unexpected interrupt. Log this!
      ++i2c0State.spuriousInterrupts;
    }
    
    // In any case clear the i2c interrupt flag
    I2C0CONCLR = SI;
    
    VIC::clear_interrupt();
    
    //IRQ_EXIT();
  }
#endif
}
