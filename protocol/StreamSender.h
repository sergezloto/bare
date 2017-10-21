/*
 *  Sender.h
 *  Embedded
 *
 *  Created by Serge on 27/11/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"
#include "util.h"

namespace protocol {
  /**
   * An base class for sending bytes string.
   * It enables decoupling between a protocol stream encoder and a transmission mechanism.
   * Derive from this class and implement the write() function.
   */
  typedef util::Writer StreamSender;
}
