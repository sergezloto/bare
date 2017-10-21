/*
 *  OS_Task.h
 *  Embedded
 *
 *  Created by Serge on 17/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "util.h"

namespace os {
  
  /**
   * Any task will have to inherit this class, and supply a run() member function.
   */
  class TaskBase: NoCopy {
    typedef TaskBase this_type;
    
  public:
    /**
     * Priority 0 is the lowest priority, same as the idle task. Highest is 5
     */
    typedef uint8 Priority;
    static const Priority PRIORITY_IDLE = 0;
    static const Priority PRIORITY_DEFAULT = 2;
    static const Priority PRIORITY_HIGHEST = 5;
    
    static const uint32 STACKSIZE_DEFAULT = 256;
    
    /**
     * The possible states for a task
     */
    enum State {
      RUN = 'R',
      SUSPENDED = 'S'
    };
    
    /**
     * Two tasks are equal only if they are the same one
     */
    bool operator ==(const TaskBase& task) const {
      return this == &task;
    }
    
    bool operator !=(const TaskBase& task) const {
      return this != &task;
    }

  protected:
    /**
     * Constructor. Task is created in the RUN state.
     * @param stack_size the size of the stack in words (=4 bytes)
     */
    TaskBase(const char* task_name, uint32 *stack, uint32 stack_size) 
    : m_task_name(task_name), m_state(RUN), m_priority(PRIORITY_DEFAULT), 
    m_stack(stack), m_stack_pointer(stack + stack_size - 1), m_stack_size(stack_size) {
      tag_stack();
    }
    
    /**
     * The task's top function.
     * The concrete task must implement this function!
     * It is not pure virtual because it is instantiated once for representing
     * the main task.
     */
    virtual void run();
        
  private:
    /**
     * Give access to the OS
     */
    friend class OS;
    
    /**
     * Special constructor for task 0
     */
    TaskBase(): m_task_name("MAIN"), m_state(RUN), m_priority(PRIORITY_DEFAULT), m_stack(0), m_stack_pointer(0), m_stack_size(0) {
    }
    
    /**
     */
    const char* get_name() const {
      return m_task_name;
    }
    
    /**
     * @return this task's state
     */
    State get_state() const {
      return m_state;
    }
    
    /**
     * Enables the OS to mark the status of this task
     */
    void set_state(State state) {
      m_state = state;
    }
    
    /**
     * This the top of the task.
     */
    static void top(TaskBase *instance) {
      instance->run();
    }
    
    /**
     * The stack is marked with a pattern.
     */
    static const uint32 TAG = 123456789;
    void tag_stack() {
      uint32 *p = const_cast<uint32*>(m_stack);
      while (p < m_stack + m_stack_size - 1) {
        *p = TAG;
        p++;
      }
    }
    
    uint32 get_stack_size() const {
      return m_stack_size;
    }
    
    uint32 get_stack_usage() const {
      const uint32 *p = m_stack;
      
      while ((p != m_stack + m_stack_size) && (*p == TAG)) {
        p++;
      }
      return m_stack_size - (p - m_stack);
    }
    
    const char* m_task_name;
    State m_state;
    
    const Priority m_priority;
    
    const uint32 *m_stack;
    const uint32 *m_stack_pointer;
    const uint32 m_stack_size;
    
  private:
    /**
     * This is so that tasks can be list nodes
     */
    friend class util::List<this_type>;
    friend class util::ListIterator<this_type>;
    
    TaskBase *m_pred;
    TaskBase *m_succ;
    
  private:

    /**
     * Set of task registers
     */
    union task_regs {
      struct {
        uint32 r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;
        uint32* sp;
        uint32 lr;
        uint32 cpsr;
      };
      uint32 r[16];
    };
    task_regs m_task_regs;
  };
  
  inline
  void TaskBase::run() {
    // Do nothing
  }
}
