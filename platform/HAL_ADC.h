/*
 *  HAL_ADC.h
 *  Embedded
 *
 *  Created by Serge on 10/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"
#include "HAL_Driver.h"
#include "HAL_InterruptHandler.h"
#include "util.h"

namespace hal {
  
  /*
   * Driver for an ADC device, both in polling and burst mode, with
   * a conversion result accumulator so as to take mean values over a period.
   * The ADC frequency should not exceed 4.5 MHz and a conversion takes 
   * 11 ADC cycles with a precision of 10 bits.
   * In burst mode, conversions run at about 400 ksps (kilo samples per second).
   * With a 10 bit results, the accumulator potentially overflows after 
   * 4.10^6 conversions, that is, after a little over 10 seconds.
   * Thus in burst mode, the accumulator has to be read at least every 10 seconds.
   */
  class ADC: public Driver, public InterruptHandler, NoCopy {
  public:
    /*
     * The LPC2148 has two separate ADC devices
     */
    enum Device {
      ADC_0 = 0,
      ADC_1 = 1
    };
    
    /**
     * The ADC can operate at 4.5 MHz at most.
     */
    static const uint32 MAX_FREQ = 4500000;
    static const uint32 MIN_FREQ = PCLK / 256;
    static const uint32 MAX_SAMPLES_PER_SECOND = MAX_FREQ / 11;
    static const uint32 MIN_SAMPLES_PER_SECOND = MIN_FREQ / 11;  // About  freq = 234175, 21306 samp/s
    
    /*
     * On lpc214x, ADC1 device has eight channels, while ADC0 has 6. Channels 0 and 5 are not implemented
     */
    enum Channel {
      CHAN_0 = 0,
      CHAN_1 = 1,
      CHAN_2 = 2,
      CHAN_3 = 3,
      CHAN_4 = 4,
      CHAN_5 = 5,
      CHAN_6 = 6,
      CHAN_7 = 7
    };
    
    enum ChannelBits {
      BIT_CHAN_0 = 1 << CHAN_0,
      BIT_CHAN_1 = 1 << CHAN_1,
      BIT_CHAN_2 = 1 << CHAN_2,
      BIT_CHAN_3 = 1 << CHAN_3,
      BIT_CHAN_4 = 1 << CHAN_4,
      BIT_CHAN_5 = 1 << CHAN_5,
      BIT_CHAN_6 = 1 << CHAN_6,
      BIT_CHAN_7 = 1 << CHAN_7
    };
    
    static const uint32 NB_CHANNELS = 8;
    
    /**
     * The ADC can be set up to inactive (NONE), run for one shot (ONE_SHOT) or
     * convert all selected channels repeatedly (BURST)
     */
    enum Mode {
      NONE,   // No conversion active
      ONE_SHOT,
      BURST
    };
    
    /**
     * Instructs whether to generate interrupts after conversions
     */
    enum InterruptMode {
      NO_INTERRUPT,
      INTERRUPT
    };
    
    /**
     * The result from accumulated conversions during burst
     */
    struct ConversionResult {
      uint32 sum;
      uint32 samples;
      uint32 max;
      uint32 min;
      
      void reset() {
        sum = samples = 0;
        max = 0;
        min = 1024;
      }
      
      void add_sample(uint32 sample) {
        if (sample > max)
          max = sample;
        if (sample < min)
          min = sample;

        sum += sample;
        ++samples;
      }
      
      uint32 get_samples() const {
        return samples;
      }
      
      uint32 get_sum() const {
        return sum;
      }
      
      template <class T>
      void get_mean_value(T& v) {
        v = sum;
        v = v / samples;
      }
      
      template <class T>
      T get_mean_value() const {
        return (T)sum / samples;
      }
      
      uint32 get_min() const {
        return min;
      }

      uint32 get_max() const {
        return max;
      }
    };
    
    /**
     * Constructs an ADC for the given device
     */
    ADC(Device device);
    
    /**
     * Sets up the device with the given channel mask and mode.
     * BURST and ONE_SHOT sets the conversion in asynchronous mode, that is
     * the function returns immediately while the conversion, if any, is
     * being performed.
     * The result can be obtained with \f get_conversion_result()
     * @param channels a mask produced by OR-ing the desired channels, that is, adc pins.
     *                 CHAN_0 and CHAN_5 are not valid for device 0 on lpc214x
     * @param mode  one of NONE, ONE_SHOT or BURST
     * @param samples_per_second the speed the ADC is to be configured with.
     *                           It must not exceed MAX_SAMPLES_PER_SECOND
     * @param interrupts generate interrupts when a conversion is done. This enables to accumulate results and 
     *                   calculate a mean value
     * @return false if any error occurred
     */
    bool configure(uint8 channel_mask, 
                   Mode mode, 
                   uint32 samples_per_second = MIN_SAMPLES_PER_SECOND, 
                   InterruptMode interrupt_mode = INTERRUPT);
    
    /**
     * Stops the unit (all channels)
     */
    void stop();
    
    /**
     * Enables/disable an ADC channel
     */
    void enable(Channel channel, bool active = true);
        
    /**
     * @return the current number of samples gathered for the given channel
     */
    uint32 get_sample_count(Channel channel) const;
    
    /**
     * Obtains a burst conversion result and reset the counter and value.
     * This function does not change the mode of the ADC.
     * @param channel the requested channel
     * @pram result the burst conversion result
     * @param reset when true, the counter for the channel is reset to zero
     */
    void get_conversion_result(Channel channel, 
                               ConversionResult& result,
                               bool reset = true);
    
    /**
     * Obtains a burst conversion result and reset the counter and value.
     * This function does not change the mode of the ADC.
     * @param channel the requested channel
     * @param reset when true, the counter for the channel is reset to zero
     * @return result the burst conversion result
     */
    ConversionResult get_conversion_result(Channel channel,
                                           bool reset = true);

    
    /**
     * Obtains the last conversion result from the given channel, if available. If interrupts are active for
     * the channel, the function will always return false since the ISR will have consumed the conversion.
     * @param channel the adc channel
     * @param result overwritten with the conversion result if available
     * @return true if a conversion is available, that is, if the DONE bit is set
     */
    bool get_instant_conversion_result(Channel channel, uint32& result) const;
    
    /**
     * Resets the given conversion result for the given channel.
     * This does not affect the mode of the ADC
     */
    void reset_conversion_result(Channel channel);
    
    /*
     * Sets up a single channel, starts a conversion, waits for the result (busy loop)
     * and returns the result.
     * This function disrupts any undergoing conversion and setup, including burst
     * conversion.
     */
    bool startNowAndGetResult(Channel Channel, uint32& result);
    
#if DEBUG
    uint32 get_interrupt_count() const {
      return m_interrupt_count;
    }
#endif
    
  private:
    const Device m_device;

    ConversionResult m_result[NB_CHANNELS];
    
#if DEBUG
    volatile uint32 m_interrupt_count;
#endif
    
    void handle_irq();
  };
  
  inline 
  ADC::ADC(Device device)
  : m_device(device) {
    for (uint32 i = 0; i < NB_CHANNELS; ++i) {
      m_result[i].reset();
    }
#if DEBUG
    m_interrupt_count = 0;
#endif
  }
  
  inline
  uint32 ADC::get_sample_count(Channel channel) const {
    return m_result[channel].get_samples();
  }
  
  inline
  void ADC::reset_conversion_result(Channel channel) {
    m_result[channel].reset();    
  }
  
  inline
  void ADC::get_conversion_result(Channel channel, ConversionResult& result, bool reset) {
    result = m_result[channel];
    if (reset) {
      m_result[channel].reset();
    }
  }
  
  inline
  ADC::ConversionResult ADC::get_conversion_result(Channel channel, bool reset) {
    const ConversionResult result = m_result[channel];
    if (reset) {
      m_result[channel].reset();
    }
    return result;
  }  
}
