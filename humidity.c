#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <limits.h>
#include "humidity.h"

// Humidity sensor code adapted from SparkFun source code SparkFun_RHT03.cpp:
// https://github.com/sparkfun/SparkFun_RHT03_Arduino_Library/blob/master/src/SparkFun_RHT03.cpp

// micros() method adapted from Arduino with help from:
// // https://garretlab.web.fc2.com/en/arduino/inside/hardware/arduino/avr/cores/arduino/wiring.c/micros.html

// Begin variables for micros() function
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

#define MAX_TIMEOUT UINT_MAX
#define LOW         ~(1 << PD3)
#define HIGH        (1 << PD3)
 
volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;
// End micros() function variables

uint64_t counter = 0;

void init_humidity(){ // Initialize pin as input with pullup
    _dataPin = (1 << PD3);
    DDRC &= ~_dataPin;
    PORTD |= _dataPin;
    _delay_ms(1000); // Sensor needs 1s after power on to stabilize
}

uint16_t get_humidity(){
    return _humidity;
}

uint8_t update_humidity(){
    int i, j;
    uint64_t high_time[40] = {0};
    _humidity = 0;

    cli(); // Cannot interrupt start signal and reading

    _delay_us(1);
    // Sensor start signal begin
    // Begin state: input HIGH
    DDRD &= ~(1 << PD3); // Set PD3 as input
    PORTD |= (1 << PD3); // Enable pullup
    _delay_ms(100);
    // Start signal: host sets data low, waits 500us, then pulls back up, wait 20-40us
    DDRD |= (1 << PD3); // Set PD3 as output
    PORTD &= ~(1 << PD3); // Send PD3 low
    _delay_us(500); // Wait 500us minimum per datasheet
    DDRD &= ~(1 << PD3); 
    PORTD |= (1 << PD3); 
    _delay_us(20);
    // Sensor should pull data pin low 80us, then pull back up    
    _delay_us(80); // Wait for sensor to hold low for 80us
    while (PIND & (1 << PD3)) // 
        _delay_us(1);
    // Sensor start signal end


    /*
        Sensor transmits 40 bytes (16 rh, 16 temp, 8 checksum)
        Each byte starts with a ~50us LOW then a HIGH pulse. The duration of the
        HIGH pulse determines the value of the bit.
        If the duration of the received high signal is between 26-28us, the bit is 0
        If the duration of the received high signal is 70us, the bit is 1
    */    
    counter = 0;
    for (i = 0; i < 32; i++)
    {
        _delay_us(50);
        while (PIND & (1 << PD3)){
            _delay_us(1);
            counter++;
        }
        high_time[i] = counter;
        counter = 0;
    }

    while (~PIND & (1 << PD3))
        _delay_us(1);

    sei();
    
    uint64_t val;
    j = 7;
    for (i = 0; i < 8; i++){ // Only need the first 8 values for humidity integer value        
        if ((high_time[i] >= 0)&&(high_time[i] < 3))         
            val = 0;
        else if ((high_time[i] >= 3)&&(high_time[i] < 10))
            val = 1;
        else
            return 0; // If value is outside of these boundaries, something didn't go right
        _humidity = _humidity | (val << j);
        
        if ((high_time[i+16] >= 0)&&(high_time[i+16] < 3))         
            val = 0;
        else if ((high_time[i+16] >= 3)&&(high_time[i+16] < 10))
            val = 1;
        else
            return 0; // If value is outside of these boundaries, something didn't go right        
        j--;
    }
    

    return 1;
}

