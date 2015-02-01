/*
 *  HAL_Processor.h
 *  Embedded
 *
 *  Created by Serge on 17/07/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "HAL_Driver.h"

namespace hal {
  class Processor: public Driver, NoInstance, NoCopy {
  public:
    static void disable_interrupts();
    static void enable_interrupts();
    static uint32 get_cpsr();
  private:
  };
  
  /**
   * A utility to disable interrupts in a C++ lexical scope
   */
  class NoInterrupt: NoCopy {
  public:
    NoInterrupt() {
      hal::Processor::disable_interrupts();      
    }
    ~NoInterrupt() {
      hal::Processor::enable_interrupts();      
    }
  };
}


#if defined(__arm__)
#  if defined(__thumb__)
//
// The following functions are provided in boot-bare.s
//
extern "C" {
  void asm_enable_interrupts();
  void asm_disable_interrupts();
  uint32 asm_get_cpsr();
}

namespace hal {
  inline void Processor::disable_interrupts() {
    asm_disable_interrupts();
  }
  
  inline void Processor::enable_interrupts() {
    asm_enable_interrupts();
  }
  
  inline uint32 Processor::get_cpsr() {
    return asm_get_cpsr();
  }
}
#  else
namespace hal {
  inline void Processor::disable_interrupts() {
    asm volatile (" mrs   r0, cpsr\n"
                  " orr   r0, r0, #0xc0\n"
                  " msr   cpsr, r0\n"
                  :  /* no output */
                  :  /* no input */
                  : "r0"  /* r0 is clobbered */
                  );
  }
  
  inline void Processor::enable_interrupts() {
    asm volatile (" mrs   r0, cpsr\n"
                  " bic   r0, r0, #0xc0\n"
                  " msr   cpsr, r0\n"
                  :  /* no output */
                  :  /* no input */
                  : "r0"  /* r0 is clobbered */
                  );
  }  
  inline uint32 Processor::get_cpsr() {
    uint32 cpsr;
    asm volatile ("  mrs    %0, cpsr \n"
                  : // output
                  "=r"(cpsr)
                  : // input
                  : // clobbered
                  );
    return cpsr;
  }
}
#  endif
#endif

