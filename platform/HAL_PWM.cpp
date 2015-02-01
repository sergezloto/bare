/*
 *  HAL_PWM.cpp
 *  Embedded
 *
 *  Created by Serge on 5/2/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#include "HAL_PWM.h"
#include "HAL_VIC.h"
#include "HAL_Vectors.h"

using namespace hal;

namespace lpc214x {
  volatile PWM::PWMRegisterBlock& PWM::pwm = *(PWMRegisterBlock*) 0xE0014000;
  
  PWM::MatchInfo PWM::m_match_info[NB_PWM_CHANNELS];
  
  void PWM::match(PWMChannel channel,
                  uint32 value,
                  MatchCallback cb, void *cbData) {
    // Populate the match info
    m_match_info[channel].callbackData = cbData;
    m_match_info[channel].callback = cb;
    
    if (cb) {
      // Set the interrupt vector for PWM if not done
      if (! VIC::is_enabled(VIC::PWM0)) {
        VIC::enable(VIC::PWM0, VEC_PWM0, *this);
      }
    }
    
    const bool v = cb != 0;
    
    switch (channel) {
      case CHAN0:
        pwm.regMCR.int0 = v;
        break;
      case CHAN1:
        pwm.regMCR.int1 = v;
        break;
      case CHAN2:
        pwm.regMCR.int2 = v;
        break;
      case CHAN3:
        pwm.regMCR.int3 = v;
        break;
      case CHAN4:
        pwm.regMCR.int4 = v;
        break;
      case CHAN5:
        pwm.regMCR.int5 = v;
        break;
      case CHAN6:
        pwm.regMCR.int6 = v;
        break;
    }
  }  
  
  /**
   * Handles PWM interrupts
   */
  void PWM::handle_irq() {
    structIR ir;
    ir.value = pwm.regIR.value;
    
    if (ir.MR0) {
      MatchInfo& mi = m_match_info[CHAN0];
      if (mi.callback) {
        const uint32 new_value = mi.callback(CHAN0, mi);
        if (new_value != mi.value) {
          mi.value = new_value;
          set_pulse(CHAN1, new_value);
        }
      }
    }
    if (ir.MR1) {
      MatchInfo& mi = m_match_info[CHAN1];
      if (mi.callback) {
        const uint32 new_value = mi.callback(CHAN1, mi);
        if (new_value != mi.value) {
          mi.value = new_value;
          set_pulse(CHAN1, new_value);
        }
      }
    }
    if (ir.MR2) {
      MatchInfo& mi = m_match_info[CHAN2];
      if (mi.callback) {
        const uint32 new_value = mi.callback(CHAN2, mi);
        if (new_value != mi.value) {
          mi.value = new_value;
          set_pulse(CHAN2, new_value);
        }
      }
    }
    if (ir.MR3) {
      MatchInfo& mi = m_match_info[CHAN3];
      if (mi.callback) {
        const uint32 new_value = mi.callback(CHAN3, mi);
        if (new_value != mi.value) {
          mi.value = new_value;
          set_pulse(CHAN3, new_value);
        }
      }
    }
    if (ir.MR4) {
      MatchInfo& mi = m_match_info[CHAN4];
      if (mi.callback) {
        const uint32 new_value = mi.callback(CHAN4, mi);
        if (new_value != mi.value) {
          mi.value = new_value;
          set_pulse(CHAN4, new_value);
        }
      }
    }
    if (ir.MR5) {
      MatchInfo& mi = m_match_info[CHAN5];
      if (mi.callback) {
        const uint32 new_value = mi.callback(CHAN5, mi);
        if (new_value != mi.value) {
          mi.value = new_value;
          set_pulse(CHAN5, new_value);
        }
      }
    }
    if (ir.MR6) {
      MatchInfo& mi = m_match_info[CHAN6];
      if (mi.callback) {
        const uint32 new_value = mi.callback(CHAN6, mi);
        if (new_value != mi.value) {
          mi.value = new_value;
          set_pulse(CHAN6, new_value);
        }
      }
    }
    
    // Reset the interrupts for all channels
    pwm.regIR.value = ir.value;
  }
}
