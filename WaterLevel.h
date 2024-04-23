#ifndef WATERLEVEL_H
#define WATERLEVEL_H

#include <avr/io.h>
#include <stdbool.h>
#include "i2c.h"
#include <stddef.h>
#include <util/delay.h>

#define NO_TOUCH 0xFE
#define THRESHOLD 100
#define ATTINY1_HIGH_ADDR 0x78
#define ATTINY2_LOW_ADDR 0x77


uint8_t checkWaterLevel(uint8_t);

void getLow8SectionValue(uint8_t);

void getHigh12SectionValue(uint8_t);


#endif