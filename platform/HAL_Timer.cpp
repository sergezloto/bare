/*
 *  HAL_Timer.cpp
 *  Embedded
 *uint32
 *  Created by Serge on 24/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_Timer.h"
#include "HAL_VIC.h"
#include "HAL_Vectors.h"
#include "HAL_PinSelect.h"

namespace hal {
  // Statics allocation
  Timer::CaptureInfo Timer::captureInfo[NB_TIMERS][NB_CAPTURE_CHANNELS];
  Timer::MatchInfo Timer::matchInfo[NB_TIMERS][NB_MATCH_CHANNELS];

    
  // Structure of interrupt register
  union  structIR {
#if ENABLE_BITFIELD
    struct {
      // 4 bits. bit 0 enables Match channel 0, and so on
      unsigned MR0: 1;
      unsigned MR1: 1;
      unsigned MR2: 1;
      unsigned MR3: 1;
      // 4 bits, bit 0 enables Capture channel0, and so on
      unsigned CR0: 1;
      unsigned CR1: 1;
      unsigned CR2: 1;
      unsigned CR3: 1;
    };
#endif
    uint32 value;
    
    structIR(volatile const structIR& ir)
      : value(ir.value) {
      }
  };
  
  // Structure of timer control register
  union structTCR {
    struct {
      unsigned enable: 1;
      unsigned reset: 1;
    };
    uint8 value;
  };
  
  // TODO: Structure of match control register
  union structMCR {
#if ENABLE_BITFIELD
    struct {
      unsigned int0: 1;
      unsigned reset0: 1;
      unsigned stop0: 1;
      unsigned int1: 1;
      unsigned reset1: 1;
      unsigned stop1: 1;
      unsigned int2: 1;
      unsigned reset2: 1;
      unsigned stop2: 1;
      unsigned int3: 1;
      unsigned reset3: 1;
      unsigned stop3: 1;
    };
#endif
    uint32 value;
  };
  
  // Structure of Capture control register
  union structCCR {
#if ENABLE_BITFIELD
    struct {
      unsigned rising0: 1;
      unsigned falling0: 1;
      unsigned interrupt0: 1;
      unsigned rising1: 1;
      unsigned falling1: 1;
      unsigned interrupt1: 1;
      unsigned rising2: 1;
      unsigned falling2: 1;
      unsigned interrupt2: 1;
      unsigned rising3: 1;
      unsigned falling3: 1;
      unsigned interrupt3: 1;
    };
#endif
    uint32 value;
  };
  
  // Mapping of Timer registers
  struct  TimerRegisterBlock {
    // Interrupt register
    structIR regIR;
    // Timer Control register
    structTCR regTCR;
    // Timer Counter
    uint32 regTC;
    // Prescaler
    uint32 regPR;
    // Prescaler counter
    uint32 regPC;
    // Match control register
    uint32 regMCR;
    // Match registers, 0 to 3
    uint32 regMR[4];
    // Capture control register
    structCCR regCCR;
    // Capture registers, 0 to 3
    uint32 regCR[4];
    // External match register
    uint32 regEMR;
    // Count control register
    uint32 CTCR;
  };
  
  
  // Register mapping for LPC214x
  volatile TimerRegisterBlock & timer0 = *(TimerRegisterBlock*) 0xE0004000;
  volatile TimerRegisterBlock & timer1 = *(TimerRegisterBlock*) 0xE0008000;
    
  uint32 Timer::getTimerValue() const {
    switch (m_timerId) {
      case TIMER0:
        return timer0.regTC;
      case TIMER1:
        return timer1.regTC;
    }
    return 0;
  }
  
  uint32 Timer::getCaptureValue(CaptureChannel channel) const {
    return captureInfo[m_timerId][channel].value;
  }

  bool Timer::configure(uint32 rate) {
    m_rate = rate;
    // Set the prescaler according to the rate
    const uint32 prescaler = PCLK / rate - 1;
    
    // Setup for interrupt
    switch (m_timerId) {
      case TIMER0:
        // Clear the prescaler counter for a clean start
        timer0.regPC = 0;
        timer0.regPR = prescaler;
        VIC::enable(VIC::TIMER0, VEC_TIMER0, *this);
        break;
      case TIMER1:
        // Clear the prescaler counter for a clean start
        timer1.regPC = 0;
        timer1.regPR = prescaler;
        VIC::enable(VIC::TIMER1, VEC_TIMER1, *this);
        break;
    }
    return true;
  }
  
  void Timer::start() {
    switch (m_timerId) {
      case TIMER0:
        timer0.regTCR.enable = 1;
        break;
      case TIMER1:
        timer1.regTCR.enable = 1;
        break;
    }
  }
  
  void Timer::stop() {
    switch (m_timerId) {
      case TIMER0:
        timer0.regTCR.enable = 0;
        break;
      case TIMER1:
        timer1.regTCR.enable = 0;
        break;
    }
  }
  
  void Timer::match(MatchChannel channel, uint32 value, MatchCallback cb, void *cbData) {
    // Set up the match info    
    matchInfo[m_timerId][channel].value = value;
    matchInfo[m_timerId][channel].callback = cb;
    matchInfo[m_timerId][channel].callbackData = cbData;

    switch (m_timerId) {
    case TIMER0:
      // Set up match value
      timer0.regMR[channel] = value;
      // Prepare for interrupt on match
      timer0.regMCR &= ~(0x07 << (3 * channel));  // Clear all actions for this match channel
      timer0.regMCR |= 1 << (3 * channel);   // Set interrupt bit only for now
      break;
      
    case TIMER1:
      // Set up match value
      timer1.regMR[channel] = value;
      // Prepare for interrupt on match
      timer1.regMCR &= ~(0x07 << (3 * channel));  // Clear all actions for this match channel
      timer1.regMCR |= 1 << (3 * channel);   // Set interrupt bit only for now
      break;
    }
  }
  
  void Timer::stop_match(MatchChannel channel) {
    switch (m_timerId) {
      case TIMER0:
        timer0.regMCR &= ~(0x07 << (3 * channel));  // Clear all actions for this match channel
        break;
      case TIMER1:
        timer1.regMCR &= ~(0x07 << (3 * channel));  // Clear all actions for this match channel
        break;
    }
  }

  
  inline int32 getccrmask(Timer::CaptureMode mode) {
    switch (mode) {
      case Timer::RISING:
        return Timer::bit(0) | Timer::bit(2);
      case Timer::FALLING:
        return Timer::bit(1) | Timer::bit(2);
      case Timer::RISING_FALLING:
        return Timer::bit(0) | Timer::bit(1) | Timer::bit(2);
      case Timer::NONE:
      default:
        // The mask is set to 0, which is what is needed.
        
        // We do not revert the pin to GPIO mode. The application
        // will have to do that if desired
        return 0;
    }
  }
  
  void Timer::capture(CaptureChannel channel, CaptureMode mode,
                       CaptureCallback cb, void *cbData) {
    switch (m_timerId) {
      case TIMER0:
        {  
          //volatile TimerRegisterBlock& timer = timer0;
          CaptureInfo& info = captureInfo[TIMER0][channel];
          
          //
          // Prepare for callback
          //
          info.mode = mode;
          info.callback = cb;
          info.callbackData = cbData;
          
          // Determine the bitmask to configure the capture register with.
          // If not none, we enable interrupt for the channel (bit 2)
          const uint32 ccrmask = getccrmask(mode);
          
          // Set up the capture
          T0CCR &= ~(0x7 << (3*channel));
          T0CCR |= ccrmask << (3*channel);
          
          if (mode != NONE) {
            // Enable CAPture function on appropriate pin
            PinSelect::enableCAP0(channel);
          }
        }
        break;

      case TIMER1:
        { 
          //volatile TimerRegisterBlock& timer = timer1;
          CaptureInfo& info = captureInfo[TIMER1][channel];

          //
          // Prepare for callback
          //
          info.mode = mode;
          info.callback = cb;
          info.callbackData = cbData;

          // Determine the bitmask to configure the capture register with.
          // If not none, we enable interrupt for the channel (bit 2)
          const uint32 ccrmask = getccrmask(mode);
        
          // Set up the capture
          T1CCR &= ~(0x7 << (3*channel));
          T1CCR |= ccrmask << (3*channel);

          if (mode != NONE) {
            // Enable CAPture function on appropriate pin
            PinSelect::enableCAP1(channel);
          }
        }        
        break;
    }
  }
  
  void Timer::stop_capture(CaptureChannel channel) {
    switch (m_timerId) {
      case TIMER0:
        // Clear all capture actions
        T0CCR &= ~(0x7 << (3*channel));
        break;
      case TIMER1:
        // Clear all capture actions
        T1CCR &= ~(0x7 << (3*channel));
        break;
    }
  }
  
  void Timer::handle_irq() {    
    const TimerId timerId = m_timerId;
    
    volatile TimerRegisterBlock& timer = (m_timerId == TIMER0) ? timer0 : timer1;
    volatile structIR& ir = timer.regIR;
    
    // Examine the source of interrupt, as per the IR register
    volatile uint32& ir_value = ir.value;
    
    // Timer matches
    for (uint8 channel = MATCH0; channel <= MATCH3; ++channel) {
      const uint8 bitmask = 1 << channel;
      
      if (ir_value & bitmask) {
        // Process Timer match: Nothing to do but call callback
        MatchInfo& info = matchInfo[timerId][channel];
        if (info.callback != 0) {
          info.value = timer.regMR[channel];
          const uint32 new_match_value = info.callback(*this,
                                                       (MatchChannel) channel,
                                                       info);
          if (new_match_value != info.value) {
            timer.regMR[channel] = new_match_value;
          }
          // Ack interrupt source
          ir_value = bitmask;
        }
      }
    }
    
    // Timer captures
    for (uint8 channel = CAP0; channel <= CAP3; ++channel) {
      // The Interrupt Register Capture bits begin at bit 4
      const uint8 bitmask = 1 << (4 + channel);
      
      if (ir_value & bitmask) {
        // Process Timer capture
        const uint32 captureValue = timer.regCR[channel];
        
        CaptureInfo& info = captureInfo[timerId][channel];
        
        // Remember value since it can be queried
        info.value = captureValue;
        // Callback if any was requested
        if (info.callback != 0) {
          const CaptureAction captureAction = info.callback(*this, 
                                                            (CaptureChannel) channel,
                                                            info);
          if (captureAction != NO_CHANGE) {
            // Setup as requested by the callback
            switch (captureAction) {
              case SET_RISING_EDGE:
                info.mode = RISING;
                break;
              case SET_FALLING_EDGE:
                info.mode = FALLING;
                break;
              case SET_BOTH_EDGES:
                info.mode = RISING_FALLING;
                break;
              case STOP_CAPTURE:
              default:
                info.mode = NONE;
                break;
            }
            
            //T0CCR &= ~(7 << (3*channel));
            //T0CCR |= getccrmask(info.mode) << (3*channel);
            
            timer.regCCR.value &= ~(7 << (3*channel));
            timer.regCCR.value |= getccrmask(info.mode) << (3*channel);
          }
        } // Done with the callback
        
        // Clear interrupt source
        //T0IR = bitmask;
        ir.value = bitmask;
      }
    }
  }
}

