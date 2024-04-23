#include "WaterLevel.h"
#include "i2c_mux.h"
#include <util/delay.h>
#include <stdint.h>

uint8_t low_data[8];
uint8_t high_data[12];


void getHigh12SectionValue(uint8_t channel)
{
  uint8_t buff[1] = {channel};
  uint8_t b[1];
  i2c_io(MUX_ADDR, buff, 1, NULL, 0);
  _delay_ms(1);
  i2c_io((0x78 << 1), NULL, 0, high_data, 12);
  close_all();    
}

void getLow8SectionValue(uint8_t channel)
{
  uint8_t buff[1] = {channel};
  uint8_t b[1];
  i2c_io(MUX_ADDR, buff, 1, NULL, 0);
  _delay_ms(1);
  i2c_io((0x77 << 1), NULL, 0, low_data, 8);
  close_all(); 
}


//Call this line to read from sensor (might need to mess with delays for repetative calls): uint8_t waterleveltest = checkWaterLevel();
uint8_t checkWaterLevel(uint8_t channel) 
{
  uint8_t sensorvalue_min = 250;
  uint8_t sensorvalue_max = 255;
  uint8_t low_count = 0;
  uint8_t high_count = 0;
  uint32_t touch_val = 0;
  uint8_t trig_section = 0;
  low_count = 0;
  high_count = 0;
  getLow8SectionValue(channel);
  getHigh12SectionValue(channel);


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
  _delay_ms(1);

  return (trig_section*5);
}