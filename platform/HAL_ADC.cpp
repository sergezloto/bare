/*
 *  HAL_ADC.cpp
 *  Embedded
 *
 *  Created by Serge on 10/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_ADC.h"
#include "HAL_PinSelect.h"
#include "HAL_VIC.h"
#include "HAL_Vectors.h"

#include "lpc214x.h"

// If 1, a single interrupt is generated for the highest enabled channel
#define ADC_SINGLE_INTERRUPT 1

namespace hal {
  
  /*
   * Breakout for AD control register
   */
  union ControlRegBits {
    uint32 value;
    struct  {
      unsigned sel	: 8;		// Mask that specifies active channels
      unsigned clkdiv	: 8;		// A divisor. Set up so pclk / div <= 4.5 MHz
      unsigned burst	: 1;		// 1 if the converter is to convert repeatedly, unattended
      /*     
       This field selects the number of clocks used for each conversion in Burst mode, and the 
       number of bits of accuracy of the result in the RESULT bits of ADDR, between 11 clocks 
       (10 bits) and 4 clocks (3 bits). 
       000 11 clocks / 10 bits 
       001 10 clocks / 9bits 
       010 9 clocks / 8 bits 
       011 8 clocks / 7 bits 
       100 7 clocks / 6 bits 
       101 6 clocks / 5 bits 
       110 5 clocks / 4 bits 
       111 4 clocks / 3 bits      
       */
      unsigned clks		: 3;		// Controls the precision of the adc: 000 for 10 bits, 111 for 3 bits
      unsigned reserved0	: 1;
      unsigned pdn		: 1;		// 0 for power down, 1 for adc operational
      unsigned reserved1	: 2;
      /*
       When the BURST bit is 0, these bits control whether and when an A/D conversion is started: 
       000 No start (this value should be used when clearing PDN to 0). 
       001 Start conversion now. 
       010 Start conversion when the edge selected by bit 27 occurs on P0.16/EINT0/MAT0.2/CAP0.2 pin. 
       011 Start conversion when the edge selected by bit 27 occurs on P0.22/TD3/CAP0.0/MAT0.0 pin. 
       100 Start conversion when the edge selected by bit 27 occurs on MAT0.1. 
       101 Start conversion when the edge selected by bit 27 occurs on MAT0.3. 
       110 Start conversion when the edge selected by bit 27 occurs on MAT1.0. 
       111 Start conversion when the edge selected by bit 27 occurs on MAT1.1.
       */
      unsigned start	: 3;
      /*
       This bit is significant only when the START field contains 010-111. In these cases: 
       1 Start conversion on a falling edge on the selected CAP/MAT signal.
       0 Start conversion on a rising edge on the selected CAP/MAT signal.
       */     
      unsigned edge		: 1;
      unsigned reserved2	: 4;
      
    };
    ControlRegBits() : value(0) {
    }
  };
  
  union GlobalStartRegBits {
    uint32 value;
    struct {
      unsigned reserved0 : 16;
      unsigned burst	: 1;
      unsigned reserved1: 7;
      unsigned start	: 3;
      unsigned edge		: 1;
      unsigned reserved2 : 4;      
    };
    GlobalStartRegBits() : value(0) {
    }
  };
  
  union GlobalDataRegBits {
    uint32 value;
    struct {
      unsigned reserved0: 6;
      unsigned result: 10;
      unsigned reserved1: 8;
      unsigned channel: 3;
      unsigned reserved2: 3;
      unsigned overrun: 1;
      unsigned done: 1;
    };
    GlobalDataRegBits() : value(0) {
    }
  };
  
  union InterruptEnableRegBits {
    uint32 value;
    struct {
      unsigned enable0: 1;
      unsigned enable1: 1;
      unsigned enable2: 1;
      unsigned enable3: 1;
      unsigned enable4: 1;
      unsigned enable5: 1;
      unsigned enable6: 1;
      unsigned enable7: 1;
      unsigned global_gen: 1;
    };
    InterruptEnableRegBits(): value(0) {
    }
  };
  
  union DataRegBit {
    uint32 value;
    struct {
      unsigned reserved0: 6;
      unsigned result: 10;
      unsigned reserved1: 14;
      unsigned overrun: 1;
      unsigned done: 1;
    };
  };
  
  union StatusRegBits {
    uint32 value;
    struct {
      unsigned done0: 1;
      unsigned done1: 1;
      unsigned done2: 1;
      unsigned done3: 1;
      unsigned done4: 1;
      unsigned done5: 1;
      unsigned done6: 1;
      unsigned done7: 1;
      unsigned overrun0: 1;
      unsigned overrun1: 1;
      unsigned overrun2: 1;
      unsigned overrun3: 1;
      unsigned overrun4: 1;
      unsigned overrun5: 1;
      unsigned overrun6: 1;
      unsigned overrun7: 1;
      unsigned adint: 1;
    };
  };
  
  /* --------------------------------------------------------------------------- */
  /* Globals section                                                             */
  /* --------------------------------------------------------------------------- */
  
  /* --------------------------------------------------------------------------- */
  /* Functions section                                                          */
  /* --------------------------------------------------------------------------- */
  bool ADC::configure(uint8 channel_mask, Mode mode, uint32 samples_per_seconds, InterruptMode interrupt_mode) {
    
    if (samples_per_seconds > MAX_SAMPLES_PER_SECOND)
      samples_per_seconds = MAX_SAMPLES_PER_SECOND;
    const uint32 freq = samples_per_seconds * 11;
    
    ControlRegBits crbits;  // Initialized to 0
    crbits.sel = channel_mask;
    crbits.clkdiv = (PCLK / freq) - 1;  // prescale the adc clock to 4.5 MHz at most
    if (mode == BURST)
      crbits.burst = 1;  // setup for burst
    crbits.clks = 0;  // Best accuracy
    if (mode != NONE)
      crbits.pdn = 1;   // Activate the device
    if (mode == ONE_SHOT)
      crbits.start = 1;
    
    // Enable the ADC function on the requested device and channels
    for (uint8 bit = 0; bit < 8; ++ bit) {
      // Enable only if enabled in the mask
      if (channel_mask & (1 << bit))
        PinSelect::enableADC(m_device, bit);
    }
    
    if (mode != NONE && interrupt_mode == INTERRUPT) {
      // Enable interrupts for the device
      InterruptEnableRegBits intbits;
#if ADC_SINGLE_INTERRUPT
      // Only the highest channel contributes to the interrupt
      if (channel_mask & BIT_CHAN_7)
        intbits.enable7 = 1;
      else if (channel_mask & BIT_CHAN_6)
        intbits.enable6 = 1;
      else if (channel_mask & BIT_CHAN_5)
        intbits.enable5 = 1;
      else if (channel_mask & BIT_CHAN_4)
        intbits.enable4 = 1;
      else if (channel_mask & BIT_CHAN_3)
        intbits.enable3 = 1;
      else if (channel_mask & BIT_CHAN_2)
        intbits.enable2 = 1;
      else if (channel_mask & BIT_CHAN_1)
        intbits.enable1 = 1;
      else if (channel_mask & BIT_CHAN_0)
        intbits.enable0 = 1;
#else
      // All enabled channels generate an interrupt
      intbits.global_gen = 1;
#endif
      switch (m_device) {
        case ADC_0:
          AD0INTEN = intbits.value;
          // Enable interrupts
          VIC::enable(VIC::AD0, VEC_AD0, *this);          
          break;
          
        case ADC_1:
          AD1INTEN = intbits.value;
          // Enable interrupts
          VIC::enable(VIC::AD1, VEC_AD1, *this);          
          break;
      }
    }
    
    // Now apply the new mode on the device
    switch (m_device) {
      case ADC_0:
        AD0CR = crbits.value;
        break;
      case ADC_1:
        AD1CR = crbits.value;
        break;
    }
    
    return true;
  }
  
  void ADC::stop() {
    switch (m_device) {
      case ADC_0:
        AD0CR = 1;  // Reset value. Unit is disabled        
        break;
        
      case ADC_1:
        AD1CR = 1;  // Reset value. Unit is disabled        
        break;
    }    
  }
  

 void ADC::enable(Channel channel, bool active) {
    ControlRegBits crbits;
    
    // Get current device setting
    switch (m_device) {
      case ADC_0:
        crbits.value = AD0CR;
        break;
      case ADC_1:
        crbits.value = AD1CR;
        break;
    }
    
    if (active)
      PinSelect::enableADC(m_device, channel);
    
    // Turn channel bits
    if (active)
      crbits.sel |= 1 << channel;
    else
      crbits.sel &= ~(1<<channel);
    
    // Write back to the device
    switch (m_device) {
      case ADC_0:
        AD0CR = crbits.value;
        break;
      case ADC_1:
        AD1CR = crbits.value;
        break;
    }
    
#if ADC_SINGLE_INTERRUPT
    //
    // Check if we are in interrupt mode
    //
    InterruptEnableRegBits intbits;
    switch (m_device) {
      case ADC_0:
        intbits.value = AD0INTEN;
        break;
      case ADC_1:
        intbits.value = AD1INTEN;
        break;
    }
    
    if (intbits.value != 0) {
      // Yes, the device is in interrupt mode. We have to recalculate the interrupt mask,
      // with the bit of the highest channel set.
      const uint8 channel_mask = crbits.sel;	// This mask is the updated mask
      
      if (channel_mask & BIT_CHAN_7)
        intbits.enable7 = 1;
      else if (channel_mask & BIT_CHAN_6)
        intbits.enable6 = 1;
      else if (channel_mask & BIT_CHAN_5)
        intbits.enable5 = 1;
      else if (channel_mask & BIT_CHAN_4)
        intbits.enable4 = 1;
      else if (channel_mask & BIT_CHAN_3)
        intbits.enable3 = 1;
      else if (channel_mask & BIT_CHAN_2)
        intbits.enable2 = 1;
      else if (channel_mask & BIT_CHAN_1)
        intbits.enable1 = 1;
      else if (channel_mask & BIT_CHAN_0)
        intbits.enable0 = 1;
    }
    //
    // Perform the change
    //
    switch (m_device) {
      case ADC_0:
        AD0INTEN = intbits.value;
        break;          
      case ADC_1:
        AD1INTEN = intbits.value;
        break;
    }
#else
    //
    // Nothing to do, since we are using global_gen in configure() and
    // all enabled channels generate an interrupt
    //
#endif
  }
  
  bool ADC::get_instant_conversion_result(Channel channel, uint32 &result) const {
    uint32 raw_value;
    switch (m_device) {
      case ADC_0:
        raw_value = AD0DR0 + channel;
        break;
      case ADC_1:
        raw_value = AD0DR1 + channel;
        break;
      default:
        return false;
    }
    
    // Extract the result
    const uint32 DONE_MASK = 1 << 31;
    const uint32 MASK_10_BITS = 0x03ff;
    
    if (raw_value & DONE_MASK) {
      result = (raw_value >> 6) & MASK_10_BITS;
      return true;
    }
    return false;
  }
  
  bool ADC::startNowAndGetResult(Channel channel, uint32& result) {
    // Initiate the conversion
    configure(1 << channel, ONE_SHOT);
    
    // And wait for the result
    const uint32 DONE_MASK = 1 << 31;
    const uint32 MASK_10_BITS = 0x03ff;
    
    volatile uint32 *adc_addr = (m_device == ADC_0) ? 
#if 1
    &AD0DR0 + channel : &AD1DR0 + channel
#else
    &AD0GDR : &AD1GDR
#endif
    ;
    
    // Loop until conversion done
    uint32 value;
    do  {
      value = *adc_addr;
    } while ((value & DONE_MASK) == 0);
    
    result = (value >> 6) & MASK_10_BITS;
    
    return true;
  }
  
  /* --------------------------------------------------------------------------- */
  /* Interrupt section                                                           */
  /* --------------------------------------------------------------------------- */
  void ADC::handle_irq() {
    const Device device = m_device;
#if DEBUG
    ++m_interrupt_count;
#endif
#if ADC_SINGLE_INTERRUPT
    DataRegBit data_bits;
    StatusRegBits st;
    switch (device) {
      case ADC_0:
        st.value = AD0STAT;
        if (st.done0) {
          data_bits.value = AD0DR0;
          m_result[0].add_sample(data_bits.result);
        }
        if (st.done1) {
          data_bits.value = AD0DR1;
          m_result[1].add_sample(data_bits.result);
        }
        if (st.done2) {
          data_bits.value = AD0DR2;
          m_result[2].add_sample(data_bits.result);
        }
        if (st.done3) {
          data_bits.value = AD0DR3;
          m_result[3].add_sample(data_bits.result);
        }
        if (st.done4) {
          data_bits.value = AD0DR4;
          m_result[4].add_sample(data_bits.result);
        }
        if (st.done5) {
          data_bits.value = AD0DR5;
          m_result[5].add_sample(data_bits.result);
        }
        if (st.done6) {
          data_bits.value = AD0DR6;
          m_result[6].add_sample(data_bits.result);
        }
        if (st.done7) {
          data_bits.value = AD0DR7;
          m_result[7].add_sample(data_bits.result);
        }
        break;

      case ADC_1:
        st.value = AD1STAT;
        if (st.done0) {
          data_bits.value = AD1DR0;
          m_result[0].add_sample(data_bits.result);
        }
        if (st.done1) {
          data_bits.value = AD1DR1;
          m_result[1].add_sample(data_bits.result);
        }
        if (st.done2) {
          data_bits.value = AD1DR2;
          m_result[2].add_sample(data_bits.result);
        }
        if (st.done3) {
          data_bits.value = AD1DR3;
          m_result[3].add_sample(data_bits.result);
        }
        if (st.done4) {
          data_bits.value = AD1DR4;
          m_result[4].add_sample(data_bits.result);
        }
        if (st.done5) {
          data_bits.value = AD1DR5;
          m_result[5].add_sample(data_bits.result);
        }
        if (st.done6) {
          data_bits.value = AD1DR6;
          m_result[6].add_sample(data_bits.result);
        }
        if (st.done7) {
          data_bits.value = AD1DR7;
          m_result[7].add_sample(data_bits.result);
        }
        break;
    }
#else
    // Check source channel
    GlobalDataRegBits glob_data_bits;
    glob_data_bits.value = (device == ADC_0) ? AD0GDR : AD1GDR;
    
    const uint32 channel = glob_data_bits.channel;
    const uint32 result = glob_data_bits.result;
    
    // Add results
    m_result[channel].add_sample(result);
#endif
  }
}

