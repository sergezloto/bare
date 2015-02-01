/*
 *  HAL_LIS3LV02DQ.cpp
 *  Embedded
 *
 *  Created by Serge on 30/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_LIS3LV02DQ.h"

namespace  hal {
  
  bool LIS3LV02DQ::configure() {
    static const uint8 buf[] = {
      CTRL_REG_1,
      XEN | YEN | ZEN | PD0 | PD1
    };
    uint8 count = m_i2c.send(m_address, buf, sizeof buf);
    
    return count == sizeof buf;
  }
  
  bool LIS3LV02DQ::getAccelXYZ(int32 &x, int32 &y, int32 &z) const {
    // For reading multiple bytes, the subaddress MSB must be 1
    static const uint8 subaddress = OUT_X_L | 0x80;
    uint8 count;
    
    count = m_i2c.send(m_address, &subaddress, sizeof subaddress);
    if (count != sizeof subaddress)
      return false;
    
    uint8 buf[6];
    count = m_i2c.receive(m_address, buf, sizeof buf);
    if (count != sizeof buf)
      return false;
    
    // The chip provides data as big endian. Convert to platform format
    x = (int16) (buf[0] | (buf[1] << 8));
    y = (int16) (buf[2] | (buf[3] << 8));
    z = (int16) (buf[4] | (buf[5] << 8));
    
    return true;
  }
  
  bool LIS3LV02DQ::getOffsetXYZ(int32 &x, int32 &y, int32 &z) const {
    // For reading multiple bytes, the subaddress MSB must be 1
    static const uint8 subaddress = OFFSET_X | 0x80;
    uint8 count;
    
    count = m_i2c.send(m_address, &subaddress, sizeof subaddress);
    if (count != sizeof subaddress)
      return false;
    
    uint8 buf[3];
    count = m_i2c.receive(m_address, buf, sizeof buf);
    if (count != sizeof buf)
      return false;
    
    // The chip provides data as big endian. Convert to platform format
    x = buf[0];
    y = buf[1];
    z = buf[2];
    
    return true;
  }

}
