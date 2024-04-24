// Code adapted from Adafruit GitHub library
// https://github.com/adafruit/Adafruit_TSL2591_Library/blob/master/Adafruit_TSL2591.cpp#L467 

#include "light_sensor.h"

// Initiate communication with the light sensor to get device ID
// First write request to sensor, then read its response
uint8_t init_lightsensor() {
    buff[0] = (TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID); 
    if ((read_status = i2c_io(LIGHTSENSOR_ADDR, buff, 1, buff, 1)) != 0)
        return read_status;
    
    if (buff[0] != 0x50)
        return buff[0];

    enable_lightsensor();
    
    buff[0] = (TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL);
    buff[1] = (TSL2591_INTEGRATIONTIME_100MS | TSL2591_GAIN_MED);
    i2c_io(LIGHTSENSOR_ADDR, buff, 2, NULL, 0);

    disable_lightsensor();

    return 0;

}

// uint16_t get_luminosity() {
//     uint16_t x = 0;
    
//     enable_lightsensor();
    
//     uint8_t d;
//     for (d = 0; d <= 2; d++) { // wait for sensor to gather data
//         _delay_ms(120);
//     }

//     buff[0] = (TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW);
//     i2c_io(LIGHTSENSOR_ADDR, buff, 1, buff, 2);
    
//     x |= (buff[1] << 8) | (buff[0]);

//     disable_lightsensor();
//     // Check for overflow conditions first
//     if (x == 0xFFFF) {
//         // Signal an overflow
//             return 0;
//     }
//     return x;            
// }

// uint32_t get_luminosity(){
//     uint32_t x = 0;
    
//     enable_lightsensor();
    
//     uint8_t d;
//     for (d = 0; d <= _integration; d++) { // wait for sensor to gather data
//         _delay_ms(1);
//     }

//     buff[0] = (TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW);
//     i2c_io(LIGHTSENSOR_ADDR, buff, 1, buff, 2);
    
//     x |= (buff[1] << 8) | (buff[0]);

//     buff[0] = (TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW);
//     i2c_io(LIGHTSENSOR_ADDR, buff, 1, buff, 2);

//     x |= (buff[1] << 24) | (buff[0] << 16);

//     disable_lightsensor();
//     // Check for overflow conditions first
//     if ((x & 0xFFFF0000) == 0xFFFF0000 | (x & 0xFFFF) == 0xFFFF) {
//         // Signal an overflow
//             return 0;
//     }
//     return x;            
// }

uint32_t get_luminosity(void) {
  uint8_t x[2];
  uint8_t y[2];
  uint8_t buff1[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW};
  uint8_t buff2[] = {TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW};
  // Enable the device
  enable_lightsensor();

  // Wait x ms for ADC to complete
  uint8_t d;
  for (d = 0; d <= TSL2591_INTEGRATIONTIME_100MS; d++) {
    _delay_ms(120);
  }

  // CHAN0 must be read before CHAN1
  // See: https://forums.adafruit.com/viewtopic.php?f=19&t=124176
    i2c_io(LIGHTSENSOR_ADDR, buff1, 1, x, 2);
    i2c_io(LIGHTSENSOR_ADDR, buff2, 1, y, 2);
    disable_lightsensor();

    uint32_t final = ((uint32_t)(y[1]) << 24) | ((uint32_t)(y[0]) << 16) | ((uint32_t)(x[1]) << 8) | ((uint32_t)(x[0]));


  return final;
}