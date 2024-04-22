#include "WaterLevel.h"
#include <util/delay.h>

uint8_t low_data[8];
uint8_t high_data[12];


void getHigh12SectionValue(void)
{
    i2c_io(ATTINY1_HIGH_ADDR, NULL, 0, high_data, 12);
    _delay_ms(1);
}

void getLow8SectionValue(void)
{
    i2c_io(ATTINY2_LOW_ADDR, NULL, 0, low_data, 8);
    _delay_ms(1);
}


//Call this line to read from sensor (might need to mess with delays for repetative calls): uint8_t waterleveltest = checkWaterLevel();
uint8_t checkWaterLevel(void) 
{
  uint8_t sensorvalue_min = 250;
  uint8_t sensorvalue_max = 255;
  uint8_t low_count = 0;
  uint8_t high_count = 0;
    uint32_t touch_val = 0;
    uint8_t trig_section = 0;
    low_count = 0;
    high_count = 0;
    getLow8SectionValue();
    getHigh12SectionValue();

    // Serial.println("low 8 sections value = ");
    // for (int i = 0; i < 8; i++)
    // {
    //   Serial.print(low_data[i]);
    //   Serial.print(".");
    //   if (low_data[i] >= sensorvalue_min && low_data[i] <= sensorvalue_max)
    //   {
    //     low_count++;
    //   }
    //   if (low_count == 8)
    //   {
    //     Serial.print("      ");
    //     Serial.print("PASS");
    //   }
    // }
    // Serial.println("  ");
    // Serial.println("  ");
    // Serial.println("high 12 sections value = ");
    // for (int i = 0; i < 12; i++)
    // {
    //   Serial.print(high_data[i]);
    //   Serial.print(".");

    //   if (high_data[i] >= sensorvalue_min && high_data[i] <= sensorvalue_max)
    //   {
    //     high_count++;
    //   }
    //   if (high_count == 12)
    //   {
    //     Serial.print("      ");
    //     Serial.print("PASS");
    //   }
    // }

    // Serial.println("  ");
    // Serial.println("  ");
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