/*
 *  peb_header.h
 *  Embedded
 *
 *  Created by Serge on 18/02/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "nor_types.h"
#include "util.h"  // for CRC32

namespace lpcnor {
  
  static const uint16 PEB_MAGIC = 0xacdc;
  
  /**
   * Each on-flash erase block begins with this header.
   * @peb_magic:	so as to recognize a managed flash area
   * @peb_leb:		logical erase block number for this PEB
   * @peb_leb_generation: allows to choose the latest of two PEB versions
   * @peb_transferred_slots: the number of slots (vacant or not) transferred during switch
   * @peb_flags:	no use for now
   * @peb_erase_count:	the number of time the PEB has been erased
   * @crc:		crc32 for the peb for integrity
   * The header size is fixed at 16 bytes.
   */
  struct peb_header_type {
    typedef peb_header_type this_type;
    void reset() {
      memset(this, 0xff, sizeof *this);
    }
    
    util::CRC32::result_type calc_crc32() const {
      using util::CRC32;
      const uint8* address = reinterpret_cast<const uint8*>(this);
      const uint32 sz = sizeof *this - sizeof peb_header_crc32;
      CRC32 crc32;
      CRC32::result_type crc = crc32.process(address, sz).get_result();
      return crc;
    }
    
    bool check() const {
      return peb_magic == PEB_MAGIC && check_crc32();
    }
    
    slot_index_type transferred_slots() const {
      return peb_transferred_slots;
    }
    
    bool is_leb_more_recent(const this_type& ph) const {
      const bool more_recent = 
      (peb_leb_generation == (ph.peb_leb_generation + 1) & 0x3)
      || (peb_leb_generation == (ph.peb_leb_generation + 2) & 0x3);
      return more_recent;
    }
    
    uint32 next_leb_generation() const {
      return (peb_leb_generation + 1) & 0x3;
    }
    
    bool check_crc32() const {
      return peb_header_crc32 == calc_crc32();
    }

    static uint32 size() {
      return sizeof(this_type);
    }
    
    uint16 peb_magic;
    leb_number_type peb_leb;
    unsigned peb_leb_generation: 2;
    unsigned peb_reserved0: 6;
    uint8 peb_reserved1;
    unsigned peb_erase_count: 24;
    slot_index_type peb_transferred_slots;
    uint16 peb_reserved2;
    uint32 peb_header_crc32;
    
  private:
    this_type& operator=(const this_type&);
  } ZOROBO_PACKED;
  ZOROBO_CHECK_SIZE(peb_header_type, PEB_WRITE_LINE_SIZE)
}
