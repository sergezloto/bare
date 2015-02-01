/*
 *  media.h
 *  Embedded
 *
 *  Created by Serge on 19/02/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "nor_types.h"
#include "peb_header.h"

#define INTEGRATE_SMALL_DATA_IN_SLOT 1

namespace lpcnor {
  
  /**
   * Our default setup, as an example traits type
   * for the media_type class
   */
  struct media_sample_traits {
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
   * The media_type manages a set of nor erase blocks called physical erase blocks (PEB).
   * Each PEB has a header with a logical erase block (LEB).
   * It divides an erase block into pages, and writes new data at page boundaries.
   */
  template <class T>
  class media_type: NoCopy {
  private:
    /**
     * Slots represent occupied portions on flash.
     * They can be in 3 states:
     * - blank, never written
     * - valid, written once
     * - invalid, because it was deleted or incorrectly written or overwritten
     */
    struct slot_type {
      static const data_size_type DATA_SIZE = 8;
      typedef uint8 data_type[DATA_SIZE];
      
      /**
       * Slot created as allocated and pointing to a page
       */
      slot_type(page_number_type pg, data_size_type size, util::CRC32::result_type data_crc32)
      : m_page(pg), m_size(size), 
      m_reserved(~0u),
      m_data_crc32(data_crc32),
      m_crc32(calc_crc32()) {}
      
#if INTEGRATE_SMALL_DATA_IN_SLOT
      /**
       * Constructor for a slot whose data fits within the slot itself
       */
      slot_type(const uint8* data, data_size_type size)
      : m_page(0xffff), m_size(size), 
      m_data_as_64(*reinterpret_cast<const uint64*>(data)),  // Ugly cast. Efficient copy
      m_crc32(calc_crc32()) {}
      
      /**
       * @return the address of the slot data area
       */
      const uint8* get_data_address() const { return m_data; }
#endif
#if FORCE_VACANT_SLOT_SWRITE
      /**
       * Blank slot
       */
      static const slot_type* make_blank() {
        // slot with page at zero!
        static const uint8 d[PEB_WRITE_LINE_SIZE] = {
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        };
        return reinterpret_cast<const slot_type*>(d);
      }
#endif
      /**
       * deleted slot
       */
      static const slot_type* make_deleted() {
        // slot with page at zero!
        static const uint8 d[PEB_WRITE_LINE_SIZE] = {
          0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        };
        return reinterpret_cast<const slot_type*>((const void*)d);
      }
      
      bool is_blank() const {
        return m_page == 0xffff && m_size == 0xffff 
        //&& m_reserved1 == ~0u && m_reserved2 == ~0u 
        && m_crc32 == ~0u;
      }
      bool is_valid() const { return m_page != 0 && m_crc32 == calc_crc32(); }
      page_number_type get_page() const { return m_page; }
      data_size_type get_size() const { return m_size; }
    private:
      util::CRC32::result_type calc_crc32() const {
        using util::CRC32;
        const uint8* address = reinterpret_cast<const uint8*>(this);
        const uint32 sz = sizeof *this - sizeof m_crc32;
        CRC32 crc32;
        CRC32::result_type crc = crc32.process(address, sz).get_result();
        return crc;
      }        
      const page_number_type m_page;
      const data_size_type m_size;
      const union {
        data_type m_data;
        struct {
          uint32 m_reserved;
          util::CRC32::result_type m_data_crc32;
        };
        uint64 m_data_as_64;
      };
      const util::CRC32::result_type m_crc32;
    } ZOROBO_PACKED;
    ZOROBO_CHECK_SIZE(slot_type, PEB_WRITE_LINE_SIZE)
    
  public:
    // Import into the namespace
    typedef T traits_type;
    typedef lpcnor::page_number_type page_number_type;
    typedef lpcnor::data_size_type data_size_type;
    
    /**
     * An in-flash or off-flash pointer, referring  to LEBs and slot index within that LEB
     * It stays valid as long as the associated data is not deleted
     */
    struct ptr_type {
      ptr_type(): m_leb(LEB_INVALID), m_slot_index(0xff) {
      }
      ptr_type(leb_number_type leb, slot_index_type slot_index): m_leb(leb), m_slot_index(slot_index) {
      }
      void set_blank() {
        m_leb = LEB_INVALID;
        m_slot_index = 0xff;
      }
      bool is_blank() const {
        return m_leb == LEB_INVALID;
      }
      void set_zero() {
        m_leb = 0;
        m_slot_index = 0;
      }
      bool is_zero() const {
        return m_leb == 0 && m_slot_index == 0;
      }
      void set(leb_number_type leb, slot_index_type slot_index) {
        m_leb = leb; slot_index = slot_index;
      }
      leb_number_type m_leb;
      slot_index_type m_slot_index;
    } ZOROBO_PACKED;
    ZOROBO_CHECK_SIZE(ptr_type, 3)
    
    /**
     * Whether to start from last saved state or from scratch
     */
    enum init_type {
      INIT_NORMAL,
      INIT_ERASE
    };
        
    media_type() {
    }
    
    /**
     * Sets up the norfs area.
     * If needed, performs the initial format
     */
    static bool configure(init_type init = INIT_NORMAL);
    
    /**
     * @return the pointer to the first slot
     */
    static ptr_type ptr_first();
    
    /**
     * @return the next pointer. If the given pointer is not valid, start with first slot
     */
    static ptr_type ptr_next(const ptr_type& ptr);
    
    /**
     * @return the size of a valid slot, in bytes
     */
    static data_size_type ptr_size(const ptr_type& ptr) {
      // Get the ptr slot
      const slot_type* slot = slot_address(ptr);
      return slot->get_size();
    }
    
    /**
     * @return the size of a valid slot, in pages
     */
    static page_number_type ptr_pages(const ptr_type& ptr) {
      const data_size_type size = ptr_size(ptr);
#if INTEGRATE_SMALL_DATA_IN_SLOT
      if (size <= slot_type::DATA_SIZE) {
        return 0;
      }
#endif
      return size_to_pages(size);
    }
    
    /**
     * @return an in-flash address of data pointed to by a valid media pointer
     * The address remains valid until the next write operation
     */
    static const void* ptr_address(const ptr_type& ptr) {
      const leb_info_type& li = leb_info(ptr.m_leb);
      const slot_type* d = slot_address(ptr);
#if INTEGRATE_SMALL_DATA_IN_SLOT
      if (d->get_size() <= slot_type::DATA_SIZE) {
        return d->get_data_address();
      }
#endif
      return pg_address(li.peb, d->get_page());
    }
    
#if DEBUG
    static page_number_type ptr_page(const ptr_type& ptr) {
      const slot_type* d = slot_address(ptr);
      return d->get_page();
    }
#endif
    
    /**
     * Creates a slot and writes the given data.
     * The operation is atomic
     * @return a media pointer to the data. The pointer won't be valid if the creation failed
     */
    static ptr_type data_write(const void* data, data_size_type data_size);
    
    /**
     * A temporary pointer, used for uncommitted data. It keeps track of the offset
     * to write to, and will not program the flash beyond its slot size.
     * It performs buffering so as to write whole write line, or a partial final one.
     * It is not an on-flash structure due to its temporary nature.
     */
    class temp_ptr_type {
    public:
      bool is_blank() const { return m_pos == 0xffff; }
      void invalidate() { m_pos = 0xffff; }
#if DEBUG
      leb_number_type get_leb() const { return m_leb; }
      page_number_type get_page() const { return m_page; }
      data_size_type get_pos() const { return m_pos; }
#endif
    private:
      temp_ptr_type(leb_number_type leb, page_number_type page, data_size_type size)
      : m_leb(leb), m_page(page), m_size(size), m_pos(0) {
      }
      temp_ptr_type(): m_leb(0), m_page(0), m_size(0), m_pos(0xffff) {}
      
      const leb_number_type m_leb;
      const page_number_type m_page;
      const data_size_type m_size;
      data_size_type m_pos;
      uint8 m_buffer[PEB_WRITE_LINE_SIZE];
      util::CRC32 m_crc32_calc;
      friend class media_type;
    };
    
    /**
     * Creates a slot in a non-committed state, reserving enough space to hold
     * the given data size
     * @return a media pointer to the uncommitted slot
     */
    static temp_ptr_type data_new(data_size_type data_size);
    
    /**
     * Appends data to an uncommitted slot.
     * @return true if the operation is successful
     */
    static bool data_write(temp_ptr_type& temp_ptr, const void* data, data_size_type size);
    
    /**
     * Commits the slot, obtaining a valid regular pointer if commit was successfull
     */
    static ptr_type data_commit(temp_ptr_type& temp_ptr);
    
    /**
     * Removes a slot, atomically
     * @return true if the update was successfull
     */
    static bool data_delete(const ptr_type& ptr);
    
    /**
     * Checks the integrity of the data
     * @return true if the data is correct
     */
    static bool data_check(const ptr_type& ptr);
    
#if DEBUG
    typedef typename T::return_code return_code;
    /**
     * Debugging help
     */
    static return_code get_last_code() {
      return T::get_last_code();
    }
#endif
    
  private:
    /**
     * Parameters, modeled after the template
     */
    enum {
      PEB_NB = T::PEB_NB,
      LEB_NB = T::PEB_NB - 1,
      PEB_SIZE = T::PEB_SIZE,
      PEB_PAGE_SIZE = PEB_WRITE_LINE_SIZE,
      PEB_PAGE_COUNT = PEB_SIZE / PEB_PAGE_SIZE
    };
    
    static const uint8* peb_address(peb_number_type peb) {
      return T::peb_address(peb);
    }
    static const uint8* pg_address(peb_number_type peb, page_number_type page) {
      const uint8* address = peb_address(peb) + page * PEB_PAGE_SIZE;
      return address;
    }
    static bool peb_erase(peb_number_type peb) {
      return T::peb_erase(peb);
    }
    static bool peb_program(peb_number_type peb, uint32 offset, const void* data, uint32 data_size) {
      return T::peb_program(peb, offset, data, data_size);
    }
    static bool peb_is_blank(peb_number_type peb) {
      return T::peb_is_blank(peb);
    }
    
  private:    
    /**
     * The array of slots begins right after the PEB header
     */
    static const uint32 PEB_SLOT_ARRAY_OFFSET = sizeof(peb_header_type);
    
    /**
     * @return the offset of a slot in a PEB (or LEB) given its index
     */
    static uint32 slot_offset(slot_index_type slot_index) {
      const uint32 offset = PEB_SLOT_ARRAY_OFFSET + slot_index * sizeof(slot_type);
      return offset;
    }
    
    /**
     * @return a pointer to an on-flash slot
     */
    static const slot_type* slot_address(const ptr_type& ptr) {
      return slot_address(ptr.m_leb, ptr.m_slot_index);
    }
    
    /**
     * @return a reference to an on-flash slot
     */
    static const slot_type* slot_address(leb_number_type leb, slot_index_type slot_index) {
      const peb_number_type peb = leb_info(leb).peb;
      const uint8* peb_addr = peb_address(peb);
      const slot_type* ps = reinterpret_cast<const slot_type*>(peb_addr + PEB_SLOT_ARRAY_OFFSET);
      return &ps[slot_index];
    }
    
    /**
     * Write/overlay a PEB slot at the given index
     */
    static bool peb_slot_program(peb_number_type peb,
                                 slot_index_type slot_index,
                                 const slot_type& slot) {
      const uint32 offset = slot_offset(slot_index);
      return peb_program(peb, offset, &slot, sizeof slot);
    }
    
    /**
     * Write/overlay a LEB slot at the given index
     */
    static bool leb_slot_program(leb_number_type leb,
                                 slot_index_type slot_index,
                                 const slot_type& slot) {
      const peb_number_type peb = leb_info(leb).peb;
      return peb_slot_program(peb, slot_index, slot);
    }
    
    /**
     * We keep track of lebs using this table
     */
    struct leb_info_type {
      void reset() {
        peb = PEB_INVALID;
        lowest_busy_page = PEB_PAGE_COUNT;
        nb_busy_pages = 0;
        nb_slots = 0;
      }
      bool valid() const {
        return peb != PEB_INVALID;
      }
      page_number_type reclaimable_pages() const {
        return PEB_PAGE_COUNT - lowest_busy_page - nb_busy_pages;
      }
      peb_number_type peb;
      page_number_type lowest_busy_page;  // Lowest occupied page, or 0 if none
      page_number_type nb_busy_pages;  // used to see how much garbage will be collected during switch
      page_number_type nb_slots;
    };
    
    /**
     * @return true if the given page is blank
     */
    static bool pg_is_blank(peb_number_type peb, page_number_type page) {
      const uint32* pg = (const uint32*)pg_address(peb, page);
      for (uint32 i = 0; i < PEB_PAGE_SIZE / 4; ++i) {
        if (pg[i] != 0xffffffff) {
          return false;
        }
      }
      return true;
    }
    
    /**
     * Rounds the given size to the upper number of pages
     * @return the number of pages that will contain this many bytes
     */
    static page_number_type size_to_pages(uint32 size) {
      return (size - 1) / PEB_PAGE_SIZE + 1;
    }
    
    /*
     * @return a reference to the in-flash PEB header of the given PEB
     */
    static const peb_header_type& peb_header(peb_number_type peb) {
      return *reinterpret_cast<const peb_header_type*>(peb_address(peb));
    }
    
    /**
     * Writes a PEB header. Interrupts must be disabled during the call.
     */
    static bool peb_write_header(peb_number_type peb, const peb_header_type& header) {
      // We are writing right at sector start
      const bool ok = peb_program(peb, 0, &header, sizeof header);
      return ok;
    }    
  
    /**
     * Writes a leb header to a peb. Assumes the peb is blank.
     * Interrupts must be disabled during the call.
     */
    static bool peb_write_leb_header(peb_number_type peb, leb_number_type leb, 
                                     uint32 leb_generation, 
                                     slot_index_type transferred_slots,
                                     uint32 erase_count) {
      peb_header_type ph;
      ph.reset();  // Init with all 0xff
      ph.peb_magic = PEB_MAGIC;
      ph.peb_leb = leb;
      ph.peb_leb_generation = leb_generation;
      ph.peb_transferred_slots = transferred_slots;
      ph.peb_erase_count = erase_count;
      ph.peb_header_crc32 = ph.calc_crc32();
      
      return peb_write_header(peb, ph);
    }
        
    /**
     * @return a reference to LEB information
     */
    static leb_info_type& leb_info(leb_number_type leb) {
      return m_leb_info[leb];
    }
    
    /**
     * Initial LEB check
     * Builds the peb_info and leb_info tables
     * Also locates the root node, by scanning all 16-bytes boundaries.
     * The root node with the highest generation is retained.
     * The LEB array must be correct and valid.
     * @return false if something is wrong. In this case, the erase_count field is 
     *               set and suitable to format the lebs
     */
    static bool init_check_lebs(uint32& erase_count);
    
    /**
     * Initial LEB format
     * @param leb_erase_count the LEBs initial erase count
     * @return false if something went wrong
     */
    static bool init_format_lebs(uint32 leb_erase_count);
    
    /**
     * Chooses a LEB to store data, possibly trigerring a 
     * switch operation
     * @return true if enough space could be gathered
     */
    static bool leb_choose(uint32 data_size, leb_number_type& leb);
    
    /**
     * Moves a LEB to the spare. All active slots located on the LEB
     * are copied and compacted in the process.
     */
    static bool leb_switch(leb_number_type leb);
    
    static leb_info_type m_leb_info[LEB_NB];
    static peb_number_type m_spare_peb;
  };
  
  template <class T>
  typename media_type<T>::leb_info_type media_type<T>::m_leb_info[LEB_NB];
  
  template <class T>
  peb_number_type media_type<T>::m_spare_peb;

  template <class T>
  bool media_type<T>::configure(init_type init) {
    // Sanity LEB check
    uint32 erase_count;
    bool ok = init_check_lebs(erase_count);
    
    if (init == INIT_ERASE || !ok) {
      // Not good, reformat the whole thing
      // Note: this precludes extending the media_type area!
      ok = init_format_lebs(erase_count);    
    }
    // Now the LEBs are set up, and the root node is also set up
    
    return ok;
  }

  template <class T>
  bool media_type<T>::init_check_lebs(uint32& erase_count) {
    bool ok = false;
    uint32 sum_erase_count = 0;
    uint32 nb_erase_count = 0;
    
    // Reset LEBs
    for (leb_number_type leb = 0; leb < LEB_NB; ++leb) {
      leb_info(leb).reset();
    }
    // No spare yet
    m_spare_peb = PEB_INVALID;
    
    // Scan all PEBs looking for LEBs
    for (peb_number_type peb = 0; peb < PEB_NB; ++peb) {
      if (peb_is_blank(peb)) {
        // If there is another spare peb, then we have corruption
        if (m_spare_peb != PEB_INVALID) {
          ok = false;
          goto done;
        }
        // Remember it
        m_spare_peb = peb;
        // No need to analyze it further
        continue;
      }
      
      const peb_header_type& ph = peb_header(peb);
      
      // Check PEB header well-formedness
      ok = ph.check();
      if (!ok) {
        // problem with this PEB. Try to use it as spare if none is registered
        if (m_spare_peb != PEB_INVALID) {
          ok = false;
          goto done;
        }
        // Remember it
        m_spare_peb = peb;
        // No need to analyze it further
        continue;
      }
      
      //
      // From here on, the PEB is well formed
      //
      const leb_number_type leb = ph.peb_leb;

      // Check for LEB numbers over the configured limit
      if (leb >= LEB_NB) {
        // We have a problem, there are more LEBs than configured in here!
        // Bail out!
        ok = false;
        goto done;
      }
      
      // Add to our erase count
      sum_erase_count += ph.peb_erase_count;
      ++nb_erase_count;
      
      //
      // Fill in LEB info for this PEB
      //
      leb_info_type& li = leb_info(leb);
      
      // Check for duplicate LEB
      if (li.valid()) {
        // Duplicate. Retain the one with highest generation. The older one is a spare
        // But if there already is a spare accounted for, then two spares are one too many
        if (m_spare_peb != PEB_INVALID) {
          // Bail out, we're corrupted
          ok = false;
          goto done;
        }
        // This same LEB was encountered in another PEB. Which one to retain?
        peb_number_type prev_peb = li.peb;
        const peb_header_type& prev_ph = peb_header(prev_peb);
        if (ph.is_leb_more_recent(prev_ph)) {
          // More recent. Adopt it
          li.peb = peb;
          // the previous one serves as spare
          m_spare_peb = prev_peb;
        } else {
          // The previous PEB was more recent. Make this one a spare
          m_spare_peb = peb;
        }
      } else {
        // Assign PEB to LEB
        li.peb = peb;
      }      
    }
    
    //
    // In every LEB, scan slots and measure total pages and lowest busy page
    //
    for (leb_number_type leb = 0; leb < LEB_NB; ++leb) {
      leb_info_type& li = leb_info(leb);
      const peb_header_type& ph = peb_header(li.peb);
      const slot_index_type transferred_slots = ph.peb_transferred_slots;
      // The number of slots is at least equal to the one recorded in the PEB header
      // After which new slots may be valid or invalid, but never blank
      for (slot_index_type slot_index = 0;;++slot_index) {
        const slot_type* slot = slot_address(leb, slot_index);
        if (slot_index >= transferred_slots && slot->is_blank()) {
          // That was the last slot, we are done with this LEB
          // Adjust the number of slots for this leb
          li.nb_slots = slot_index;

          //
          // Scan from the lowest busy page downwards, in case data was written but not committed.
          // We adjust lowest_busy_page accordingly
          // Also, we subtract the total # of pages known busy from what is calculated with lowest_busy_page. This
          // Gives us the amount of garbage in the LEB
          //
          for (page_number_type pg = li.lowest_busy_page - 1; pg > li.nb_slots; --pg) {
            if (!pg_is_blank(li.peb, pg)) {
              li.lowest_busy_page = pg;
            }
          }

          // Done with this LEB
          break;
        }
        if (slot->is_valid()) {
          const data_size_type size = slot->get_size();
#if INTEGRATE_SMALL_DATA_IN_SLOT
          if (size <= slot_type::DATA_SIZE) {
            // Busy pages do not change, no accounting necessary
          } else {            
#endif
#if INTEGRATE_SMALL_DATA_IN_SLOT
            // Account for the busy pages
            li.nb_busy_pages += size_to_pages(size);     
            if (li.lowest_busy_page == 0 || slot->get_page() < li.lowest_busy_page) {
              li.lowest_busy_page = slot->get_page(); 
            }
          }
#endif
        } else {
          // The slot is garbage, or corrupted.
          // In any case, we can't read its size reliably.
          // Just skip it.
        }
      }
    }
    
    
  done:
    if (nb_erase_count > 0) {
      erase_count = sum_erase_count / nb_erase_count;
    } else {
      // No info on erase counts at all. Space should be blank, hence set erase count to 0
      erase_count = 0;
    }
    return ok;
  }
  
  
  template <class T>
  bool media_type<T>::init_format_lebs(uint32 erase_count) {
    const uint32 initial_generation = 0;
    const slot_index_type nb_slots = 0;
    bool ok;
    peb_number_type peb = 0;
    for (leb_number_type leb = 0; leb < LEB_NB; ++leb, ++peb) {
      leb_info_type& li = leb_info(leb);
      li.reset();
      
      // Erase the PEB if needed
      const bool is_blank = peb_is_blank(peb);
      if (!is_blank) {
        ok = peb_erase(peb);
        if (!ok) {
          goto done;
        }
      }
      // Now blank. Write PEB header
      ok = peb_write_leb_header(peb, leb, 
                                initial_generation, nb_slots, erase_count);
      if (!ok) {
        goto done;
      }
      // Fill info on brand new leb
      li.peb = peb;
    }
    // At this point, the last PEB is empty. Just erase it
    // It will be chosen as our spare later           
    if (!peb_is_blank(peb)) {
      ok = peb_erase(peb);
      m_spare_peb = peb;
    }
    
  done:
    return ok;
  }
  
  template <class T>
  bool media_type<T>::leb_switch(leb_number_type leb) {
    // Remember spare erase count if not blank. Otherwise, average other peb erase counts
    uint32 spare_erase_count;
    {
      const peb_header_type& sph = peb_header(m_spare_peb);
      if (sph.check()) {
        // We reuse the erase count
        spare_erase_count = sph.peb_erase_count;
      } else {
        // We can't rely on a valid erase count. Average others
        uint32 sum_erase_count = 0;
        for (leb_number_type l = 0; l < LEB_NB; ++l) {
          const peb_header_type& ph = peb_header(leb_info(l).peb);
          sum_erase_count += ph.peb_erase_count;
        }
        spare_erase_count = sum_erase_count / LEB_NB;
      }
      
      //
      // Blank spare PEB if not done
      //
      if (!peb_is_blank(m_spare_peb)) {
        const bool ok = peb_erase(m_spare_peb);
        if (!ok) {
          return false;
        }
      }
    }
    
    //
    // Transfer active slots, storing them in decreasing pages
    // REMEMBER, WE HAVE TO WRITE SLOTS (NOT DATA) AT THEIR ORIGINAL INDEX
    //
    leb_info_type& li = leb_info(leb);
    const peb_header_type& ph = peb_header(li.peb);
    page_number_type dest_page = PEB_PAGE_COUNT;
    page_number_type nb_busy_pages = 0;
#if FORCE_VACANT_SLOT_SWRITE
    const slot_type& vacant_slot = *slot_type::make_blank();
#endif
    slot_index_type slot_index = 0;
    for (; slot_index < li.nb_slots; ++slot_index) {
      const slot_type* slot = slot_address(leb, slot_index);
      if (!slot->is_valid() || slot->is_blank()) {
        // We have to transfer the slot as vacant only, no data
#if FORCE_VACANT_SLOT_SWRITE
        const bool ok = peb_slot_program(m_spare_peb, slot_index, vacant_slot);
        if (!ok) {
          return false;
        }
#endif
      } else {
        //
        // Slot is valid and not vacant. Transfer slot and data
        //
        const data_size_type size = slot->get_size();
#if INTEGRATE_SMALL_DATA_IN_SLOT
        if (size <= slot_type::DATA_SIZE) {
          //
          // The data will be within the slot itself
          // Write slot in the committed state straight away
          //
          slot_type new_slot(slot->get_data_address(), size);
          const bool ok = peb_slot_program(m_spare_peb, slot_index, new_slot);
          if (!ok) {
            return false;
          }
        } else {
          // Write the data separately from the slot
#endif
          const page_number_type pages = size_to_pages(size);
          
          //
          // Write data
          //
          const void* data = pg_address(li.peb, slot->get_page());
          dest_page -= pages;
          bool ok = peb_program(m_spare_peb, dest_page * PEB_PAGE_SIZE,
                                data, pages * PEB_PAGE_SIZE);
          if (!ok) {
            return false;
          }
          nb_busy_pages += pages;

          //
          // Write slot in the committed state straight away
          //
          slot_type new_slot(dest_page, size, slot->m_data_crc32);
          ok = peb_slot_program(m_spare_peb, slot_index, new_slot);
          if (!ok) {
            return false;
          }
#if INTEGRATE_SMALL_DATA_IN_SLOT
        }
#endif       
      }        
    }
    
    // If ok, write new PEB header with higher LEB gen and old erase count + 1
    // and lowest_transferred page, which is simply the last destination page
    const uint32 new_generation = ph.next_leb_generation();
    const bool ok = peb_write_leb_header(m_spare_peb, leb, new_generation,
                                         slot_index, spare_erase_count + 1);
    if (!ok) {
      return false;
    }
    // Mark old PEB as spare, but do not erase it to preserve erase count
    const peb_number_type new_peb = m_spare_peb;
    m_spare_peb = li.peb;
    li.peb = new_peb;
    li.lowest_busy_page = dest_page;
    li.nb_busy_pages = nb_busy_pages;
    li.nb_slots = slot_index;

    return true;
  }
  
  template <class T>
  bool media_type<T>::leb_choose(uint32 data_size, 
                            leb_number_type& chosen_leb) {
    // Figure out how many pages are needed for the storage of this slot
    const page_number_type pages_needed = size_to_pages(data_size);
    
    // Scan all LEBS, retaining the one with enough space and the lowest
    // erase count
    // Just in case, also remember the one with the highest amount of garbage
    chosen_leb = LEB_INVALID;
    uint32 lowest_erase_count = ~0u;
    uint32 garbage_lowest_erase_count = ~0u;
    leb_number_type collect_leb = LEB_INVALID;
    for (leb_number_type leb = 0; leb < LEB_NB; ++leb) {
      // How many pages are free in the LEB?
      const leb_info_type& li = leb_info(leb);
      // Space taken by header and slots, plus one (conservatively) 
      // for the new slot if needed
      const page_number_type pages_for_header_and_slots = size_to_pages(PEB_SLOT_ARRAY_OFFSET + (li.nb_slots + 1) * sizeof(slot_type));
      const page_number_type free_pages = (li.lowest_busy_page != 0 ? li.lowest_busy_page : PEB_PAGE_COUNT) - pages_for_header_and_slots;
      // We need space for one blank slot as a slot end marker. Make sure there's enough
      // by requiring at least one extra page, hence the strict comparison below
      const peb_header_type& ph = peb_header(li.peb);
      const uint32 peb_erase_count = ph.peb_erase_count;
      if (pages_needed < free_pages) {
        // If low erase count, then it's a candidate
        if (peb_erase_count < lowest_erase_count) {
          lowest_erase_count = peb_erase_count;
          chosen_leb = leb;
        }
      } else if (chosen_leb == LEB_INVALID) {
        // We haven't yet found a LEB with enough immediate free space.
        // Not enough space in this LEB either. However, check if it is suitable for
        // garbage collection in case no LEB has enough free space
        // It is the one where the recoverable space is big enough and the erase count is lowest
        const page_number_type collect_pages = li.reclaimable_pages();
        if (pages_needed < free_pages + collect_pages) {
          // Would fit after garbage collection
          if (peb_erase_count <= garbage_lowest_erase_count) {
            // Best candidate so far
            collect_leb = leb;
            garbage_lowest_erase_count = peb_erase_count;
          }
        }
      }      
    }
    
    if (chosen_leb != LEB_INVALID) {
      // We made a choice
      return true;
    }
    
    // Did not find any. Collect the lowest erase count LEB with enough recoverable space
    if (collect_leb != LEB_INVALID) {
      // This LEB would do!
      if (leb_switch(collect_leb)) {
        chosen_leb = collect_leb;
        return true;
      }
    }
    return false;
  }

  template <class T>
  typename media_type<T>::temp_ptr_type media_type<T>::data_new(data_size_type data_size) {
    temp_ptr_type ptr_invalid;
    
    // Choose LEB to write to
    leb_number_type leb;
    bool ok = leb_choose(data_size, leb);
    if (!ok) {
      return ptr_invalid;
    }
    
    //
    // We'll need to write the data as follows
    //
#if INTEGRATE_SMALL_DATA_IN_SLOT
    if (data_size <= slot_type::DATA_SIZE) {
      // No need to adjust anything in the LEB info
      const temp_ptr_type temp_ptr(leb, 1, data_size);
      return temp_ptr;
    }
#endif
    const page_number_type pages_needed = size_to_pages(data_size);
    leb_info_type& li = leb_info(leb);
    const page_number_type page = (li.lowest_busy_page == 0 ? PEB_PAGE_COUNT : li.lowest_busy_page) - pages_needed;
    
    //
    // We'll write at that page, surely. Adjust lowest_busy_page
    //
    li.lowest_busy_page = page;
    
    const temp_ptr_type ptr(leb, page, data_size);
    return ptr;
  }

  template <class T>
  bool media_type<T>::data_write(temp_ptr_type& temp_ptr, const void* data, data_size_type size) {
    const data_size_type original_data_size = size;
    
    //
    // Ensure there is space to write the data
    //
    if (temp_ptr.m_pos + size > temp_ptr.m_size) {
      // Too big!
      return false;
    }
    
    const uint8* bytes = reinterpret_cast<const uint8*>(data);
    leb_info_type& li = leb_info(temp_ptr.m_leb);
    const peb_number_type peb = li.peb;
    const uint32 offset = temp_ptr.m_page * PEB_PAGE_SIZE;
    
    //
    // Complete and write any previously buffered data
    //
    data_size_type bytes_buffered = temp_ptr.m_pos % PEB_WRITE_LINE_SIZE;
    if (bytes_buffered > 0) {
      // Add to the buffer until full and program
      while (size > 0 && bytes_buffered != PEB_WRITE_LINE_SIZE) {
        temp_ptr.m_buffer[bytes_buffered] = *bytes;
        ++bytes;
        --size;
        ++bytes_buffered;
        ++temp_ptr.m_pos;
      }
      // Write buffer if we have a complete write line
      if (bytes_buffered == PEB_WRITE_LINE_SIZE)
        if (!peb_program(peb, offset + temp_ptr.m_pos - bytes_buffered, 
                         temp_ptr.m_buffer, sizeof temp_ptr.m_buffer)) {
          return false;
      }
    }
        
    //
    // Write rest of data in PEB_WRITE_LINE_SIZE chunks
    //
    const data_size_type remainder_size = size % PEB_WRITE_LINE_SIZE;
    if (size >= PEB_WRITE_LINE_SIZE) {
      // We can write full lines!
      const data_size_type chunked_size = size - remainder_size;
      const bool ok = peb_program(peb, temp_ptr.m_pos + offset, bytes, chunked_size);
      if (!ok) {
        // Check whether the area is still blank
        // Leave slot uncommitted and bail out
        return false;
      }
      bytes += chunked_size;
      size -= chunked_size;
      temp_ptr.m_pos += chunked_size;
    }
    
    //
    // Buffer up remaining incomplete chunk
    // The last remainder, if any, will be programmed at commit time
    //
    if (remainder_size > 0) {
      // Buffer it 
      for (data_size_type i = 0; i < remainder_size; ++i) {
        temp_ptr.m_buffer[i] = bytes[i];
      }
      temp_ptr.m_pos += remainder_size;
    }
    temp_ptr.m_crc32_calc.process(reinterpret_cast<const uint8*>(data), original_data_size);
    return true;
  }
  
  template <class T>
  typename media_type<T>::ptr_type media_type<T>::data_commit(temp_ptr_type& temp_ptr) {
    const leb_number_type leb = temp_ptr.m_leb;
    leb_info_type& li = leb_info(leb);
    
    const ptr_type ptr_invalid;

    //
    // Fill unused space with 0xff - less strain on flash, hopefully
    //
    const data_size_type remainder = temp_ptr.m_pos % PEB_WRITE_LINE_SIZE;
    if (remainder > 0 ) {
      memset(&temp_ptr.m_buffer[remainder], 0xff, PEB_WRITE_LINE_SIZE - remainder);      
    }
    
#if INTEGRATE_SMALL_DATA_IN_SLOT
    if (temp_ptr.m_size > slot_type::DATA_SIZE) {
      // The data is too large for being within the slot
#endif
      //
      // Write buffer if any
      //
      if (remainder > 0) {
        
        // And write it
        const data_size_type offset = temp_ptr.m_page * PEB_PAGE_SIZE + temp_ptr.m_pos - remainder;
        if (!peb_program(li.peb, offset, temp_ptr.m_buffer, PEB_WRITE_LINE_SIZE)) {
          return ptr_invalid;
        }
      }
#if INTEGRATE_SMALL_DATA_IN_SLOT
    }
#endif
    
    //
    // Find a suitable slot in the chosen LEB
    //
    slot_index_type slot_index = 0;
    for (; slot_index < li.nb_slots; ++slot_index) {
      const slot_type* slot = slot_address(leb, slot_index);
      if (slot->is_blank()) {
        // Found an empty one. Use it
        break;
      }
    }    
    
    // We attempt to write. Do not reuse this slot, even if the write fails
    // If we used a vacant slot, no need to increase the number of slots
    if (slot_index == li.nb_slots) {
      // We scanned all slots without finding a vacant one. Allocate a new one then!
      ++li.nb_slots;
    }

    // Commit. We write the slot at once
#if INTEGRATE_SMALL_DATA_IN_SLOT
    if (temp_ptr.m_size <= slot_type::DATA_SIZE) {
      // The data will be within the slot
      typename slot_type::data_type data;
      memcpy(data, temp_ptr.m_buffer, temp_ptr.m_size);
      memset(&data[temp_ptr.m_size], 0xff, slot_type::DATA_SIZE - temp_ptr.m_size);
      slot_type new_slot(data, temp_ptr.m_size);
      const bool ok = leb_slot_program(leb, slot_index, new_slot);
      if (!ok) {
        // Slot won't check anyway
        // The lowest busy page does not change since no data was written
        return ptr_invalid;
      }
      // LEB nb_busy_pages stays unchanged since no data space was consumed
    } else {
      // Data in data space
#endif
      const util::CRC32::result_type data_crc32 = temp_ptr.m_crc32_calc.get_result();
      slot_type new_slot(temp_ptr.m_page, temp_ptr.m_size, data_crc32);
      const bool ok = leb_slot_program(leb, slot_index, new_slot);
      if (!ok) {
        // Slot won't check anyway
        // The lowest busy page does not change since no data was written
        return ptr_invalid;
      }
      // It worked. The new data pages are now valid, and must be accounted for
      li.nb_busy_pages += size_to_pages(temp_ptr.m_size);
#if INTEGRATE_SMALL_DATA_IN_SLOT
    }
#endif
    return ptr_type(leb, slot_index);
  }
  
  template <class T>
  typename media_type<T>::ptr_type media_type<T>::data_write(const void* data, data_size_type data_size) {
    const ptr_type blank_pointer;
    temp_ptr_type tp = data_new(data_size);
    if (tp.is_blank()) {
      return blank_pointer;
    }
    if (!data_write(tp, data, data_size)) {
      return blank_pointer;
    }
    const ptr_type p = data_commit(tp);
    return p;
  }
  
  template <class T>
  bool media_type<T>::data_delete(const ptr_type& ptr) {
    // Delete the slot if not already deleted
    const slot_type* slot = slot_address(ptr);
    if (!slot->is_valid()) {
      return false;
    }
    // Remember how many pages are about to be deleted.
    // You can't get this info once the slot is invalidated!
    const page_number_type deleted_pages = ptr_pages(ptr);

    // A single extra write in the slot with page 0, size 0 will invalidate it
    // Because ECC is sure to screw up an extra write to a write line
    const slot_type& deleted_slot = *slot_type::make_deleted();

    // Invalidate the slot. We don't check for errors
    // since generally, the write will fail to verify (other bits will flip
    // in the write line due to ECC)
    leb_slot_program(ptr.m_leb, ptr.m_slot_index, deleted_slot);

    //
    // Update the LEB deleted page count for garbage collection purposes
    //
    leb_info_type& li = leb_info(ptr.m_leb);
    li.nb_busy_pages -= deleted_pages;
    
    return true;
  }
  template <class T>
  bool media_type<T>::data_check(const ptr_type& ptr) {
    const slot_type* slot = slot_address(ptr);
    // If the slot itself is corrupted or otherwise invalid,
    // no point looking further
    if (!slot->is_valid()) {
      return false;
    }
    const data_size_type size = slot->get_size();
#if INTEGRATE_SMALL_DATA_IN_SLOT
    if (size <= slot_type::DATA_SIZE) {
      // The data, within a valid slot, has already been checked
      return true;
    }
#endif
    const leb_info_type& li = leb_info(ptr.m_leb);
    const peb_number_type peb = li.peb;
    const uint8* const address = pg_address(peb, slot->get_page());
    util::CRC32 crc32_calc;
    crc32_calc.process(address, size);
    const util::CRC32::result_type crc32 = crc32_calc.get_result();
    if (crc32 == slot->m_data_crc32) {
      return true;
    }
    return false;
  }

  
  template <class T>
  typename media_type<T>::ptr_type media_type<T>::ptr_first() {
    ptr_type ptr_invalid;
    return ptr_next(ptr_invalid);
  }

  template <class T>
  typename media_type<T>::ptr_type media_type<T>::ptr_next(const ptr_type& ptr) {
    const leb_number_type leb_start = ptr.is_blank() ?  0 : ptr.m_leb;
    slot_index_type i = ptr.is_blank() ? 0 : ptr.m_slot_index + 1;
    
    // Start with the pointer leb and slot index
    for (leb_number_type leb = leb_start; leb < LEB_NB; ++leb) {
      const leb_info_type& li = leb_info(leb);
      for (;i < li.nb_slots;++i) {
        const slot_type* slot = slot_address(leb, i);
        if (slot->is_valid()) {
          // we got it
          ptr_type ptr_found(leb, i);
          return ptr_found;
        }
      }
      // Start at first slot in next LEB
      i = 0;
    }
    // We scanned all lebs without success
    ptr_type ptr_invalid;
    return ptr_invalid;
  }
  
}
