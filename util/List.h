/*
 *  List.h
 *  Embedded
 *
 *  Created by Serge on 12/11/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"

namespace util {
  
  /**
   * Iterator model
   */
  template <class NODE>
  class ListIterator {
  public:
    ListIterator(NODE* node) : m_node(node) {
    }
    
    ListIterator& operator++() {
      m_node = m_node->m_succ;
      return *this;
    }
    
    ListIterator& operator--() {
      m_node = m_node->m_pred;
      return *this;
    }
    
    NODE& operator*() {
      return *m_node;
    }
    
    operator bool() const {
      return m_node != 0;
    }
    
  private:
    NODE* m_node;
  };
  
  /**
   * Const Iterator model
   */
  template <class NODE>
  class ConstListIterator {
  public:
    ConstListIterator(const NODE* node) : m_node(node) {
    }
    
    ConstListIterator& operator++() {
      m_node = m_node->m_succ;
      return *this;
    }
    
    ConstListIterator& operator--() {
      m_node = m_node->m_pred;
      return *this;
    }
    
    const NODE& operator*() const {
      return *m_node;
    }
    
    operator bool() const {
      return m_node != 0;
    }
    
  private:
    const NODE* m_node;
  };
  
  /**
   * A generic double linked list that does not allocate any memory.
   * Instead, it is templated on node objects, which must support the following fields:
   * - m_succ
   * - m_pred
   * which are pointers to other nodes.
   * It means that objects can't belong to two lists at the same time!
   * The nodes must have a lifetime at least as great as the list itself.
   *
   */
  template <class NODE> 
  class List {
  public:
    typedef List this_type;
    typedef NODE value_type;
    typedef NODE* pointer_type;
    typedef NODE& ref_type;
    typedef ListIterator<NODE> iterator;
    typedef ConstListIterator<NODE> const_iterator;
    
    List(): m_head(0), m_tail(0) {
      // An empty list is marked by m_head == 0
    }
    
    void add_head(NODE* n) {
      n->m_pred = 0;
      n->m_succ = m_head;
      if (m_head == 0) {
        m_tail = n;
      } else {
        m_head->m_pred = n;        
      }
      m_head = n;
    }
    
    void add_tail(NODE* n) {
      n->m_succ = 0;
      n->m_pred = m_tail;
      if (m_tail == 0) {
        m_head = n;
      } else {
        m_tail->m_succ = n;        
      }
      m_tail = n;
    }
    
    NODE* remove_head() {
      NODE* head = m_head;
      
      remove(head);
      return head;
    }
    
    void remove(NODE* n) {
      NODE* pred = n->m_pred;
      NODE* succ = n->m_succ;

      if (pred == 0) {
        // This was the head node. New head is succ then
        m_head = succ; // head is 0 n was only node
      } else {
        pred->m_succ = succ;
      }
      
      if (succ == 0) {
        // This was the tail as well. Reconnect
        m_tail = pred;  // tail is 0 if n was only node
      } else {
        succ->m_pred = pred;
      }

      // Invalidate node
      n->m_succ = 0;
      n->m_pred = 0;
    }
    
    const_iterator begin() const {
      return const_iterator(m_head); 
    }
    
    const_iterator end() const {
      return 0; 
    }
    
    iterator begin() {
      return iterator(m_head); 
    }
    
    iterator end() {
      return 0; 
    }
    
  private:
    
    NODE *m_head;
    NODE *m_tail;
  };

  
  /**
   * A node adapter for objects that cannot be modified to participate in a list.
   * It stores objects by value (but pointers to objects can be stored!)
   */
  template <typename T>
  class NodeAdapter {
  public:
    NodeAdapter(T& value) : m_value(value) {
    }
  private:
    NodeAdapter* m_node_pred;
    NodeAdapter* m_node_succ;
    T m_value;
    
    friend class List<NodeAdapter<T> >;
  };
}
