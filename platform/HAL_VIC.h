/*
 *  HAL_VIC.h
 *  Embedded
 *
 *  Created by Serge on 09/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

#include "lpc214x.h"

#include "HAL_InterruptHandler.h"
namespace hal {
  /**
   * Provides an interface to the LCP21xx VIC.
   * For now this interface deals solely with vectored interrupts, not FIQ.
   */
  class VIC: NoCopy, NoInstance {
public:
    /*
     * The interrupt service routine address type
     */
    typedef void (*ISR)();
    
    /*
     * Devices that can generate interrupts, for use with the status registers.
     */
    enum Device {
      WDT = 0,		// Watchdog timer
      ARMCore0 = 2,
      ARMCore1 = 3,
      TIMER0 = 4,
      TIMER1 = 5,
      UART0 = 6,
      UART1 = 7,
      PWM0 = 8,
      I2C0 = 9,
      SPI0 = 10,
      SPI1 = 11,
      PLL = 12,
      RTC = 13,
      EINT0 = 14,
      EINT1 = 15,
      EINT2 = 16,
      EINT3 = 17,
      AD0 = 18,
      I2C1 = 19,
      BOD = 20,		// Brown out detector
      AD1 = 21,
      USB = 22
    };
    
    enum VICControlRegisterEnableBitMask {
      ENABLE = 1 << 5
    };
    
    /**
     * Sets up a handler for the default interrupt
     * @param handler
     */
    static void set_default_handler(const InterruptHandler& handler);
    
    /**
     * Enables an interrupt for the requested device on a vector
     * @param vectorNum a value between 0 and 15. The lower the number
     *                  the higher the priority
     * @param handler
     */
    static void enable(Device dev, uint8 vectorNum, const InterruptHandler& handler);
    
    /**
     * Disables the vectored interrupt for the given vector
     */
    static void disable(Device dev, uint8 vectorNum);
    
    /**
     * Disables all vectors
     */
    static void disable_all();
    
    /**
      * Gets the interrupt status on the given device and vector
     */
    static bool is_enabled(Device dev);
    
    /**
      * Signal the VIC that the interrupt has been processed.
     * so it can interrupt again.
     * This can be called from an ISR.
     */
    static void clear_interrupt();
    
    /**
     * Changes the protection level of the VIC
     * @param protect if true, any change to the VIC must be made from
     *                a priviliged mode, that is, not user mode.
     *                Of course, disabling the protection must be done
     *                from a privileged mode.
     */
    void enforce_protection(bool protect);
  };
  
  inline void VIC::set_default_handler(const InterruptHandler& handler) {
    *(&VICDefVectAddr) = (typeof(VICDefVectAddr)) &handler;
  }
  
  inline void VIC::enable(Device dev, uint8 vectorNum, const InterruptHandler& handler) {
    // Setup the desired device for the vector
    *( &VICVectCntl0 + vectorNum) = dev | ENABLE;
    // Setup the vector to the ISR
    *(&VICVectAddr0 + vectorNum) = (typeof(VICVectAddr0)) &handler;
    // Begin receiving interrupts
    VICIntEnable = 1 << dev;
  }
  
  inline void VIC::disable(Device dev, uint8 vectorNum) {    
    // Disable interrupts for this device
    VICIntEnClr = 1 << dev;
    // Clear control (device and enabled)
    *( &VICVectCntl0 + vectorNum) = 0;
    // Clear the desired vector for the function
    *(&VICVectAddr0 + vectorNum) = (typeof(VICVectAddr0)) 0;
  }
  
  inline void VIC::disable_all() {
    VICIntEnClr = ~0u;
    for (uint32 vectorNum = 0; vectorNum < 16; ++vectorNum) {
      // Clear control (device and enabled)
      *( &VICVectCntl0 + vectorNum) = 0;
      // Clear the desired vector for the function
      *(&VICVectAddr0 + vectorNum) = (typeof(VICVectAddr0)) 0;      
    }
  }
  
  inline bool VIC::is_enabled(Device dev) {
    return VICIRQStatus & (1 << dev);
  }
  
  inline void VIC::clear_interrupt() {
    VICVectAddr = 0x0;  // Dummy write to clear the VIC interrupt status
  }
  
  inline void VIC::enforce_protection(bool protect) {
    VICProtection = protect;
  }

}
