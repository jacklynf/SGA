#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>

#include "light_sensor.h"
#include "i2c.h"

#define LIGHTSENSOR_ADDR 0x29

// Initiate communication with the light sensor to get device ID
// First write request to sensor, then read its response
uint8_t begin_lightsensor(){
    uint8_t wb[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID}; 
    uint8_t rb[1];
    uint8_t write_status, read_status;

    if ((write_status = i2c_io(LIGHTSENSOR_ADDR, wb, 1, NULL, 0)) != 0)
        return write_status;
    if ((read_status = i2c_io(LIGHTSENSOR_ADDR, wb, 1, rb, 1)) != 0)
        return read_status;
    return rb[0];
}

uint16_t enable_lightsensor(){    
    uint8_t wb[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWERON};
    // uint8_t wb[] = {TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWERON};
    return i2c_io(LIGHTSENSOR_ADDR, wb, 2, NULL, 0); // Returns 0 on success
}

uint8_t configure_lightsensor(){    
  uint8_t integration = TSL2591_INTEGRATIONTIME_100MS;
  uint8_t gain = TSL2591_GAIN_MED;
  uint8_t reg_addr = TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL;
  uint8_t command = integration | gain;
  uint8_t wb[2] = {reg_addr, command}, rb[1] = {0};
  uint8_t read_status, write_status;

  if ((write_status = i2c_io(LIGHTSENSOR_ADDR, wb, 2, NULL, 0)) != 0)
        return 3;
  if ((read_status = i2c_io(LIGHTSENSOR_ADDR, wb, 1, rb, 1)) != 0)
        return 4;

  return 0;
}
