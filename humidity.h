#include <avr/io.h>
#include <stdbool.h>

// humidity.h adapted from SparkFun_RHT03.h at https://github.com/sparkfun/SparkFun_RHT03_Particle_Library

#define RHT_READ_INTERVAL_MS 1000

#ifndef RHT03_h
#define RHT03_h

uint8_t _dataPin;
uint16_t _humidity;

enum dataByteLocations { HUMIDITY_H, HUMIDITY_L, TEMP_H, TEMP_L, CHECKSUM };

void init_humidity();
uint64_t update_humidity();
uint16_t get_humidity();
_Bool checksum(char check, char * data, unsigned int datalen);
uint8_t errorExit(int code);
_Bool waitForRHT(int pinState, unsigned int timeout);
unsigned long micros();

#endif