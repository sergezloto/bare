/*
 *  os.cpp
 *  Embedded
 *
 *  Created by Serge on 06/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "OS_os.h"
#include "../platform/HAL_Processor.h"


namespace os {
  uint32 OS::m_task_num = 1;

  OS::task_queue_type OS::m_run_queue;
  OS::task_queue_type OS::m_sleep_queue;
  OS::task_queue_type::iterator OS::m_current_task(0);

  TaskBase OS::m_task_main;
  
  bool OS::m_started = false;
  uint32 OS::m_critical = 0;
  
  static void switch_context(void* from, void* to) __attribute__((naked));
  static void switch_context(void* from, void* to) {
#ifdef __GNUC__
#  if defined(__thumb2__)
#      error THUMB2 not supported yet
#  elif defined(__arm__)
#    if defined(__thumb__)
    // A shim to switch from thumb to arm mode
    asm("  ldr    r2, =arm_switch_context \n"
        "  bx     r2"
        : // output
        : // input
        : // clobbered
        "r2"
        );
#    endif
    asm("  .arm \n"
        "  .align 4 \n"
        "arm_switch_context: \n"
        "       @ Save non-apcs registers. We save r12(ip) and the cpsr, not really needed. \n"
        "  add     r0, r0, #4 * 4    @ skip r0 to r3, no need to save since it is a cooperative switch \n"
        "  stmia   r0!, {r4-r12, sp, lr} \n"
        "  mrs     r2, cpsr \n"
        "  stmia   r0!, {r2}         @ status register too \n"
        
        "  @ Restore from the area pointed by r1 \n"
        "  add     r1, r1, #4 * 4    @ skip r0-r3 \n"
        "  ldmia   r1!, {r4-r12, sp, lr} \n"
        "  ldmia   r1!, {r0} \n"
        "  msr     cpsr_f, r0 \n"
        "  mov     r0, r4                @ so that r0 is (a TaskBase*) when calling the task top 1st time -- hack! \n"
        "  bx lr                         @ resume execution \n"
        : // Output
        : // Input
        : // Clobbered
        "r0", "r1", "r2"
    );
#  endif
#else
#  error Compiler not yet supported
#endif
  }

  void OS::init_tasks() {
    //out << "OS: init_tasks, " << m_task_num << " tasks\n";
    m_run_queue.add_head(&m_task_main);
    //out << "OS: done setup task 0\n";
    for (task_queue_type::iterator t = m_run_queue.begin(); t != m_run_queue.end(); ++t) {
      if ((*t) != m_task_main)
        init_task(&(*t));
    }
    m_current_task = m_run_queue.begin();
  }
  
  void OS::init_task(TaskBase *task) {
    //out << "OS: init_task(" << task_num << ")\n";
    const uint32 cpsr = hal::Processor::get_cpsr();
    //out << "OS: cpsr is " << cpsr << "\n";
    
    TaskBase *t = task;
    TaskBase::task_regs &regs = t->m_task_regs;
    // setup instance address (the this pointer)
    regs.r4 = (uint32)t;   // MAJOR HACK! r4 will be copied to r0 during context switch. Only for cooperative OS!!
    // setup member function to call
    void (*const top_function)(TaskBase*) = &TaskBase::top;
    regs.lr = (uint32) top_function;
    // setup the stack for the task
    const uint32 *stack_end = t->m_stack_pointer;
    regs.sp = const_cast<uint32*>(stack_end);
    // setup the cpsr
    regs.cpsr = cpsr;
  }
  
  void OS::suspend() {
    TaskBase *task = &*m_current_task;
    // Remember this task's registers
    TaskBase::task_regs& current_task_regs = (*m_current_task).m_task_regs;

    // Mark it as sleeping
    (*m_current_task).set_state(TaskBase::SUSPENDED);
    
    // Choose the next task
    ++m_current_task;
    
    if (!m_current_task)
      m_current_task = m_run_queue.begin();


    // Move the task from the RUN to the SLEEP queue
    m_run_queue.remove(task);
    m_sleep_queue.add_head(task);
        
    TaskBase::task_regs& next_task_regs = (*m_current_task).m_task_regs;
    // Perform the switch...
    switch_context(&current_task_regs, &next_task_regs);
  }
  
  void OS::wakeup(TaskBase* task) {
    if (task->get_state() == TaskBase::RUN) {
      // Nothing to do
      return;
    }
    
    // Put it in the run state
    task->set_state(TaskBase::RUN);
    
    // Remove from sleep queue and put at the tail of the run queue
    m_sleep_queue.remove(task);
    m_run_queue.add_tail(task);
  }
  
  void OS::yield() {
    if (!m_started)
      return;
    if (m_critical > 0)
      return;
    
    //out << "OS: yield current task " << m_current_task_index << ", " << m_task_num << " tasks\n";
    // Remember this task's registers
    TaskBase::task_regs& current_task_regs = (*m_current_task).m_task_regs;
    
    // Choose the next task
    ++m_current_task;
    
    if (!m_current_task)
      m_current_task = m_run_queue.begin();
    
    TaskBase::task_regs& next_task_regs = (*m_current_task).m_task_regs;

    // Perform the switch...
    switch_context(&current_task_regs, &next_task_regs);
  }  
}
