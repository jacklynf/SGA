#include <avr/io.h>
#include <stdbool.h>

// humidity.h adapted from SparkFun_RHT03.h at https://github.com/sparkfun/SparkFun_RHT03_Particle_Library

uint8_t _dataPin;
uint16_t _humidity;


void init_humidity();

/*
    update_humidity(void);    
    This function communicates with the sensor and receives a humidity reading back.
    Needs to be called prior to reading the sensor.
    Returns 1 on success, 0 on failure.
*/
uint8_t update_humidity();

// Returns 16 bit humidity value
uint16_t get_humidity();
