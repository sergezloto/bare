/*
 *  HAL_Flash.h
 *  Embedded
 *
 *  Created by Serge on 4/02/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */
#pragma once

#include "HAL_IAP.h"
#include "util.h"

namespace hal {
  /**
   * A class that abstracts LPC2000 flash.
   * It supports the write and erase function
   */
  class Flash: NoInstance {
  public:
    enum blank_check_status_type {
      BC_BLANK = IAP::CMD_SUCCESS,
      BC_NOT_BLANK = IAP::SECTOR_NOT_BLANK,
      BC_ERROR
    };
    
    static const uint32 WRITE_LINE_SIZE = 16;
    static const uint32 WRITE_LINE_SIZE_ALIGN_MASK = 0xfffffff0;  // used to snap to lower line boundary
    static const uint32 BUFFER_SIZE = 256;
    static const uint32 BUFFER_ALIGN_MASK = 0xffffff00;
    static uint8 BUFFER[BUFFER_SIZE];
    
    /**
     * Aligns an address on the lower flash write line boundary
     */
    static uint8* align_address_to_line(const uint8* address) {
      const uint32 aligned_address = reinterpret_cast<uint32>(address) & WRITE_LINE_SIZE_ALIGN_MASK;
      return reinterpret_cast<uint8*>(aligned_address);
    }
    
    /**
     * Aligns an address on the upper flash write line boundary
     */
    static uint8* align_address_to_upper_line(const uint8* address) {
      const uint32 aligned_address = reinterpret_cast<uint32>(address + WRITE_LINE_SIZE - 1) & WRITE_LINE_SIZE_ALIGN_MASK;
      return reinterpret_cast<uint8*>(aligned_address);
    }
    
    /**
     * Gets info on given sector
     */
    static bool sector_to_address_size(uint32 sector, const uint8* &address, uint32& size) {
      return IAP::sector_to_address_size(sector, address, size);
    }
    
    /**
     * Given an address, obtains a start address and offset to the sector start
     */
    static void address_to_sector_offset(const void *address, uint32& sector, uint32& offset) {
      IAP::address_to_sector_offset(address, sector, offset);
    }
    
    /**
     * Obtains the first non-blank offset and content (4 bytes) in the given sectors
     * @return false if no blank area was found, true if all given sectors are entirely blank
     */
    static blank_check_status_type blank_check_sectors(uint32 sector_start, uint32 sector_end, 
                                                       uint32 &non_blank_offset, uint32 &non_blank_content) {
      const IAP::return_code code = IAP::blank_check_sectors(sector_start, sector_end, 
                                                             non_blank_offset, non_blank_content);
      switch (code) {
        case IAP::CMD_SUCCESS:
          return BC_BLANK;
        case IAP::SECTOR_NOT_BLANK:
          return BC_NOT_BLANK;
        default:
          return BC_ERROR;
      }
    }
    
    /**
     * Version where no offset or content is needed
     */
    static blank_check_status_type blank_check_sectors(uint32 sector_start, uint32 sector_end) {
      const IAP::return_code code = IAP::blank_check_sectors(sector_start, sector_end);
      switch (code) {
        case IAP::CMD_SUCCESS:
          return BC_BLANK;
        case IAP::SECTOR_NOT_BLANK:
          return BC_NOT_BLANK;
        default:
          return BC_ERROR;
      }
    }
    
    
    /**
     * Erases a sector so it can be written to
     * Interrupts must be disabled for this to work!
     */
    static bool erase_sectors(uint32 sec_from, uint32 sec_to) {
      IAP::return_code code = IAP::prepare_sectors_for_write(sec_from, sec_to);
#if DEBUG
      m_last_code = code;
#endif
      if ( code != IAP::CMD_SUCCESS) {
        // TODO: log error
        return false;
      }
      code = IAP::erase_sectors(sec_from, sec_to);
#if DEBUG
      m_last_code = code;
#endif
      if (code != IAP::CMD_SUCCESS) {
        return false;
      }
      return true;
    }
    
    /**
     * Write a memory area to flash 
     * Interrupts must be disabled at this point
     * @param dest_address must reside in flash, and be aligned to a 16-bytes boundary
     * @param nbytes the actual size of data written will be rounded up to a multiple of 16 bytes
     */
    static bool copy_ram_to_flash(const uint8* dest_address, 
                                  const void* src_address, uint32 nbytes) {
      const uint8* uint8_src_address = static_cast<const uint8*>(src_address);
      while (nbytes > 0) {
        // Place data within a 256 bytes window, where the rest is 0xff
        blank_buffer();
        
        // This is the address we'll write to!
        const uint8* aligned_dest_address = (const uint8*)((uint32)dest_address & BUFFER_ALIGN_MASK);
        const uint32 aligned_dest_offset = dest_address - aligned_dest_address;
        
        // This many bytes
        const uint32 nbytes_this_turn = util::min(nbytes, BUFFER_SIZE - aligned_dest_offset);
        
        // Copy data
        for (uint32 i = 0; i < nbytes_this_turn; ++i) {
          BUFFER[i + aligned_dest_offset] = uint8_src_address[i];
        }
        
        // Finally prepare and write the buffer
        uint32 ignore;
        uint32 start_sector; uint32 end_sector;
        address_to_sector_offset(aligned_dest_address, start_sector, ignore);
        address_to_sector_offset(aligned_dest_address + BUFFER_SIZE - 1, end_sector, ignore);
        IAP::return_code code = IAP::prepare_sectors_for_write(start_sector, end_sector);
#if DEBUG
        m_last_code = code;
#endif
        if (code != IAP::CMD_SUCCESS) {
          return false;
        }
        
        code = IAP::copy_ram_to_flash((uint32)aligned_dest_address, (uint32)BUFFER, sizeof BUFFER);
#if DEBUG
        m_last_code = code;
#endif
        switch (code) {
          case IAP::CMD_SUCCESS:
            // All good, continue
            break;
          default:
            return false;
        }
        nbytes -= nbytes_this_turn;
        dest_address += nbytes_this_turn;
        uint8_src_address += nbytes_this_turn;
      }
      return true;
    }
    
    /**
     */
    static void blank_buffer() {
      memset(BUFFER, 0xff, sizeof BUFFER);
    }
    
#if DEBUG
  public:
    typedef IAP::return_code return_code; 
    static return_code get_last_code() {
      return m_last_code;
    }
  private:
    static return_code m_last_code;
#endif
  };
}
