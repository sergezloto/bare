/*
 *  nor_types.h
 *  Embedded
 *
 *  Created by Serge on 18/02/10.
 *  Copyright 2010 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace lpcnor {
  //
  // Types and definitions used internally
  //
  typedef uint8 peb_number_type;
  static const peb_number_type PEB_INVALID = 0xff;
  typedef uint8 leb_number_type;
  static const leb_number_type LEB_INVALID = 0xff;
  typedef uint16 page_number_type;
  typedef uint16 slot_index_type;
  typedef uint16 data_size_type;

  /**
   * The smallest write size is 16 bytes on LPCxxx, due to 
   * the use of ECC.
   * We divide erase blocks into N pages of 16 bytes
   * This results in
   *
   *	Block size	| lines  | Maximum addressable capacity
   *	----------------+------------+-----------------------------
   *	4k		| 256    | 1MB
   *	32k		| 2048   | 8MB
   *	64k		| 4096   | 16MB
   *
   * In practice, we use 12 bits to denote the page number
   */
  enum {
   PEB_WRITE_LINE_SIZE = 16 
  };
}
