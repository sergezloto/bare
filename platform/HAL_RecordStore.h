/*
 *  HAL_RecordStore.h
 *  Embedded
 *
 *  Created by Serge on 7/02/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "base.h"
#include "util.h"
#include "HAL_IAP.h"
#include "HAL_Flash.h"

#if DEBUG
namespace sysconf {
  // access the global debug stream
  extern util::stringstream<256> ss;
}

#  define DEBUG_RS 1
#endif

namespace hal {
  typedef uint8 peb_number_type;
  typedef uint8 leb_number_type;

  /**
   * Our default setup, as an example traits type
   * for the media_type class
   */
  struct recordstore_sample_traits {
    enum {
      // Number of physical erase blocks
      PEB_NB = 3,
      // Size in byte for each PEB
      PEB_SIZE	= 1 << 12
    };
    
    static const uint8* peb_address(peb_number_type peb);
    static bool peb_erase(peb_number_type peb);
    static bool peb_program(peb_number_type peb, uint32 offset, const void* data, uint32 data_size);
    static bool peb_is_blank(peb_number_type peb);
  };
  
  /**
   * Manages contiguous, same-size flash PEB (physical erase blocks or sectors) to provide
   * a transparent wear-levelling record store.
   * There must be at least two sectors comprising the record store.
   * It is parametrized with the traits type with:
   * SEC_BEGIN, SEC_END
   */
  template <typename T>
  class RecordStore: NoInstance {
  private:
    typedef RecordStore<T> this_type;
    
    static const uint32 SEC_NB = T::PEB_NB;
    static const uint32 SEC_SIZE = T::PEB_SIZE;
    static const uint32 SEC_WRITE_LINE_SIZE = 16;    
    static const uint32 NB_LEB = NB_PEB - 1;
        
    static const peb_number_type PEB_INVALID = 0xff;
    static const leb_number_type LEB_INVALID = 0xff;
        
    /**
     * Each on-flash erase block begins with this header
     */
    struct peb_header_type {
      void reset() {
        memset(this, 0xff, sizeof *this);
      }
      
      util::CRC32::result_type calc_crc32() const {
        using util::CRC32;
        const uint8* address = reinterpret_cast<const uint8*>(this);
        const uint32 size = sizeof *this - sizeof peb_header_crc32;
        CRC32 crc32;
        CRC32::result_type crc = crc32.process(address, size).get_result();
        return crc;
      }
      
      uint32 peb_magic;	// must be PEB_PEB or PEB_SPARE
      uint32 peb_erase_count;	// beging at 1, incremented with each erase operation
      uint16 peb_leb;	// Logical sector number
      unsigned peb_gen: 2;
      unsigned peb_reserved: 14;
      uint32 peb_header_crc32;      
    private:
      typedef peb_header_type this_type;
      this_type& operator=(const this_type&);
    } ZOROBO_PACKED;
    ZOROBO_CHECK_SIZE(peb_header_type, SEC_WRITE_LINE_SIZE)
    
    /**
     * Our spare is also here
     */
    static peb_number_type m_peb_spare;
    
    /**
     * We keep track of lebs using this table
     * @switch_count: Everytime a LEB is switched, we increase a generation counter.
     *                This way, cached record offsets can be invalidated
     */
    struct leb_info_type {
      void reset() {
        peb = PEB_INVALID;
        switch_count = 0;
        free = 0;
        reclaimable = 0;
      }
      peb_number_type peb;
      uint16 switch_count;
      uint16 free;
      uint16 reclaimable;
    };
    static leb_info_type m_leb_info[NB_LEB];
    
    /**
     * A record begins with this 16 bit magic number
     */
    enum record_magic_type {
      RECORD_MAGIC	=	0xfede
    };
    
    /**
     * Record information flags.
     * @BIT_RECORD_ACTIVE: if 0, this record is deleted
     * @BIT_RECORD_KEEP: if 0, this record is not permanent and may be lost during PEB switchover
     */
    enum record_header_flags {
      BIT_RECORD_KEEP	=	1 << 1
    };
    
    /**
     * The on-flash record header
     * @rec_magic: marker for a data record. Should be RECORD_MAGIC.
     * @rec_sequence: increased monotonically for each new record. Uniquely identifies a record!
     * @rec_size: the record content length in bytes
     * The record crc comes immediately after the record content. A record with an invalid crc is invalid or deleted
     */
    union record_header_type {
      void reset() {
        for (uint32 i = 0; i < sizeof all4 / sizeof all4[0]; ++i) {
          all4[i] = ~0;
        }
      }
      bool is_newer_than(const record_header_type& other) const {
        return rec_sequence > other.rec_sequence;
      }
      util::CRC32::result_type calc_crc32() const {
        // TODO: recode this
//        using util::CRC32;
//        const uint8* address = reinterpret_cast<const uint8*>(this);
//        const uint32 size = sizeof *this - sizeof this->crc32;
//        CRC32 crc32;
//        CRC32::result_type crc = crc32.process(address, size).get_result();
//        return crc;
      }
      uint16 rec_magic;
      uint32 rec_sequence;
      uint16 rec_size;
      uint8 rec_content[0];

    private:
      typedef record_header_type this_type;
      this_type& operator=(const this_type&);
    } ZOROBO_PACKED;
    ZOROBO_CHECK_SIZE(record_header_type, 

  public:
    
    /**
     * Sets up the record store structure in memory.
     * The sectors will be formatted if needed.
     */
    static void configure() {
      // Reset all LEB
      for (uint32 leb = 0; leb < NB_LEB; ++leb) {
        m_leb_info[leb].reset();
      }
      // No spare so far
      m_peb_spare = PEB_INVALID;
      
      // Scan all PEB
      for (uint32 peb = SEC_BEGIN; peb <= SEC_END; ++peb) {
        peb_info_type& pi = peb_info(peb);
        pi.type = peb_check(peb);
        
        switch (pi.type) {
          case PEB_LEB:
          {
            const peb_header_type& ph = peb_header(peb);
            const leb_number_type leb = ph.peb_leb;
            if (leb < NB_LEB) {
              // LEB within range.
              // Check if not duplicate
              if (m_leb_info[leb].peb != LEB_INVALID) {
                // Duplicate! Arbitrage.
                // For now the previous wins
                pi.good = false;
              } else {
                pi.good = true;
                pi.leb = leb;
                m_leb_info[leb].peb = peb;
              }
            } else {
              // Weird LEB#. This is not a LEB, hence not a good PEB
              pi.good = false;
            }
            // Keep the erase count no matter what
            pi.erase_count = ph.peb_erase_count;
          }
            break;
          case PEB_SPARE:
          {
            const peb_header_type& ph = peb_header(peb);
            if (m_peb_spare == PEB_INVALID) {
              pi.good = true;
              m_peb_spare = peb;
            } else {
              // We have already seen a spare! This is bogus and should not happen
              pi.good = false;
            }
            // Keep the erase count no matter what
            pi.erase_count = ph.peb_erase_count;
          }
            break;
          case PEB_NOT_PEB:
          {
            pi.good = false;
            pi.erase_count = 0;
          }
            break;
        }
#if DEBUG
        sysconf::ss << "PEB#" << peb << " typ=" << (uint32)pi.type << " good=" << pi.good << " ec=" << pi.erase_count <<  "\n";
#endif
      }
#if DEBUG
      sysconf::ss << "SPARE#" << m_peb_spare << "\n";
#endif
      // Scan for missing stuff
      peb_number_type last_scanned_peb = SEC_BEGIN;
      
      // A missing LEB corresponds to a missing PEB. Make them both
      for (uint32 leb = 0; leb < NB_LEB; ++leb) {
        if (m_leb_info[leb].peb != PEB_INVALID) {
          // No problem, next LEB
          continue;
        }
        // Missing. Get a vacant PEB (not PEB or PEB but not good)
        for (uint32 peb = last_scanned_peb; peb <= SEC_END; ++peb) {
          peb_info_type& pi = peb_info(peb);
          if (pi.type == PEB_NOT_PEB || !pi.good) {
            // Make a PEB with current LEB
            const bool ok = peb_init_leb(peb, leb, pi.erase_count);
            // We tried to init it as a LEB. remember this
            pi.type = PEB_LEB;
            if (ok) {
              pi.good = true;
              pi.leb = leb;
              ++pi.erase_count;
              
              m_leb_info[leb].peb = peb;  // cross ref too
              
              break;  // next LEB
            } else {
              // We couldn't init the sector! Remember that
              pi.good = false;
            }
            
            last_scanned_peb = peb;
          }
        }
      }
      
      // Missing spare?
      if (m_peb_spare == PEB_INVALID) {
        // We need a spare. Find a PEB for this
        for (uint32 peb = SEC_BEGIN; peb <= SEC_END; ++peb) {
          peb_info_type& pi = peb_info(peb);
          if (pi.type == PEB_NOT_PEB || !pi.good) {
            const bool ok = peb_init_spare(peb, pi.erase_count);
            pi.type = PEB_SPARE;
            if (ok) {
              pi.good = true;
              m_peb_spare = peb;
              break;  // done with spare
            }
            pi.good = false;
          }
        }
      }
      
      // Walk records and find out free and reclaimable space for each PEB
      for (uint32 peb = SEC_BEGIN; peb <= SEC_END; ++peb) {
        peb_info_type& pi = peb_info(peb);
        if (pi.type != PEB_LEB || !pi.good) {
          // Skip
          continue;
        }
        
        // Scan PEB for records. We start right after the PEB header
        pi.free = SEC_SIZE - sizeof(peb_header_type);
        pi.reclaimable = 0;
        const record_header_type* rh = rec_peb_first_header(peb);
        while (rh != 0) {
          switch (record_check(*rh)) {
            case RECORD_DELETED:
              pi.reclaimable += rec_bytes(*rh);
              /* FALLTHROUGH */
            case RECORD_RECORD:
              pi.free -= rec_bytes(*rh);            
              rh = rec_next_header(*rh);
              break;
            case RECORD_NOT_RECORD:
              // Can't process further
              rh = 0;
              break;;
          }
        }
      }
    }

    /**
     * Obtains a record handle for a given identifier.
     * All LEBs are scanned. Duplicate records are scheduled for deletion.
     */
    static record_handle_type record_get(uint32 ident) {
      record_handle_type handle(ident);
      
      // Memorize the record header
      record_header_type* rh_found = 0;

      //
      // Find record in all LEBs. Walk a LEB up to its free space
      // If multiple records found, choose one closer to end of LEB
      // And schedule others for deletion (flag ACTIVE to 0)
      //
      for (leb_number_type leb = 0; leb < NB_LEB; ++ leb) {
        const bool found = record_find_in_leb(handle);
      }
      
      return handle;
    }
    
#if DISABLE_FOR_NOW
    /**
     * Find the last instance of a record, believed to exist, in its LEB.
     * If the record generation matches the LEB switch count, the handle is current
     * and no further lookup is necessary
     * The record offset may be modified if necessary.
     * @return false if the record is not found
     */
    static bool record_find_in_leb(leb_number_type leb, record_handle_type& handle) {
      const leb_number_type leb = handle.leb;
      const leb_info_type& li = m_leb_info[leb];
      if (handle.switch_generation == li.switch_count) {
        // All good
        return true;
      }
      // We have to adjust offset in LEB. Find the record in whole LEB
      const peb_number_type peb = li.peb;
      const peb_info_type& pi = peb_info(peb);
      record_header_type* rh = rec_peb_first_header(peb);
      while (rh != 0) {
        switch (record_check(*rh)) {
          case RECORD_RECORD:
            if (rh->rec_ident == handle.ident) {
              bool retain_current = true;  // Assume current is the one we are looking for
              if (rh_found != 0) {
                // Duplicate. Which one to keep?
                if (rh->is_newer_than(*rh_found)) {
                  peb_record_delete(peb, *rh_found);                  
                } else {
                  peb_record_delete(peb, *rh);
                  retain_current = false;  // Not newer, get rid of it
                }
              }
              
              // Remember record details
              handle.exists = true;
              if (retain_current) {
                handle.leb = leb;
                handle.header_offset = record_get_offset_in_leb(rh);
                handle.switch_generation = m_leb_info[leb].switch_count;
                
                rh_found = rh;
              }
            }
            /* FALLTHROUGH */  // To next record
          case RECORD_DELETED:
            // Next record
            rh = rec_next_header(*rh);
            break;
          case RECORD_NOT_RECORD:
            // Can't process further
            rh = 0;
            break;;
        }
      }
    }
    
    /**
     * Finds a record header of a given identifier
     */
#endif
    
#if DISABLE_FOR_NOW
    /**
     * Schedules the record for deletion
     */
    static bool record_delete(record_handle_type& handle) {
      if (!handle.exists) {
        // Do nothing. Pretend it worked
        return true;
      }
      // Use cached LEB/offset if LEB was not switched
      const leb_number_type leb = handle.leb;
      if (handle.switch_generation != m_leb_info[leb].switch_count) {
        // TODO: LEB was switched to spare, so offset is no longer valid. 
        // TODO: We have to find it and update handle
        // If it no longer exists all is fine...
      }
      const peb_number_type peb = m_leb_info[leb].peb;
      record_header_type& rh = peb_rec_header(peb, offset);
      const bool ok = peb_record_delete(peb, rh);
      if (ok) {
        // Mark handle invalid
        handle.exists = false;
      }
      return ok;
    }
    
    /**
     * Writes a record
     * Possible errors: write error, no space left
     */
    static bool record_write(record_info_type& record, const uint8* data, uint32 size) {
      // Find space for new record, possibly switch LEB and spare (but without copying the current record if any)
      // Write record
      // if good, commit the switchover if any
      // Mark previous, if any, as not active
      // Increase switch count
      ++m_leb_info[leb].switch_count;
    }
#endif
    
#if DEBUG
    static uint32 stat_peb_free(peb_number_type peb) {
      return peb_info(peb).free;
    }
    static uint32 stat_peb_reclaimable(peb_number_type peb) {
      return peb_info(peb).reclaimable;
    }
#endif
  private:
    /**
     * @return a hashcode for a record identifier
     */
    static uint32 hash(uint32 a) {
      a -= (a<<6);
      a ^= (a>>17);
      a -= (a<<9);
      a ^= (a<<4);
      a -= (a<<3);
      a ^= (a<<10);
      a ^= (a>>15);
      return a;
    }
    
    /**
     * From identifier to index in the hashtable
     */
    static uint32 hash_index(uint32 a) {
      a = hash(a);
      a &= NB_RECORDS_BITS - 1;
      return a;
    }
    
    /**
     * For an identifier, return its hash table info entry
     * WE IGNORE COLLISIONS!!
     */
    static record_info_type& get_info(uint32 ident) {
      const uint32 index = hash_index(ident);
      const record_info_type& ri = m_record_info_table[index];
      return ri;
    }
#if DISABLE_FOR_NOW
    /**
     * Scan all records, filling m_record_info_table hashtable
     */
    static void rec_set_hashtable() {
      // Reset all
      for (uint32 i = 0; i < NB_RECORDS; ++i) {
        m_record_info_table[i].reset();
      }
      
      // All LEBs
      for (leb_number_type i = 0; i < NB_LEB; ++i) {
        const peb_number_type peb = m_leb_info[i];
        // Scan all records
        record_header_type* rh = rec_peb_first_header(peb);
        while (rh != 0) {
          const record_header_type rht = record_check(*rh);
          switch (rht) {
            case RECORD_RECORD:
            {
              bool retain_current = true;
              record_info_type& ri = get_info(rht.rec_ident);
              if (ri.valid()) {
                // Identified already scanned!
                // Choose one based on generation
                if (rh->
              }
#error IMPLEMENT
              // Current one, remember it in the hash
            }
              // Next record
              rh = rec_next_header(*rh);
              break;
            case RECORD_DELETED:
              // Next record
              rh = rec_next_header(*rh);
              break;
            case RECORD_NOT_RECORD:
              // Stop the scan
              rh = 0;
              break;
          }
        }
      }
    }
#endif
                  
    /**
     * Decides which peb (not spare) to be used as next spare
     * TODO: Find something better than: for now peb with the lowest erase count
     */
    static peb_number_type peb_choose_as_spare() {
      const peb_number_type chosen_peb = PEB_INVALID;
      
      for (uint32 peb = SEC_BEGIN + 1; peb <= SEC_END; ++peb) {
        const peb_info_type& pi = peb_info(peb);
        if (pi.type != PEB_LEB || pi.good) {
          continue;
        }
        if (chosen_peb == PEB_INVALID || pi.erase_count < peb_info(chosen_peb).erase_count) {
          chosen_peb = peb;
        }
      }
      return chosen_peb;
    }
    
    /**
     * Switches over a LEB to the spare
     */
    static bool leb_switch(leb_number_type leb) {
      // Init spare as peb/leb
      // Copy LEB record to new peb
      // Set up pointers
      // Init ex peb as spare
      // Invalidate generation counte for this LEB
      ++m_leb_info[leb].switch_count;
      return false;
    }
    
    /**
     * Finds a suitable leb to write a record to
     */
    
    /**
     * @return  a reference to the requested peb header
     * Note, the requested PEB must be valid, as a LEB or spare
     */
    static const peb_header_type& peb_header(peb_number_type peb) {
      const uint8* address; uint32 size;
      Flash::sector_to_address_size(peb, address, size);
      return *reinterpret_cast<const peb_header_type*>(address);
    }
    
    /**
     * @return a reference to a peb info record for the given peb
     */
    static peb_info_type& peb_info(peb_number_type peb) {
      return m_peb_info[peb - SEC_BEGIN];
    }
    
    /**
     * Sets up a peb for leb
     */
    static bool peb_init_leb(peb_number_type peb, leb_number_type leb, uint32 erase_count) {
#if DEBUG_RS
      sysconf::ss << __func__ << "\n";
#endif
      
      bool ok = peb_blank(peb);
      if (!ok) {
        return false;
      }
      
      ok = peb_write_leb_header(peb, leb, erase_count + 1);
      
      return ok;
    }
    
    /**
     * Sets up a peb for spare
     */
    static bool peb_init_spare(peb_number_type peb, uint32 erase_count) {
#if DEBUG_RS
      sysconf::ss << __func__ << "\n";
#endif
      
      bool ok = peb_blank(peb);
      if (!ok) {
        return false;
      }
      
      ok = peb_write_spare_header(peb, erase_count +1);
      
      return ok;
    }
    
    /**
     * Check the type of the given peb header
     */
    static peb_type peb_check(peb_number_type peb) {
      const peb_header_type& ph = peb_header(peb);
      
      switch (ph.peb_magic) {
        case PEB_LEB:
        case PEB_SPARE:
          // Ok, proceed to check crc32
          break;
        default:
          // Not a peb
          return PEB_NOT_PEB;
      }
      
      const util::CRC32::result_type crc = ph.calc_crc32();
      if (crc != ph.peb_header_crc32) {
        return PEB_NOT_PEB;
      }
      return static_cast<peb_type>(ph.peb_magic);
    }
    
    /**
     * Formats a PEB for use with the record store.
     * Assumes this is the first time ever the PEB is formatted
     */
    static bool peb_blank(peb_number_type peb) {
#if DEBUG_RS && 0
      sysconf::ss << __func__ << "\n";
#endif
      // Sector blank?
      Flash::blank_check_status_type code = Flash::blank_check_sectors(peb, peb);
      if (code == Flash::BC_BLANK) {
        // Already blank, nothing to do
        return true;
      }
      
      // Not blank. format it
      const bool ok = Flash::erase_sectors(peb, peb);
      
      return ok;
    }
    
    /**
     * Writes a spare header to a peb. Assumes the peb is blank
     */
    static bool peb_write_spare_header(peb_number_type peb, uint32 erase_count) {
#if DEBUG_RS && 0
      sysconf::ss << __func__ << "\n";
#endif
      peb_header_type h;
      h.reset();  // Init with all 0xff
      h.peb_magic = PEB_SPARE;
      h.peb_leb = LEB_INVALID;
      h.peb_erase_count = erase_count;
      h.peb_header_crc32 = h.calc_crc32();
      
      return peb_write_header(peb, h);
    }
    
    /**
     * Writes a leb header to a peb. Assumes the peb is blank
     */
    static bool peb_write_leb_header(peb_number_type peb, leb_number_type leb, uint32 erase_count) {
#if DEBUG_RS && 0
      sysconf::ss << __func__ << "\n";
#endif
      peb_header_type ph;
      ph.reset();  // Init with all 0xff
      ph.peb_magic = PEB_LEB;
      ph.peb_leb = leb;
      ph.peb_erase_count = erase_count;
      ph.peb_header_crc32 = ph.calc_crc32();
      
      return peb_write_header(peb, ph);
    }
    
    /**
     * Writes a PEB header
     */
    static bool peb_write_header(peb_number_type peb, peb_header_type& header) {
#if DEBUG_RS && 0
      sysconf::ss << __func__ << "\n";
#endif
      // Cook up the crc32
      // We are writing right at sector start
      const uint8* dest_address = 0;  // Initialized to zero to please gcc
      uint32 size;
      Flash::sector_to_address_size(peb, dest_address, size);
      const bool ok = Flash::copy_ram_to_flash(peb, dest_address, &header, sizeof header);
#if DEBUG_RS
      sysconf::ss << "flash#" << peb << ":" << (uint32)dest_address << "->" << (uint32)Flash::get_last_code() << "\n";
#endif
      
      return ok;
    }
    
    /**
     * @return the on-flash size of a (valid) record
     */
    static uint32 rec_bytes(const record_header_type& header) {
      const uint32 on_flash_size = sizeof header + ((header.rec_size + SEC_WRITE_LINE_SIZE - 1) & SEC_WRITE_LINE_SIZE_ALIGN_MASK);
      return on_flash_size;
    }
    
    /**
     * @return the first record header in a given PEB.
     */
    static const record_header_type* rec_peb_first_header(peb_number_type peb) {
      const uint8* address; uint32 size;
      Flash::sector_to_address_size(peb, address, size);
      const uint8* const next_record_address = address + sizeof(peb_header_type);
      return reinterpret_cast<const record_header_type*>(next_record_address);
    }
    
    /**
     * @return a record header reference given an address
     */
    static const record_header_type& rec_header(const void* address) {
      return *reinterpret_cast<const record_header_type*>(address);
    }
    
    /**
     * @return a pointer to the next record header. The record has to checked for validity
     */
    static const record_header_type* rec_next_header(const record_header_type& rec) {
      const uint8* address = reinterpret_cast<const uint8*>(&rec) + rec_bytes(rec);
      return reinterpret_cast<const record_header_type*>(address);
    }
    
    /**
     * @return true if a record header is valid
     */
    static record_type record_check(const record_header_type& header) {
      if (header.rec_magic != RECORD_MAGIC) {
        return RECORD_NOT_RECORD;
      }
      // If record was deleted, no need to check crc32 (it will be wrong anyway)
      if (header.rec_flags & BIT_RECORD_ACTIVE == 0) {
        // Record deleted, and therefore valid
        return RECORD_DELETED;
      }
      uint32 crc32 = header.calc_crc32();
      if  (header.crc32 == crc32) {
        return RECORD_RECORD;
      }
      return RECORD_NOT_RECORD;
    }
#if DISABLE_FOR_NOW
    /**
     * Marks a record in a PEB as deleted
     */
    static bool peb_record_delete(peb_number_type peb, record_header_type& record_header) {
      
    }
#endif
    /**
     * @return the offset of a record header in a sector
     */
    static uint32 record_get_offset_in_leb(const record_header_type* address) {
      const uint32 offset = (uint32)address & (SEC_SIZE - 1);
      return offset;
    }
  };
  
  template <typename T>
  typename RecordStore<T>::peb_info_type RecordStore<T>::m_peb_info[NB_PEB];
  
  template <typename T> 
  typename RecordStore<T>::leb_info_type RecordStore<T>::m_leb_info[NB_LEB];
  
  template <typename T> 
  typename RecordStore<T>::peb_number_type  RecordStore<T>::m_peb_spare;
}
