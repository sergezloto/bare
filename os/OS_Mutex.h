/*
 *  OS_Mutex.h
 *  Embedded
 *
 *  Created by Serge on 16/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "OS.h"

namespace os {
  /**
   * Mutual exclusion object. Has two states, free or acquired.
   * Can be acquired once at a time only.
   * The owner task can recursively acquire it. Only the last release() will free the mutex
   */
  class Mutex: NoCopy {
  public:
    /**
     * Creates a free mutex
     */
    Mutex();
    
    /**
     * Destroys the mutex
     */
    ~Mutex();
    
    /**
     * Attempts to acquire the mutex without blocking
     * @return true if the mutex was acquired
     */
    bool try_acquire();
    
    /**
     * Acquires the mutex in a blocking fashion
     */
    void acquire();
    
    /**
     * Release the mutex. Calling release() on a free mutex has no effect.
     * @return true if the mutex was in the acquired state
     */
    bool release();
    
  private:
    TaskBase *m_owner;
    uint32 m_nesting;
    
    typedef util::List<TaskBase> task_queue_type;
    static task_queue_type m_waiters;
  };
  
  inline
  Mutex::Mutex() : m_owner(0), m_nesting(0) {
  }
  
  inline
  Mutex::~Mutex() {
  }
  
  inline
  bool Mutex::try_acquire() {
    if (m_owner != 0 && m_owner != OS::get_current()) {
      // Not available
      return false;
    }    
    m_owner = OS::get_current();
    ++m_nesting;
  }
  
  inline
  void Mutex::acquire() {
    if (m_owner != 0 && m_owner != OS::get_current()) {
      // Add the task to the list of waiters for this mutex and suspend
      m_waiters.add_tail(OS::get_current());
      OS::suspend();
    }
    m_owner = OS::get_current();
    ++m_nesting;
  }
  
  inline
  bool Mutex::release() {
    if (m_owner == 0) {
      // Nothing to do
      return false;
    }
    if (--m_nesting > 0) {
      // Was recursively acquired
      return false;
    }
    
    // Wakeup first waiter
    m_owner = m_waiters.remove_head();
    
    if (m_owner != 0) {
      // Wake him up!
      OS::wakeup(m_owner);
    }
    
    return true;
  }
}
