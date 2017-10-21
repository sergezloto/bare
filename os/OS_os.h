/*
 *  os.h
 *  Embedded
 *
 *  Created by Serge on 06/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "util.h"
#include "OS_Task.h"

namespace  os {
  
  /**
   * A non-preemptive, cooperative multitasking OS.
   * The code starting the OS is deemed task #0.
   * 
   */
  class OS: NoInstance {
  public:
    /**
     * Adds a task to the OS
     */
    static void add(TaskBase& task);
    
    /**
     * Starts the scheduler, that is, the OS itself
     */
    static void start();
    
    /**
     * @return the current task
     */
    static TaskBase* get_current();
    
    /**
     * Tries to give control to another task.
     */
    static void yield();
    
    /**
     * Puts the current task into suspended mode
     */
    static void suspend();
    
    /**
     * Wakes up a sleeping task
     */
    static void wakeup(TaskBase* task);
    
    /**
     * Critical section entry.
     * Entries can be nested
     */
    static void enter_critical_section() {
      ++m_critical;
    }
    
    /**
     * Critical section exit.
     * Entries/leaves *must* be paired!
     */
    static void leave_critical_section() {
      --m_critical;
    }
    
    /**
     * After this the task cannot be preempted but interrupts
     * are still enabled
     */
    //static void enterCriticalSection();
    
    /**
     * Enables preemption again
     */
    //static void leaveCriticalSection();
    
    /**
     * @return the number of milliseconds since the scheduler started
     */
    //static uint32 getElapsedTimeMs();
    
    /**
     * Sleeps the calling task for N milliseconds.
     * Note that the delay can be at most 2^32 - 1 milliseconds, that is,
     * a bit more than 49 days.
     */
    //static void delayMs(uint32 delay);
    
    /**
     * Sleeps the calling task until a fixed date
     * @param ref the time at which the duration starts
     * @param delayMs the delay, starting at delayRef
     */
    //static void delayUntilMs(uint32 &ref, uint32 delay);
    
    /**
     * @return the number of tasks in the OS
     */
    static uint32 get_task_number();
    
    template <class WRITER>
    static WRITER& dump(WRITER& os) {
      os << "TASKS: " << m_task_num << "\n";
      for (task_queue_type::iterator t = m_run_queue.begin(); t != m_run_queue.end(); ++t) {
        os << "R-" << (*t).get_name() <<  ": "
        << "stack(" << (*t).get_stack_usage() 
        << "/" << (*t).get_stack_size() << ")" << "\n";
      }
      for (task_queue_type::iterator t = m_sleep_queue.begin(); t != m_sleep_queue.end(); ++t) {
        os << "S-" << (*t).get_name() <<  ": "
        << "stack(" << (*t).get_stack_usage() 
        << "/" << (*t).get_stack_size() << ")" << "\n";
      }
      return os;
    }
    
  private:
    /**
     * Sets up all registered tasks
     */
    static void init_tasks();
    
    /**
     * Sets up a single task
     */

    static void init_task(TaskBase* task);

    /**
     * All tasks are here
     */
    static uint32 m_task_num;    
    
    typedef util::List<TaskBase> task_queue_type;
    static task_queue_type m_run_queue;
    static task_queue_type m_sleep_queue;
    static task_queue_type::iterator m_current_task;
    
    static TaskBase m_task_main;
    static bool m_started;
    static uint32 m_critical;
  };
  
  inline 
  void OS::add(TaskBase& task) {

    if (m_started) {
      //
      // When the OS is first started, all pre-existing tasks are initialized 
      //
      init_task(&task);      
    }
    m_run_queue.add_tail(&task);
    ++m_task_num;
  }

  
  inline
  void OS::start() {
    // Initialize all tasks
    init_tasks();
    // We are task 0. Carry on!
    m_started = true;
  }
  
  inline
  TaskBase* OS::get_current() {
    return &*m_current_task;
  }
  
  inline
  uint32 OS::get_task_number() {
    return m_task_num;
  }
}
