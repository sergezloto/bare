/*
 *  HAL_Timer.h
 *  Embedded
 *
 *  Created by Serge on 24/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "HAL_Driver.h"
#include "HAL_InterruptHandler.h"

namespace hal {
  /*
   * Driver for the on-chip timers.
   * It includes a PWM measurement facility.
   */
  class Timer: public Driver, InterruptHandler, NoCopy {
public:
    /*
     * This hardware has 2 timers
     */
    static const uint32 NB_TIMERS = 2;
    
    enum TimerId {
      TIMER0 = 0,
      TIMER1 = 1
    };
    
    /*
     * There are 4 Match registers per channel
     */
    static const uint32 NB_MATCH_CHANNELS = 4;
    
    enum MatchChannel {
      MATCH0 = 0,
      MATCH1 = 1,
      MATCH2 = 2,
      MATCH3 = 3
    };
            
    struct MatchInfo;
    /*
     * A callback run upon match. Its return value is the new match value
     */
    typedef uint32 (*MatchCallback)(Timer& timer, MatchChannel channel, 
                                  const MatchInfo& info);

    struct MatchInfo {
      MatchInfo() : value(0), callback(0), callbackData(0) {
      }
      uint32 value;
      MatchCallback callback;
      void *callbackData;
    };
    
    /*
     * There are 4 capture channels per timer
     */
    static const uint32 NB_CAPTURE_CHANNELS = 4;
    
    enum CaptureChannel {
      CAP0 = 0,
      CAP1 = 1,
      CAP2 = 2,
      CAP3 = 3
    };
        
    /*
     * A capture channel can be set up to capture in the following modes
     */
    enum CaptureMode {
      NONE,
      RISING, 
      FALLING,
      RISING_FALLING
    };
    
    enum CaptureAction {
      NO_CHANGE,
      STOP_CAPTURE,
      SET_RISING_EDGE,
      SET_FALLING_EDGE,
      SET_BOTH_EDGES
    };
    
    struct CaptureInfo;
    typedef CaptureAction (*CaptureCallback)(Timer& timer, 
                                           CaptureChannel channel,
                                           const CaptureInfo& info);
    /**
     * Timer capture information, passed to callbacks
     */
    struct CaptureInfo {
      CaptureInfo() 
      : mode(NONE), value(0), callback(0), callbackData(0) {
      }
      
      /**
       * Mode under which the measurement was done
       */
      CaptureMode mode;
      
      /**
       * Last timer value for capture event.
       * The value unit is as per the timer rate.
       */
      uint32 value;
      
      CaptureCallback callback;
      void *callbackData;
    };
    
    /**
     * Gets an instance
     */
    Timer(TimerId timerId);
    
    /**
     * Sets up the timer for counting at the given rate.
     * Note: Rate must not be higher than PCLK!
     * @return true if success
     */
    bool configure(uint32 rate);
    
    /**
     * Runs the timer
     */
    void start();
    
    /**
     * Halts the timer
     */
    void stop();
    
    /**
     * Sets up match for a given channel.
     * Action on match pins are not handled yet, hence a callback must be specified
     * to perform the job of toggling pins if this function is desired.
     * A future version of this software will handle action on match pins in hardware
     */
    void match(MatchChannel channel,
               uint32 value,
               MatchCallback cb, void *cbData = 0);
        
    /**
     * Cancels a timer match on a given channel. The timer running state is unchanged
     */
    void stop_match(MatchChannel channel);
    
    /**
     * Sets up capture on one of 4 channels
     * @param channel the capture channel
     * @param The capture mode, that is, what event to capture
     * @param cb a function called upon capture. NOTE: This function is
     *           run within an interrupt service routine
     * @param cbData an auxilliary value to be passed to the callback.
     */
    void capture(CaptureChannel channel, CaptureMode mode,
                 CaptureCallback cb = 0, void *cbData = 0);
    
    /**
     * Cancels timer capture on the given channel. The timer running state is unchanged.
     * The capture pin stays in capture mode. The application has to change its function if
     * desired.
     */
    void stop_capture(CaptureChannel channel);
    
    /**
     * @return the current timer value
     */
    uint32 getTimerValue() const;
    
    /**
     * @return the time value for the last capture event
     *         for the given channel
     */
    uint32 getCaptureValue(CaptureChannel channel) const;
    
    /**
     * @return the time interval value for the last capture event
     *         since the previous capture
     */
    uint32 getCapturePWM(CaptureChannel channel) const;
    
    /**
     * @return the rate the time runs at in Hz
     */
    uint32 get_rate() const {
      return m_rate;
    }
    
private:    
    const TimerId m_timerId;
    
    static CaptureInfo captureInfo[NB_TIMERS][NB_CAPTURE_CHANNELS];
    
    static MatchInfo matchInfo[NB_TIMERS][NB_MATCH_CHANNELS];
    
    void handle_irq();
    
    uint32 m_rate;
  };
  
  inline Timer::Timer(TimerId timerId)
    : m_timerId(timerId), m_rate(0) {
    }
}
