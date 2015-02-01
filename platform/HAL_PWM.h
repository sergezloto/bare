/*
 *  HAL_PWM.h
 *  Embedded
 *
 *  Created by Serge on 5/2/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */
#pragma once
#include "base.h"
#include "HAL_Driver.h"
#include "HAL_InterruptHandler.h"
#include "HAL_PinSelect.h"

namespace lpc214x {
  
  /**
   * LPC214x PWM function.
   * Implemented for single edge PWM.
   */
  class PWM: public hal::Driver, public hal::InterruptHandler, NoCopy {
  public:
    
    static const uint32 NB_PWM_CHANNELS = 7;	// 6 channels plus MR0

    /**
     * As per NXP pinout, the PWM channels are numbered 1 to 6
     */
    enum PWMChannel {
      CHAN0 = 0,	// This one is special, as in single-edge mode, it control all rising edges
      CHAN1 = 1,
      CHAN2 = 2,
      CHAN3 = 3,
      CHAN4 = 4,
      CHAN5 = 5,
      CHAN6 = 6,
    };
    
    /**
     * Definition for effecting multiple changes simultaneously
     */
    union PWMChannelMask {
      explicit PWMChannelMask(uint32 mask) : value(mask) {
      }
      
      struct {
        unsigned chan0: 1;
        unsigned chan1: 1;
        unsigned chan2: 1;
        unsigned chan3: 1;
        unsigned chan4: 1;
        unsigned chan5: 1;
        unsigned chan6: 1;
      };
      uint32 value;
    };
    
    struct MatchInfo;
    /**
     * A callback run upon match. Its return value is the new match value
     */
    typedef uint32 (*MatchCallback)(PWMChannel channel, 
                                    const MatchInfo& info);
    
    struct MatchInfo {
      MatchInfo() : value(0), callback(0), callbackData(0) {
      }
      uint32 value;
      MatchCallback callback;
      void *callbackData;
    };
  
    /**
     * Constructor
     */
    PWM();
    
    /**
     * Sets up the PWM for counting at a given rate.
     * Note: Interrupt generation is *not* enabled by this function.
     * @param rate timer frequency in Hz
     */
    bool configure(uint32 rate);
    
    /**
     * Sets the cycle length of the PWM unit
     * @param the cycle duration in PWM ticks
     */
    void set_cycle(uint32 cycle);
    
    /**
     * @return the cycle length, in pwm timer units
     */
    uint32 get_cycle() const;
    
    /**
     * Starts the PWM
     * @param cycle the number of ticks of the PWM cycle, in the chosen rate (see \fn configure(uint32 rate))
     */
    void start(uint32 cycle);
    
    /**
     * Halts the PWM
     */
    void stop();
    
    /**
     * Enables generation on the given PWM channel, in single edge mode.
     * The initial pulse value is 0, that is, the signal remains low.
     * The PWM function is enabled for the pin corresponding to the channel.
     */
    void enable(PWMChannel channel);
    
    /**
     * Enables the generation of PWM on several PWM channels simultaneously
     */
    void enable_mask(PWMChannelMask channel_mask);
    
    /**
     * Set the on duty length as a percentage of the cycle length
     */
    void set_duty_percent(PWMChannel channel, uint32 percent);
    
    /**
     * Sets the duty percentage on several channels simultaneously
     */
    void set_duty_percent_mask(PWMChannelMask channel_mask, uint32 percent);
    
    /**
     * Sets the pulse length, in single edge mode, for the given PWM channel.
     * @param channel the PWM channel to use
     * @param the pulse duration in the unit set up with the \fn configure(uint32 rate) function.
     */
    void set_pulse(PWMChannel channel, uint32 pulse);
    
    /**
     * Set the pulse length for several PWM channels
     */
    void set_pulse_mask(PWMChannelMask channel_mask, uint32 pulse);
    
    /**
     * @return the pulse value for the given channel, in PWM timer units
     */
    uint32 get_pulse(PWMChannel channel) const;
    
    /**
     * Stops a single edge PWM
     */
    void disable(PWMChannel channel);
    
    /**
     * Stops multiple single edge PWM
     */
    void disable_mask(PWMChannelMask channel_mask);
    
    /**
     * Sets up match for a given channel.
     * @param value the value the counter matches against
     * @param cb the callbackfunction to execute upon match
     * @param cbData data to be passed to the callback
     */
    void match(PWMChannel channel,
               uint32 value,
               MatchCallback cb, void *cbData = 0);
    
    /**
     * @return the current counter value
     */
    uint32 getCounterValue() const;
    
    /**
     * @return the value of the latch register
     */
    uint32 getLER() const;
    
  protected:
    void handle_irq();
    
  private:
    /**
     * Interrupt register
     */
    union structIR {
      uint32 value;
      struct {
        unsigned MR0: 1;
        unsigned MR1: 1;
        unsigned MR2: 1;
        unsigned MR3: 1;
        unsigned : 4; // ignore
        unsigned MR4: 1;
        unsigned MR5: 1;
        unsigned MR6: 1;
      };
    };
    
    /**
     * Timer control register
     */
    union structTCR {
      uint32 value;
      struct {
        unsigned enable: 1;
        unsigned reset: 1;
        unsigned: 1; // ignore
        unsigned pwmenable: 1;
      };
      enum {
        ENABLE = 1, RESET = 2, PWMENABLE = 8
      };
    };
    
    /**
     * PWM Match control register
     */
    union structMCR {
      uint32 value;
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
        unsigned int4: 1;
        unsigned reset4: 1;
        unsigned stop4: 1;
        unsigned int5: 1;
        unsigned reset5: 1;
        unsigned stop5: 1;
        unsigned int6: 1;
        unsigned reset6: 1;
        unsigned stop6: 1;
      };
      enum {
        INT0 = 1 << 0, RESET0 = 1 << 1, STOP0 = 1 << 2,
        INT1 = 1 << 3, RESET1 = 1 << 4, STOP1 = 1 << 5,
        INT2 = 1 << 6, RESET2 = 1 << 7, STOP2 = 1 << 8,
        INT3 = 1 << 9, RESET3 = 1 << 10, STOP3 = 1 << 11,
        INT4 = 1 << 12, RESET4 = 1 << 13, STOP4 = 1 << 14,
        INT5 = 1 << 15, RESET5 = 1 << 16, STOP5 = 1 << 17,
        INT6 = 1 << 18, RESET6 = 1 << 19, STOP6 = 1 << 20
      };
    };
    
    /**
     * PWM Control register
     */
    union structPCR {
      uint32 value;
      struct {
        unsigned: 2; // ignore
        unsigned sel2: 1;  // set to 1 for double edge PWM2
        unsigned sel3: 1;  // set to 1 for double edge PWM3
        unsigned sel4: 1;  // set to 1 for double edge PWM4
        unsigned sel5: 1;  // set to 1 for double edge PWM5
        unsigned sel6: 1;  // set to 1 for double edge PWM6
        unsigned: 2;  // ignore
        unsigned enable1: 1;  // PWM 1 output enabled if 1
        unsigned enable2: 1;  // PWM 2 output enabled if 1
        unsigned enable3: 1;  // PWM 3 output enabled if 1
        unsigned enable4: 1;  // PWM 4 output enabled if 1
        unsigned enable5: 1;  // PWM 5 output enabled if 1
        unsigned enable6: 1;  // PWM 6 output enabled if 1
      };
      enum {
        SEL2 = 1 << 2, SEL3 = 1 << 3, SEL4 = 1 << 4, SEL5 = 1 << 5, SEL6 = 1 << 6,
        ENABLE1 = 1 << 9, ENABLE2 = 1 << 10, ENABLE3 = 1 << 11, ENABLE4 = 1 << 12, ENABLE5 = 1 << 13, ENABLE6 = 1 << 14 
      };
    };
 
    /**
     * Latch enable register
     */
    union structLER {
      uint32 value;
      struct {
        unsigned latch0: 1;
        unsigned latch1: 1;
        unsigned latch2: 1;
        unsigned latch3: 1;
        unsigned latch4: 1;
        unsigned latch5: 1;
        unsigned latch6: 1;
      };
      enum {
        LATCH0 = 1 << 0, LATCH1 = 1 << 1, LATCH2 = 1 << 2, LATCH3 = 1 << 3,
        LATCH4 = 1 << 4, LATCH5 = 1 << 5, LATCH6 = 1 << 6
      };
    };
    
    // Mapping of PWM registers
    struct  PWMRegisterBlock {
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
      structMCR regMCR;
      // Match registers, 0 to 3
      uint32 regMRA[4];      
      uint32 ignore[6];  // There is a big hole in the register map!
      // Match registers, 4 to 6
      uint32 regMRB[3];
      // PWM Control register
      structPCR regPCR;
      // PWM Latch Control register
      structLER regLER;
    };
    
    static volatile PWMRegisterBlock &pwm; //= *(PWMRegisterBlock*) 0xE0014000;
    
    static MatchInfo m_match_info[NB_PWM_CHANNELS];
  };
  
  /* ---------------------------------------------------------- */
  inline
  PWM::PWM() {
    // Nothing to do
  }
  
#if 1 || EXPERIMENT
#  include "../platform/lpc214x.h"
#endif
  
  inline
  bool PWM::configure(uint32 rate) {
    const uint32 prescaler = PCLK / rate - 1;
    
    pwm.regTCR.value = structTCR::RESET;
    pwm.regPR = prescaler;
    
    return true;
  }
  
  inline
  void PWM::set_cycle(uint32 cycle) {
    if (pwm.regMRA[0] == cycle) {
      // SETTING MR0 TO THE SAME VALUE SEEMS TO INTRODUCE PWM GLITCHES
      return;
    }
    // Set up the cycle length, by setting MR0 to reset the counter
    pwm.regMRA[0] = cycle;
    pwm.regLER.value |= structLER::LATCH0;
  }
  
  inline
  uint32 PWM::get_cycle() const {
    return pwm.regMRA[0];
  }

  inline
  void PWM::start(uint32 cycle) {

     // Set up the cycle length, by setting MR0 to reset the counter
    pwm.regMRA[0] = cycle;
    
#if 1
    // Reset counter on match 0. Not strictly necessary in PWM mode?
    pwm.regMCR.value = structMCR::RESET0;
    // No need to latch the MR0 write, because the unit is not in PWM mode, so 
    // shadow MR and LER are not operational yet
    //pwm.regLER.value = structLER::LATCH0;
#endif
    // Set in PWM mode, reset and start the counter
    pwm.regTCR.value = structTCR::ENABLE | structTCR::PWMENABLE;
    
    // Latch all shadow match registers, so that their initial value (0) is effective
    pwm.regLER.value = structLER::LATCH0 | structLER::LATCH1 | structLER::LATCH2 | structLER::LATCH3 | structLER::LATCH4 | structLER::LATCH5 | structLER::LATCH6;
  }

  inline
  void PWM::stop() {
    pwm.regTCR.enable = 0;
  }
  
  inline
  void PWM::enable(PWMChannel channel) {
    switch (channel) {
      case CHAN1: pwm.regPCR.enable1 = 1; break;
      case CHAN2: pwm.regPCR.enable2 = 1; break;
      case CHAN3: pwm.regPCR.enable3 = 1; break;
      case CHAN4: pwm.regPCR.enable4 = 1; break;
      case CHAN5: pwm.regPCR.enable5 = 1; break;
      case CHAN6: pwm.regPCR.enable6 = 1; break;
      default:
        return;
    }
    // enable the output
    hal::PinSelect::enablePWM(channel);
  }
  
  inline
  void PWM::enable_mask(PWMChannelMask channel_mask) {
    structPCR pcr;
    pcr.value = 0;
    if (channel_mask.chan1) {
      pcr.enable1 = 1;
    }
    if (channel_mask.chan2) {
      pcr.enable2 = 1;
    }
    if (channel_mask.chan3) {
      pcr.enable3 = 1;
    }
    if (channel_mask.chan4) {
      pcr.enable4 = 1;
    }
    if (channel_mask.chan5) {
      pcr.enable5 = 1;
    }
    if (channel_mask.chan6) {
      pcr.enable6 = 1;
    }
    pwm.regPCR.value |= pcr.value;
    
    // Enable the outputs
    if (channel_mask.chan1) {
      hal::PinSelect::enablePWM(1);
    }
    if (channel_mask.chan2) {
      hal::PinSelect::enablePWM(2);
    }
    if (channel_mask.chan3) {
      hal::PinSelect::enablePWM(3);
    }
    if (channel_mask.chan4) {
      hal::PinSelect::enablePWM(4);
    }
    if (channel_mask.chan5) {
      hal::PinSelect::enablePWM(5);
    }
    if (channel_mask.chan6) {
      hal::PinSelect::enablePWM(6);
    }   
  }
  
  inline
  void PWM::set_pulse(PWMChannel channel, uint32 pulse) {
    switch (channel) {
      case CHAN1: 
        if (pulse != pwm.regMRA[1]) {
          pwm.regMRA[1] = pulse;
          pwm.regLER.value |= structLER::LATCH1;
        }
        break;
      case CHAN2: 
        if (pulse != pwm.regMRA[2]) {
          pwm.regMRA[2] = pulse;
          pwm.regLER.value |= structLER::LATCH2;
        }
        break;
      case CHAN3: 
        if (pulse != pwm.regMRA[3]) {
          pwm.regMRA[3] = pulse; 
          pwm.regLER.value |= structLER::LATCH3;
        }
        break;
      case CHAN4: 
        if (pulse != pwm.regMRB[0]) {
          pwm.regMRB[0] = pulse; 
          pwm.regLER.value |= structLER::LATCH4;
        }
        break;
      case CHAN5: 
        if (pulse != pwm.regMRB[1]) {
          pwm.regMRB[1] = pulse;
          pwm.regLER.value |= structLER::LATCH5;
        }
        break;
      case CHAN6: 
        if (pulse != pwm.regMRB[2]) {
          pwm.regMRB[2] = pulse;
          pwm.regLER.value |= structLER::LATCH6;
        }
        break;
      default:
        return;
    }
  }
  
  inline
  void PWM::set_pulse_mask(PWMChannelMask channel_mask, uint32 pulse) {    
    structLER ler;
    ler.value = 0;
    if (channel_mask.chan1) {
      if (pulse != pwm.regMRA[1]) {
        pwm.regMRA[1] = pulse; 
        ler.value |= structLER::LATCH1;
      }
    }
    if (channel_mask.chan2) {
      if (pulse != pwm.regMRA[2]) {
        pwm.regMRA[2] = pulse; 
        ler.value |= structLER::LATCH2;
      }
    }
    if (channel_mask.chan3) {
      if (pulse != pwm.regMRA[3]) {
        pwm.regMRA[3] = pulse; 
        ler.value |= structLER::LATCH3;
      }
    }
    if (channel_mask.chan4) {
      if (pulse != pwm.regMRB[0]) {
        pwm.regMRB[0] = pulse; 
        ler.value |= structLER::LATCH4;
      }
    }
    if (channel_mask.chan5) {
      if (pulse != pwm.regMRB[1]) {
        pwm.regMRB[1] = pulse; 
        ler.value |= structLER::LATCH5;
      }
    }
    if (channel_mask.chan6) {
      if (pulse != pwm.regMRB[2]) {
        pwm.regMRB[2] = pulse; 
        ler.value |= structLER::LATCH6;
      }
    }

    pwm.regLER.value |= ler.value;
  }
  
  inline
  uint32 PWM::get_pulse(PWMChannel channel) const {
    switch (channel) {
      case CHAN0: return pwm.regMRA[0];
      case CHAN1: return pwm.regMRA[1];
      case CHAN2: return pwm.regMRA[2];
      case CHAN3: return pwm.regMRA[3];
      case CHAN4: return pwm.regMRB[0];
      case CHAN5: return pwm.regMRB[1];
      case CHAN6: return pwm.regMRB[2];
    }
    return 0;
  }


  inline
  void PWM::set_duty_percent(PWMChannel channel, uint32 percent) {
    const uint32 cycle_length = get_cycle();
    const uint32 pulse = (cycle_length * percent) / 100;
    set_pulse(channel, pulse);
  }
  
  inline
  void PWM::set_duty_percent_mask(PWMChannelMask channel_mask, uint32 percent) {
    const uint32 cycle_length = get_cycle();
    const uint32 pulse = (cycle_length * percent) / 100;
    set_pulse_mask(channel_mask, pulse);
  }
  
  inline
  void PWM::disable(PWMChannel channel) {
    switch (channel) {
      case CHAN0: /* Nothing */ break;
      case CHAN1: pwm.regPCR.enable1 = 0; break;
      case CHAN2: pwm.regPCR.enable2 = 0; break;
      case CHAN3: pwm.regPCR.enable3 = 0; break;
      case CHAN4: pwm.regPCR.enable4 = 0; break;
      case CHAN5: pwm.regPCR.enable5 = 0; break;
      case CHAN6: pwm.regPCR.enable6 = 0; break;
    }
  }
  
  inline
  void PWM::disable_mask(PWMChannelMask channel_mask) {
    // We zero the bits in the PWM Control Register that are marked in the channel mask
    pwm.regPCR.value &= ~channel_mask.value;
  }
  
  inline
  uint32 PWM::getCounterValue() const {
    return pwm.regTC;
  }
  
  inline
  uint32 PWM::getLER() const {
    return pwm.regLER.value;
  }
}

