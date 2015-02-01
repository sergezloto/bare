/*
 *  HAL_IAP.h
 *  Embedded
 *
 *  Created by Serge on 6/26/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once
#include "base.h"

#include "HAL_Driver.h"

namespace hal {
  /**
   * Compile-time definitions of sectors sizes and addresses
   */
  template <uint32 SECTOR>
  class SectorInfo: NoInstance {
  public:
    static const uint32 size = SectorInfo<SECTOR-1>::size;
    static const uint32 address = SectorInfo<SECTOR-1>::address + SectorInfo<SECTOR-1>::size;
  };
  
  template <>
  class SectorInfo<0>: NoInstance {
  public:
    static const uint32 size = 1 << 12;
    static const uint32 address = 0;
  };  
  template <>
  class SectorInfo<8>: NoInstance {
  public:
    static const uint32 size = 1 << 15;
    static const uint32 address = SectorInfo<7>::address + SectorInfo<7>::size;
  };
  template <>
  class SectorInfo<22>: NoInstance {
  public:
    static const uint32 size = 1 << 12;
    static const uint32 address = SectorInfo<21>::address + SectorInfo<21>::size;
  };
  
  
  /**
   * In Application Programming interface class.
   * Note: The flash is not mapped during prepare/erase/write operations. Therefore interrupts 
   * must be disabled, or interrupts vectors and hanlers must reside in ram.
   */
  class IAP: Driver, NoInstance {
  public:
    /**
     * Return codes from IAP.
     */
    enum return_code {
      CMD_SUCCESS = 0,
      INVALID_COMMAND = 1,
      SRC_ADDR_ERROR = 2,
      DST_ADDR_ERROR = 3,
      SRC_ADDR_NOT_MAPPED = 4,
      DST_ADDR_NOT_MAPPED = 5,
      COUNT_ERROR = 6,
      INVALID_SECTOR = 7,
      SECTOR_NOT_BLANK = 8,
      SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION = 9,
      COMPARE_ERROR = 10,
      BUSY = 11,
      PARAM_ERROR = 12,
      ADDR_ERROR = 13,
      ADDR_NOT_MAPPED = 14,
      CMD_LOCKED = 15,
      INVALID_CODE = 16,
      INVALID_BAUD_RATE = 17,
      INVALID_STOP_BIT = 18,
      CODE_READ_PROTECTION_ENABLED = 19
    };
    
    /**
     * High level flash function. It maps the destination address to a sector and offset,
     * then writes the flash.
     *
     * The number of bytes written will be the data type size, rounded up to 256, 512, 1024 or 4096.
     * @param dest_address an address in the flash, on a 256 byte boundary.
     */
    template <typename T>
    static return_code flash(T *dest_address, const T *src_address) {
      
      const uint32 nbytes = (sizeof(T) <= 256) ? 256 
        : (sizeof(T) <= 512) ? 512 
        : (sizeof(T) <= 1024) ? 1024 
        : (sizeof(T) <= 4096) ? 4096 
        : -1;  // This one will make compile fail below!
    
      typedef uint8 check_type_sizeof_not_bigger_than_4k[nbytes];
      
      uint32 sector, offset;
      address_to_sector_offset(dest_address, sector, offset);      
      return_code ret = prepare_sectors_for_write(sector, sector);
      if (ret != CMD_SUCCESS)
        return ret;
      
      ret = copy_ram_to_flash((uint32)dest_address, (uint32)src_address, nbytes);
      
      return ret;
    }
        
    /**
     * @return the part id
     */
    static uint32 read_part_id() {
      uint32 cmd = READ_PART_ID;
      uint32 result[2];
      iap_call(&cmd, result);
      return result[1];
    }
    
    /**
     * @returns the boot code version
     */
    static uint32 read_boot_code_version() {
      uint32 cmd = READ_BOOT_CODE_VERSION;
      uint32 result[2];
      iap_call(&cmd, result);
      return result[1];
    }
    
    /**
     * Prepares a sector for an erase or write operation
     */
    static return_code prepare_sectors_for_write(uint32 sector_start, uint32 sector_end) {
      uint32 cmd[] = {
        PREPARE_SECTOR_FOR_WRITE,
        sector_start,
        sector_end
      };
      iap_call(cmd, cmd);  // result is put back into cmd
      return static_cast<return_code>(cmd[0]);
    }
    
    /**
     * Erases a sector
     */
    static return_code erase_sectors(uint32 sector_start, uint32 sector_end) {
      uint32 cmd[] = {
        ERASE_SECTORS,
        sector_start,
        sector_end,
        SYSTEM_CLOCK_IN_KHZ
      };
      iap_call(cmd, cmd);  // result is put back into cmd
      return static_cast<return_code>(cmd[0]);
    }
    
    /**
     * Write ram to flash
     * @param dest_address, should belong to a prepared sector, and be on a page (256 bytes) boundary
     * @param src_address, should be at a word (4 bytes) boundary
     * @param nbytes: must be one of 256, 512, 1024, 4096
     */
    static return_code copy_ram_to_flash(uint32 dest_address, uint32 src_address, uint32 nbytes) {
      uint32 cmd[] = {
        COPY_RAM_TO_FLASH,
        dest_address,
        src_address,
        nbytes,
        SYSTEM_CLOCK_IN_KHZ
      };
      iap_call(cmd, cmd);  // result is put back into cmd
      return static_cast<return_code>(cmd[0]);
    }
    
    /**
     * Compare ram to flash
     * @param dest_address, should belong to a prepared sector, and be on a page (256 bytes) boundary
     * @param src_address, should be at a word (4 bytes) boundary
     * @param nbytes: must be one of 256, 512, 1024, 4096
     */
    static return_code compare_ram_to_flash(uint32 dest_address, uint32 src_address, uint32 nbytes) {
      uint32 cmd[] = {
        COMPARE,
        dest_address,
        src_address,
        nbytes
      };
      iap_call(cmd, cmd);  // result is put back into cmd
      return static_cast<return_code>(cmd[0]);
    }
    
    /**
     * Checks if a range of sectors are blank
     * @param non_blank_offset is the offset of the first non blank location
     *                         if the return code is SECTOR_NOT_BLANK
     * @param non_blank_content is the content of the first non blank location
     *                          if the return code is SECTOR_NOT_BLANK
     * @return CMD_SUCCESS if sectors are blank
     */
    static return_code blank_check_sectors(uint32 sector_start, uint32 sector_end, 
                                           uint32 &non_blank_offset, uint32 &non_blank_content) {
      uint32 cmd[] = {
        BLANK_CHECK_SECTORS,
        sector_start,
        sector_end
      };
      iap_call(cmd, cmd);  // result is put back into cmd
      if (cmd[0] == SECTOR_NOT_BLANK) {
        non_blank_offset = cmd[1];
        non_blank_content = cmd[2];        
      }
      return static_cast<return_code>(cmd[0]);
    }
    
    /**
     * Shorter version of the above, when the caller does not care about non blank location
     * and offset
     */
    static return_code blank_check_sectors(uint32 sector_start, uint32 sector_end) {
      uint32 cmd[] = {
        BLANK_CHECK_SECTORS,
        sector_start,
        sector_end
      };
      iap_call(cmd, cmd);  // result is put back into cmd
      return static_cast<return_code>(cmd[0]);
    }
    
    /**
     * Enters the ISP so the chip can be programmed
     */
    static void enter_isp() __attribute__((noreturn)) {
      uint32 cmd[] = {
        REINVOKE_ISP
      };
      iap_call(cmd, cmd);
      for (;;)
        ;
      //return static_cast<return_code>(cmd[0]);
    }
    
    /**
     * Calculates the start address and size for a given sector.
     * Code suits 512k LPC2000 µc, such as lpc2138/lpc2148
     * @return false if the sector is invalid
     */
    inline
    static bool sector_to_address_size(uint32 sector, const uint8* &address, uint32& size) {
      switch (sector) {
        case 0 ... 7:
          size = 1 << 12;  // 4k
          address = reinterpret_cast<const uint8*> (SectorInfo<0>::address + ((sector - 0) * SectorInfo<0>::size));
          break;
        case 8 ... 21:
          size = 1 << 15; // 32k sectors
          address = reinterpret_cast<const uint8*> (SectorInfo<8>::address + ((sector - 8) * SectorInfo<8>::size));
          break;
        case 22 ... 26:
          size = 1 << 12;  // 4k
          address = reinterpret_cast<const uint8*> (SectorInfo<22>::address + ((sector - 22) * SectorInfo<22>::size));
          break;
        default:
          return false;
      }
      return true;
    }
    
    /**
     * @return an address in flash corresponding to the start of the given sector
     */
    static void address_to_sector_offset(const void *address, uint32& sector, uint32& offset) {
      const uint32 addr = (uint32) address;
      
      if (addr < 0x00008000) {
        sector = addr >> 12;  // number of 4k chunks
        offset = addr & 0xfff;
      } else if (addr < 0x00078000) {
        sector = 8 + ((addr - 0x00008000) >> 15);  // number of 32k chunks
        offset = addr & 0x7fff;
      } else {
        sector = 22 + ((addr - 0x00078000) >> 12);  // number of 4k chunks
        offset = addr & 0xfff;
      }
    }
    
  private:
    static void iap_call(uint32 params[], uint32 result[]) {
      typedef void (*IAP)(uint32 params[], uint32 result[]);
      ((IAP)IAP_LOCATION)(params, result);
    }
    
    static const uint32 SYSTEM_CLOCK_IN_KHZ = PCLK / 1000;  // FIXME: Should use CCLK not PLCK

    static const uint32 IAP_LOCATION = 0x7ffffff1;
    enum command {
      PREPARE_SECTOR_FOR_WRITE = 50,
      COPY_RAM_TO_FLASH = 51,
      ERASE_SECTORS = 52,
      BLANK_CHECK_SECTORS = 53,
      READ_PART_ID = 54,
      READ_BOOT_CODE_VERSION = 55,
      COMPARE = 56,
      REINVOKE_ISP = 57
    };

  };
}
