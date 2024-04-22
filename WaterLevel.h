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


uint8_t checkWaterLevel(void);

void getLow8SectionValue(void);

void getHigh12SectionValue(void);


#endif