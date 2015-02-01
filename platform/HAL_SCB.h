/*
 *  HAL_SCB.h
 *  Embedded
 *
 *  Created by Serge on 08/07/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "HAL_Driver.h"

#include "lpc214x.h"

namespace hal {
  /**
   * System control block. Sets up the Memory Accelerator Module, the PLL, and 
   * the peripheral clock speed.
   */
  class SCB: public Driver, NoCopy, NoInstance {
public:
    static volatile uint32& RSIR;
    static void configure();
    
    static void disconnect_pll();

    static uint32 get_reset_source();
    static void clear_reset_source();

    enum Mapping {
      MAP_BOOT_BLOCK = 0,
      MAP_FLASH = 1,
      MAP_RAM = 2
    };
    
    enum ResetSource {
      RS_POR = 1 << 0,
      RS_EXTR = 1 << 1,
      RS_WDTR = 1 << 2,
      RS_BODR = 1 << 3
    };
    
    enum PowerControl {
      PWR_TIMER0	=	1 << 1,
      PWR_TIMER1	=	1 << 2,
      PWR_UART0	=	1 << 3,
      PWR_UART1	=	1 << 4,
      PWR_PWM0	=	1 << 5,
      PWR_I2C0	=	1 << 7,
      PWR_SPI10	=	1 << 8,
      PWR_RTC	=	1 << 9,
      PWR_SPI1	=	1 << 10,
      PWR_ADC0	=	1 << 12,
      PWR_I2C1	=	1 << 19,
      PWR_ADC1	=	1 << 20
    };

    static void configureMAP(Mapping mapping);
    
    static void set_power_control(uint32 power_control_mask) {
      PCONP = power_control_mask;
    }

    
private:
    static void configurePLL();
    static void configureMAM();
    static void configureVPB();
    static void feed();
  };
    
  inline
  void SCB::configure() {
    configureMAM();
    configureVPB();
    configurePLL();
  }
  
  inline
  uint32 SCB::get_reset_source() {
    return RSIR;
  }
  
  inline
  void SCB::clear_reset_source() {
    RSIR = 0;
  }
  
  inline 
  void SCB::configurePLL() {
    // Assuming 12 MHz crystal
    const unsigned PSEL = 1;  // Get to an acceptable FCCO
    const unsigned MSEL = 4;  // Multiply by 5 to get 60 MHz
    PLL0CFG = ( (PSEL << 5) | MSEL );
    
    // Enable the PLL
    PLL0CON = 1;
    feed();
    
    /* Wait for the PLL to lock... */
    const uint16 PLOCKED = bit(10);
    while( !( PLL0STAT & PLOCKED ) );
    
    // ...and connect it 
    PLL0CON = 3;
    feed();
  }
  
  inline
  void SCB::disconnect_pll() {
    PLL0CON &= bit(1);
    feed();
  }
  
  inline 
  void SCB::configureMAM() {
    enum Mode {
      DISABLED = 0,
      PARTIAL = 1,
      FULL = 2
    };
    
    // Number of fetch cycles
    MAMTIM = 3;
    MAMCR = FULL;    // But see http://ics.nxp.com/support/documents/microcontrollers/pdf/errata.lpc2138.01.pdf
  }
  
  inline 
  void SCB::configureVPB() {
    enum Div {
      ONE_FOURTH = 0,
      SAME = 1,
      ONE_HALF = 2
    };
    // For now same frequency as PLL
    VPBDIV = SAME;
  }
  
  inline 
  void SCB::configureMAP(Mapping mapping) {
    MEMMAP = mapping;
  };
  
  inline void SCB::feed() {
    PLL0FEED = (uint8)0xaa;
    PLL0FEED = (uint8)0x55;
  }
}
