/*
 *  HAL_PinSelect.h
 *  Embedded
 *
 *  Created by Serge on 17/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace hal {
  /**
   * Microcontrollers support multiple functions for most of their i/o pins.
   * This class allows to select these functions.
   * Note: if two function that share a pin are enabled, the last one that
   * was enabled wins.
   */
  class PinSelect: NoCopy, NoInstance {
  public:
    /**
     * Enables the GPIO as follows:
     * - port 0, pins 0 to 30 (but pin 24 to 27 inclusive are reserved)
     * - port 1	 is special, that is, its pins cannot be enabled individually and
     *  only pins from 16 to 31 exist at all
     *  In practice we don't change the function for these pins because the 
     *  alternate functions are TRACE and JTAGS, which we don't use
     *  so do nothing for port 1!
     */
    static void enableGPIO(uint8 port, uint8 pin);
    
    /*
     * Enables the ADC on the given device port and channel.
     * @param adc 0 or 1 for the LPC21xx
     * @param channel 0 to 7.
     *                For device 0, channels 0 and 5 do not exist on lpc214x
     */
    static void enableADC(uint8 adc, uint8 channel);
    
    /**
     * Enables the DAC pin
     */
    static void enableDAC();
    
    /**
     * Disables the DAC pin
     */
    static void disableDAC();

    /**
     * @param PWM device 1 to 6
     */
    static void enablePWM(uint8 port);
    
    /**
     * Enables the UART0 RxD and TxD
     */
    static void enableUART0();
    
    /**
     * Enables the UART1 RxD and TxD.
     * Note: the modem functions, RTS, DTS, RI etc are not enabled
     */
    static void enableUART1();
    
    /**
     * Enables the i2c device 0
     */
    static void enableI2C0();
    
    /**
     * Enables the i2c device 1
     */
    static void enableI2C1();
    
    /**
     * Enables the SPI device 0
     */
    static void enableSPI0();
    
    /**
     * Enables the SPI device 1
     */		
    static void enableSPI1();
    
    /*
     * Enables the capture function of timer 0 on a given channel
     * The pins retained for each channel are:
     * 0: P0.22
     * 1: P0.4
     * 2: P0.28
     * 3: P0.29
     */
    static void enableCAP0(uint8 channel);
    
    /*
     * Enables the capture function of timer 1 on a given channel
     * The pins retained for each channel are:
     * 0: P0.10
     * 1: P0.11
     * 2: P0.17
     * 3: P0.18
     */
    static void enableCAP1(uint8 channel);
    
  private:
    // Bit definitions
    static const uint8 b00 = 0x0, b01 = 0x1, b10 = 0x2, b11 = 0x3;
    
    /**
     * manipulates the PINSELx registers by giving a function number to
     * a pin
     * @param function the function number, between 0 and 3
     */
    static void enable(uint8 port, uint8 pin, uint8 function);
  };

  inline
  void PinSelect::enableGPIO(uint8 port, uint8 pin) {
    enable(port, pin, 0x0);		// Function 0 is GPIO!
  }
  
  inline
  void PinSelect::enableADC(uint8 adc, uint8 channel) {
    // Both ADC are shared with the gpio port 0
    const uint8 port = 0;
    
    switch (adc) {
      case 0:
        switch (channel) {
          case 0:
            // No AD0.0 pin for lpc214x, but okay for lpc213x!
            enable(port, 27, b01);
            break;
          case 1:
            enable(port, 28, b01);
            break;
          case 2:
            enable(port, 29, b01);
            break;
          case 3:
            enable(port, 30, b01);
            break;
          case 4:
            enable(port, 25, b01);
            break;
          case 5:
            // NO AD0.5 pin!
            break;
          case 6:
            enable(port, 4, b11);
            break;
          case 7:
            enable(port, 5, b11);
            break;
          default:
            break;
        }
        break;
        
      case 1:
        switch (channel) {
          case 0:
            enable(port, 6, b11);
            break;
          case 1:
            enable(port, 8, b11);
            break;
          case 2:
            enable(port, 10, b11);
            break;
          case 3:
            enable(port, 12, b11);
            break;
          case 4:
            enable(port, 13, b11);
            break;
          case 5:
            enable(port, 15, b11);
            break;
          case 6:
            enable(port, 21, b10);
            break;
          case 7:
            enable(port, 22, b01);
            break;
          default:
            break;
        }
        break;
    }
  }
  
  inline
  void PinSelect::enableDAC() {
    enable(0, 25, b10);
  }
  
  inline
  void PinSelect::disableDAC() {
    enable(0, 25, b00);
  }
  
  inline
  void PinSelect::enablePWM(uint8 port) {
    switch (port) {
      case 1:
        enable(0, 0, b10);		// p0.0: b10 is the PWM function
        break;
      case 2:
        enable(0, 7, b10);		// p0.7: b10 is the PWM function
        break;
      case 3:
        enable(0, 1, b10);		// p0.1: b10 is the PWM function
        break;
      case 4:
        enable(0, 8, b10);		// p0.8: b10 is the PWM function
        break;
      case 5:
        enable(0, 21, b01);		// p0.21: b01 is the PWM function			
        break;
      case 6:
        enable(0, 9, b10);		// p0.9: b10 is the PWM function
        break;
      default:
        // No such device!
        break;
    }
  }
  
  inline
  void PinSelect::enableUART0() {
    enable(0, 0, 0x01);		// P0.0 set to UART0 TxD
    enable(0, 1, 0x01);		// P0.1 set to UART0 RxD
  }
  
  inline
  void PinSelect::enableUART1(){
    enable(0, 8, 0x01);		// P0.8 set to UART1 TxD
    enable(0, 9, 0x01);		// P0.9 set to UART1 RxD		
  }
  
  inline
  void PinSelect::enableI2C0() {
    enable(0, 2, 0x1);		// P0.2 set to SCL0 for I2C0
    enable(0, 3, 0x1);		// P0.3 set to SDA0 for I2C0
  }
  
  inline
  void PinSelect::enableI2C1() {
    enable(0, 11, 0x3);		// P0.11 set to SCL1 for I2C1
    enable(0, 14, 0x3);		// P0.14 set to SDA1 for I2C1		
  }
  
  inline
  void PinSelect::enableSPI0() {
    enable(0, 4, 0x1);		// P0.4 set to SCK0 for SPI0
    enable(0, 5, 0x1);		// P0.5 set to MISO0 for SPI0
    enable(0, 6, 0x1);		// P0.6 set to MOSI0 for SPI0
    enable(0, 7, 0x1);		// P0.7 set to SSEL0 for SPI0
  }
  
  inline
  void PinSelect::enableSPI1() {
    enable(0, 17, 0x2);		// P0.17 set to SCK0 for SPI0
    enable(0, 18, 0x2);		// P0.18 set to MISO0 for SPI0
    enable(0, 19, 0x2);		// P0.19 set to MOSI0 for SPI0
    enable(0, 20, 0x2);		// P0.20 set to SSEL0 for SPI0
  }
  
  inline
  void PinSelect::enableCAP0(uint8 channel) {
    switch (channel) {
      case 0:
        PinSelect::enable(0, 30, b11);
        break;
      case 1:
        PinSelect::enable(0, 02, b10);
        break;
      case 2:
        PinSelect::enable(0, 28, b10);
        break;
      case 3:
        PinSelect::enable(0, 29, b10);
        break;
    }
  }
  
  inline
  void PinSelect::enableCAP1(uint8 channel) {
    switch (channel) {
      case 0:
        PinSelect::enable(0, 10, b10);
        break;
      case 1:
        PinSelect::enable(0, 11, b10);
        break;
      case 2:
        PinSelect::enable(0, 17, b01);
        break;
      case 3:
        PinSelect::enable(0, 18, b01);
        break;
    }
  }
}

