#define MUX_ADDR (0x70 << 1)    //I2C Mux 7-bit address
#define WATER_LEVEL_CHANNEL (1 << 0) // Water level sensor addr at mux
#define FERTILIZER_CHANNEL (1 << 2) // Fertilizer level sensor addr at mux
#define ATTINY1_HIGH_ADDR (0x78 << 1)
#define ATTINY2_LOW_ADDR (0x77 << 1)

#ifndef WATERLEVEL_H
#define WATERLEVEL_H

#include <avr/io.h>
#include <stdbool.h>
#include "i2c.h"
#include <stddef.h>
#include <util/delay.h>

#define NO_TOUCH 0xFE
#define THRESHOLD 100

uint8_t checkWaterLevel(uint8_t);

void getLow8SectionValue(void);

void getHigh12SectionValue(void);


#endif