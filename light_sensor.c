// Code adapted from Adafruit GitHub library
// https://github.com/adafruit/Adafruit_TSL2591_Library/blob/master/Adafruit_TSL2591.cpp#L467 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>

#include "light_sensor.h"
#include "i2c.h"

#define LIGHTSENSOR_ADDR (0x29 << 1)

// Initiate communication with the light sensor to get device ID
// First write request to sensor, then read its response
uint8_t init_lightsensor(){
    uint8_t wb[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID}; 
    uint8_t read_status;

    if ((read_status = i2c_io(LIGHTSENSOR_ADDR, wb, 1, wb, 1)) != 0)
        return read_status;
    return wb[0];
}

uint8_t enable_lightsensor(){    
    uint8_t wb[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE, 
                    TSL2591_ENABLE_POWERON| TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN | TSL2591_ENABLE_NPIEN};
    return i2c_io(LIGHTSENSOR_ADDR, wb, 2, NULL, 0); // Returns 0 on success
}

uint8_t disable_lightsensor(){ 
    uint8_t wb[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWEROFF};
    return i2c_io(LIGHTSENSOR_ADDR, wb, 2, NULL, 0); // Returns 0 on success
}

uint8_t configure_lightsensor(){    
  _integration = TSL2591_INTEGRATIONTIME_600MS;
  uint8_t gain = TSL2591_GAIN_MED;
  uint8_t reg_addr = TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL;
  uint8_t command = _integration | gain;
  uint8_t wb[2] = {reg_addr, command};

  return i2c_io(LIGHTSENSOR_ADDR, wb, 2, NULL, 0);
}

uint16_t get_luminosity(){
    uint8_t d;
    uint8_t wby[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW};
    uint8_t wbx[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW};
    uint16_t rbx[2], rby[2], bx, full, y;
    uint32_t x;
    
    enable_lightsensor();
    for (d = 0; d <= _integration; d++) // wait for sensor to gather data
        _delay_ms(100);

    if (!i2c_io(LIGHTSENSOR_ADDR, wby, 1, rby, 2)){
        if (!i2c_io(LIGHTSENSOR_ADDR, wbx, 1, rbx, 2)){
            y = (rby[1] << 8) | rby[0];
            x = (rbx[1] << 8) | rbx[0];
            disable_lightsensor();
            // Check for overflow conditions first
            if ((y == 0xFFFF) | (x == 0xFFFF)) {
                // Signal an overflow
                return 0;
            }
            return x;            
        }
    }

}
