#include "WaterLevel.h"

uint8_t low_data[8];
uint8_t high_data[12];
uint8_t channel_buff[1];

//Call this line to read from sensor (might need to mess with delays for repetative calls): uint8_t waterleveltest = checkWaterLevel();
uint8_t checkWaterLevel(uint8_t channel) 
{
  uint8_t low_count = 0;
  uint8_t high_count = 0;
  uint32_t touch_val = 0;
  uint8_t trig_section = 0;
  
  //Select the right I2C Mux Channel
  channel_buff[0] = channel;
  i2c_io(MUX_ADDR, channel_buff, 1, NULL, 0);
  //_delay_us(3);

  i2c_io(ATTINY1_HIGH_ADDR, NULL, 0, high_data, 12);   
  //_delay_us(3);

  i2c_io(ATTINY2_LOW_ADDR, NULL, 0, low_data, 8);
  //_delay_us(3);
  
  uint8_t i;
  for (i = 0 ; i < 8; i++) {
    if (low_data[i] > THRESHOLD) {
      touch_val |= (1 << i);

    }
  }
  for (i = 0 ; i < 12; i++) {
    if (high_data[i] > THRESHOLD) {
      touch_val |= (uint32_t)1 << (8 + i);
    }
  }

  while (touch_val & 0x01)
  {
    trig_section++;
    touch_val >>= 1;
  }
  //_delay_ms(1);

  return (trig_section*5);
}