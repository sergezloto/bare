/*
 *  HAL_AudioSource.h
 *  Embedded
 *
 *  Created by Serge on 6/18/08.
 *  Copyright 2008 Zorobo. All rights reserved.
 *
 */

#pragma once

#include "base.h"

namespace hal {
  /**
   * Defines an audio source interface.
   * Not for actual use.
   */
  class AudioSource: NoCopy, NoHeap {
    class Buffer;
  public:
    bool append_samples(Buffer& buffer);
    bool at_eof() const;
  };
}
