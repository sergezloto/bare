/*
 *  ios.h
 *  Embedded
 *
 *  Created by Serge on 2/24/09.
 *  Copyright 2009 Zorobo. All rights reserved.
 *
 */

#pragma once

namespace util {
  
  class ios: NoInstance {
  public:
    static char digit(uint32 u) {
      static char m_digit[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
      };
      if (u >= sizeof m_digit)
        return '#';
      return m_digit[u];
    }

    enum base {
      dec = 10,
      hex = 16
    };
  private:
    ios();
  };
}
