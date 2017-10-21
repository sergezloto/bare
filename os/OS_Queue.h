/*
 *  OS_Queue.h
 *  Embedded
 *
 *  Created by Serge on 09/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace os {
  
  /**
   * A queue with typed objects.
   * A task trying to post to a queue that is full, or to receive from an empty
   * queue is put into sleeping mode, and is woken up when the action becomes
   * possible.
   * Objects stored by the queue are copied bytewise by value, so only small objects
   * may be stored. No deep-copy is performed, nor any copy constructor is called,
   * so objects with deep-copy semantics should not be used with this queue.
   */
  template <typename T, uint32 N> class Queue {
  public:
    Queue();
    
    /**
     * Posts an item to the queue, possibly blocking for at most
     * waitTimeMs until a slot becomes available
     * @return true if the item was posted
     */
    bool put(const T& item, uint32 waitTimeMs);
    
    /**
     * Gets an item from the queue, possibly blocking for at most
     * waitTimeMs until available
     * return true if the item was received
     */
    bool get(T& item, uint32 waitTimeMs);
    
    /**
     * @return the number of item currently in the queue
     */
    uint32 size() const;
    
  private:
  };
}
