/*
 *  HAL_HMC6352.cpp
 *  Embedded
 *
 *  Created by Serge on 06/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_HMC6352.h"

namespace  hal {
  
  bool HMC6352::getHeading(uint16& heading) const {
    // Preparing to read MAG address
    static const uint8 cmd = GET_DATA;
    uint8 nbytes = m_i2c.send(m_address, &cmd, sizeof cmd);
    if (nbytes != sizeof cmd) {
      return false;
    }
    
    // Now read the value
    uint8 mag_data[2];
    nbytes = m_i2c.receive(m_address, mag_data, sizeof mag_data);
    if (nbytes != sizeof mag_data) {
      return false;
    }
    
    heading = (mag_data[0] << 8) | mag_data[1];
    
    return true;
  }
  
  uint8 HMC6352::getSoftwareVersion() const {
    static const uint8 cmd[2] = { READ_EEPROM, ADDR_SW_VERSION };
    uint8 nbytes = m_i2c.send(m_address, cmd, sizeof cmd);
    if (nbytes != sizeof cmd)
      return 100 + nbytes;
        
    // Read now
    static uint8 reply[1];
    nbytes = m_i2c.receive(m_address, reply, sizeof reply);
    if (nbytes != sizeof reply)
      return 200 + nbytes;
    
    return reply[0];
  }
}
