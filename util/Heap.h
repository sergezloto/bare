/*
 *  Heap.h
 *  Embedded
 *
 *  Created by Serge on 6/17/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */
#pragma once

#if defined(__EMBEDDED__)
#include "base.h"

#include "bget.h"
#include "mem.h"

#include "ostream.h"

/**
 * A memory heap manager on top of bget. The heap is allocated in an array of characters.
 * The lifetime of the heap MUST be greater than any object allocated in it.
 */
namespace util {
  template<uint32 SIZE> class Heap: public NoCopy {
  public:
    /**
     * Constructor. registers the pool to bget
     */
    Heap() {
      bpool(m_pool, SIZE);
    }
    
    void *malloc(uint32 size) {
      return bget(size);
    }
    
    void *calloc(uint32 element_count, uint32 element_size) {
      return bgetz(element_count * element_size);
    }
    
    void free(void *ptr) {
      brel(ptr);
    }
    
    void *realloc(void *ptr, uint32 new_size) {
      return bgetr(ptr, new_size);
    }
    
    struct stats_type {
      ::bufsize curalloc,totfree,maxfree, nget, nrel;
    };
    
    void get_stats(stats_type& stats) const {
      ::bstats(&stats.curalloc, &stats.totfree, &stats.maxfree, &stats.nget, &stats.nrel);
    }
    
    bool is_valid() const {
      return ::bpoolv(const_cast<uint8*>(m_pool));
    }
    
    template <class OS>
    void dump_stats(OS& os) const {
      stats_type stats;
      get_stats(stats);
      os << "HEAP STATS: "
      << " curalloc=" << stats.curalloc
      << " totfree=" << stats.totfree
      << " maxfree=" << stats.maxfree
      << " nget=" << stats.nget
      << " nrel=" << stats.nrel;
    }
      
    static const uint32 size = SIZE;
    
  private:
    uint8 m_pool[SIZE];
  };  
}

#endif
