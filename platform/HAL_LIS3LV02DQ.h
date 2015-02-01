/*
 *  HAL_LIS3LV02DQ.h
 *  Embedded
 *
 *  Created by Serge on 30/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_I2C.h"

namespace hal {
  /*
   * Driver for the LIS3LV02 triple axis accelerometer, in I2C mode.
   */
  class LIS3LV02DQ: public Driver, NoCopy {
    
    /*
     * The LIS3LV02 registers, also known as subaddress in  the documentation.
     */
    enum Register {
      WHO_AM_I	= 0x0f,
      OFFSET_X  = 0x16,
      OFFSET_Y  = 0x17,
      OFFSET_Z  = 0x18,
      GAIN_X		= 0x19,
      GAIN_Y		= 0x1a,
      GAIN_Z		= 0x1b,
      CTRL_REG_1 = 0x20,
      CTRL_REG_2 = 0x21,
      CTRL_REG_3 = 0x22,
      STATUS_REG = 0x27,
      OUT_X_L		= 0x28,
      OUT_X_H		= 0x29,
      OUT_Y_L		= 0x2a,
      OUT_Y_H		= 0x2b,
      OUT_Z_L		= 0x2c,
      OUT_Z_H		= 0x2d,
      FF_WU_CFG	= 0x30,
      FF_WU_SRC	= 0x31,
      FF_WU_ACK	= 0x32,
      FF_WU_THS_L	= 0x34, 
      FF_WU_THS_H	= 0x35,
      FF_WU_DURATION	= 0x36, 
      DD_CFG		= 0x38,
      DD_SRC		= 0x39,
      DD_ACK		= 0x3A,
      DD_THSI_L	= 0x3C,
      DD_THSI_H	= 0x3D,
      DD_THSE_L	= 0x3E,
      DD_THSE_H	= 0x3F
    };
    
    enum REG_1_Values {
      XEN		= 1,
      YEN		= 1 << 1,
      ZEN		= 1 << 2,
      PD0		= 1 << 6,
      PD1		= 1 << 7
    };
    
public:
    /*
     * Constructs the driver on top of the given I2C interface, with the 
     * given address.
     * The address does not seem to be modifiable in hardware.
     */
    LIS3LV02DQ(I2C& i2c, I2C::Address address = 0x3a);
    
    /*
     * Enables the X, Y, Z axis measurements.
     * @return false if the device is not found on the bus or does not respond
     */
    bool configure();
    
    /*
     * Obtains 3-axis acceleration data
     * @return false if the data is not valid for any reason
     */
    bool getAccelXYZ(int32 &x, int32 &y, int32 &z) const;
    
    /*
     * Obtains factory-calibrated offsets
     */
    bool getOffsetXYZ(int32 &x, int32 &y, int32 &z) const;
    
private:
    const I2C& m_i2c;
    const I2C::Address m_address;
    
    LIS3LV02DQ(const LIS3LV02DQ&);
  };
  
  inline
    LIS3LV02DQ::LIS3LV02DQ(I2C& i2c, I2C::Address address) 
    : m_i2c(i2c), m_address(address) {
    }
}
