/*
 *  AudioPlayer.h
 *  Embedded
 *
 *  Created by Serge on 6/18/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "HAL_Timer.h"
#include "HAL_DAC.h"
#include "HAL_Processor.h"
#include "util.h"

namespace hal {
  
  /**
   * A class that reads an audio source, and output audio samples to the DAC.
   * It does not free the audio source, as it assumes the audio source is not dynamically
   * allocated.
   * A timer is used for scheduling samples at 8000 Hz.
   */
  template <class AudioSource_T>
  class AudioPlayer: NoCopy {
  public:
    
    /**
     * Fixed sample rate
     */    
    static const uint32 SAMPLE_RATE = AudioSource_T::SAMPLE_RATE;
    
    /**
     * Samples size and signedness.
     */
    typedef typename AudioSource_T::sample_type sample_type;
    static const uint32 SAMPLE_SIZE = sizeof(sample_type);

    /**
     * We use this type of interpolator
     */
    typedef typename util::LinearInterpolator<sample_type> interpolator_type;
    
    /**
     * Which oversamples by this much
     */
    static const uint32 OVERSAMPLING_FACTOR = interpolator_type::OVERSAMPLING_FACTOR;
    
    /**
     * A frame is 20ms worth of audio, that is, 50 frames per second
     */
    static const uint32 FRAME_TIME_MS = AudioSource_T::FRAME_TIME_MS;

    /**
     * the size of a frame *in samples*
     */
    static const uint32 FRAME_SIZE = AudioSource_T::FRAME_SIZE;
    
    /**
     * the player maintains this many audio buffers
     */
    static const uint32 AUDIO_PLAYER_BUFFER_SIZE = AudioSource_T::AUDIO_PLAYER_BUFFER_SIZE;
    
    /**
     * The player states
     */
    enum State {
      STOPPED,
      STOPPING,
      BUFFERING,
      PLAYING
    };
    
    /**
     * The player starts off not playing at all
     * @param timer a configured timer, with a set rate (so we can calculate the sample time in ticks)
     */
    AudioPlayer(Timer& timer, Timer::MatchChannel channel) 
    : m_state(STOPPED), m_timer(timer), m_match_channel(channel), m_audio_source(0),
    m_loud(true) {
#if DEBUG
      d_interrupts = d_underrun = d_int_while_stopped = d_int_while_stopping = d_int_while_buffering = 0;
#endif
    }
    
    /**
     * cleanup
     */
    ~AudioPlayer() {
    }
    
    /**
     * Audio source setup
     */
    void set_audio_source(AudioSource_T& audio_source) {      
      if (m_state != STOPPED) {
        // Any activity must cease
        m_state = STOPPING;
      }
      m_audio_source = &audio_source;
    }
    
    /**
     * Volume setup
     */
    void set_loud(bool loud) {
      m_loud = loud;
    }
    
    void start() {
      {
        static bool dac_initialised = false;
        if (!dac_initialised) { 
          dac_initialised = true;
          using lpc214x::DAC;
          DAC::configure();
        }
      }
      m_buffer.reset();
      m_interpolator.reset();
      m_sample_ticks = m_timer.get_rate() / (SAMPLE_RATE * OVERSAMPLING_FACTOR);
      m_state = BUFFERING;
    }
    
    void stop() {
      if (m_state != STOPPED)
        m_state = STOPPING;
    }
    
    /**
     * This player must be given the cpu
     * so it can fetch from the audio source if needed.
     */
    void update() {
      if (m_audio_source == 0)
        return;
      
      switch (m_state) {
        case STOPPED:
          // Do nothing
          break;
        case STOPPING:
          m_state = STOPPED;
          break;
        case BUFFERING:
        {
          if (!m_audio_source->append_samples(m_buffer)) {
            // No data while buffering. Bail out
            m_state = STOPPING;
            break;
          }
          
          // Check whether enough has been buffered. If so, go to PLAYING state
          uint32 buffer_size;
          {
            // Protect against race condition with interrupt callback
            hal::NoInterrupt ni;
            buffer_size = m_buffer.size();
          }
          if (buffer_size >= FRAME_SIZE) {
            // We have enough to get started. Start playback in interrupt function
            m_state = PLAYING;
            m_timer.match(m_match_channel, 
                          m_timer.getTimerValue() + m_sample_ticks,
                          audio_callback, 
                          this);
          }
        }
          break;
        case PLAYING:
        {
          uint32 available;
          {
            // Protect against race condition with interrupt callback
            hal::NoInterrupt ni;
            available = m_buffer.available();
          }
          if (available >= FRAME_SIZE) {
            // There is room for one more frame
            if (!m_audio_source->append_samples(m_buffer)) {
              // If append_samples failed, then no more to play
              m_timer.stop_match(m_match_channel);
              m_state = STOPPING;
            }
          }
          break;
        }
      }
    }
    
    /**
     * Info methods
     */
    State get_state() const {
      return m_state;
    }
    
  private:
    /**
     * Timer match callback.
     * This function is called in an interrupt context, so it
     * runs asynchronously
     */
    static uint32 audio_callback(Timer& timer, 
                                 Timer::MatchChannel channel, 
                                 const Timer::MatchInfo& info) {
      // The instance was passed as callback data
      AudioPlayer* audio_player = static_cast<AudioPlayer*>(info.callbackData);
      audio_player->audio_callback();
      
      // Next interrupt is at next sample time, expressed in timer ticks
      return info.value + audio_player->m_sample_ticks;
   }
     
    /**
     * This method is called in an interrupt context, and
     * runs asynchronously
     */
    void audio_callback() {
      using lpc214x::DAC;
      
#if DEBUG
      ++d_interrupts;
#endif
      switch (m_state) {
        case STOPPED:    // Look out for funny interrupts
#if DEBUG
          ++d_int_while_stopped;
#endif
          break;
        case STOPPING:   // That can happen if stop() is called while playing
          // Cancel the timer if on
          m_timer.stop_match(m_match_channel);          
          m_state = STOPPED;
#if DEBUG
          ++d_int_while_stopping;
#endif
          break;
        case BUFFERING:  // We shouldn't get an interrupt when buffering!
#if DEBUG
          ++d_int_while_buffering;
#endif
          break;
        case PLAYING:
          if (m_buffer.is_empty()) {
            // Oops, we have an underrun. Go to buffering mode
            // The timer should no longer run. Cancel it!
            m_timer.stop_match(m_match_channel);
            m_state = BUFFERING;
#if DEBUG
            ++d_underrun;
#endif
            // Return, we have a new sample to play
            break;
          }
          // All is well, we can play a sample!

          // Peek a sample to feed the interpolator
          const sample_type audio_sample = m_buffer.peek();
          sample_type output_value;
          if (m_interpolator.interpolate(audio_sample, output_value)) {
            // A sample was consumed. Let the buffer know about it.
            m_buffer.get();
          }
          
          // Volume down if needed
          if (!m_loud) {
            output_value >>= 1;
          }
          
          DAC::set_value(output_value);
          m_state = PLAYING;
          break;
      }
    }
    
    volatile State m_state;
    
    /**
     * Timer used to schedule samples out
     */
    Timer& m_timer;
    /**
     * The timer match channel we use
     */
    const Timer::MatchChannel m_match_channel;
    
    AudioSource_T* m_audio_source;
    uint32 m_sample_ticks;
    
    /**
     * Volume setting
     */
    bool m_loud;
    
    /**
     * The samples to be played are stored here
     */
    typedef util::Buffer<sample_type, AUDIO_PLAYER_BUFFER_SIZE> buffer_type;
    buffer_type m_buffer;
    
    interpolator_type m_interpolator;
    
#if DEBUG
  public:
    volatile uint32 d_interrupts, d_underrun, d_int_while_stopped, d_int_while_stopping, d_int_while_buffering;
#endif
  };
}
