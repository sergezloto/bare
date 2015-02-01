/*
 *  HAL_HMC6352.h
 *  Embedded
 *
 *  Created by Serge on 06/06/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_I2C.h"
#include "HAL_Driver.h"

namespace hal {
  
  /*
   * Driver for the I2C HMC6352 magnetometer.
   */
  class HMC6352: public Driver, NoCopy {
    static const uint8 DEFAULT_ADDRESS = 0x42;
public:
    /*
     * Constructs the driver atop the given I2C interface
     */
    HMC6352(I2C& i2c, I2C::Address address = DEFAULT_ADDRESS);
    
    /*
     * Sets up the device for use
     * @return false if device cannot be found or has an error condition
     */
    bool configure();
    
    /*
     * Obtains the heading in tenths of a degree
     * @return false if device cannot be found or has an error condition
     */
    bool getHeading(uint16& heading) const;
    
    /*
     * @return software version
     */
    uint8 getSoftwareVersion() const;
        
private:
    enum Command {
      READ_EEPROM = 'r',  // eeprom address, data
      WRITE_EEPROM = 'w',  // eeprom address, data
      READ_RAM = 'g',  // ram address, data
      WRITE_RAM = 'G',   // ram address, data
      SLEEP = 'S',
      WAKEUP = 'W',
      ENTER_CALIBRATION = 'C',
      EXIT_CALIBRATION = 'E',
      SAVE_OPMODE_TO_EEPROM = 'L',
      GET_DATA = 'A'  // Next read transaction receives MSB and LSB heading in tenths of a degree
    };
    
    /*
     * The HMC6352 registers, also known as subaddress in  the documentation.
     */
    enum MagReg {
      SLAVE_ADDRESS = 0,
      X_MSB,
      X_LSB,
      Y_MSB,
      Y_LSB,
      DELAY_MS,
      SW_VERSION,
      OP_MODE
    };
    
    enum Mode {
      STANDBY = 0,  // Every measurement must be preceded by GET_DATA
      QUERY = 1,  // An initial GET_DATA is needed. Afterward each read will return new data
      CONTINUOUS = 2  // The device keeps measuring at a specified rate
    };
    
    enum EepromAddress {
      ADDR_SLAVE            = 0, // I2C Slave Address 42(hex) 
      ADDR_XOFFSETM         = 1, // Magnetometer X Offset MSB factory test value 
      ADDR_XOFFSETL         = 2, // Magnetometer X Offset LSB factory test value 
      ADDR_YOFFSETM         = 3, // Magnetometer Y Offset MSB factory test value 
      ADDR_YOFFSETL         = 4, // Magnetometer Y Offset LSB factory test value 
      ADDR_TIME_DELAY       = 5, // Time Delay (0 – 255  ms) 01(hex) 
      ADDR_SUM_MEASUREMENTS = 6, // Number of Summed measurements(1-16) 04(hex) 
      ADDR_SW_VERSION       = 7, // Software Version Number > 01(hex) 
      ADDR_OPERATION_MODE   = 8, // Operation Mode Byte 50(hex) 
    };
    
    
    const I2C& m_i2c;
    const I2C::Address m_address;
    
  };
  
  inline
    HMC6352::HMC6352(I2C& i2c, I2C::Address address) 
    : m_i2c(i2c), m_address(address) {
    }
  
  inline
    bool HMC6352::configure() {
      return true;
    }
}
