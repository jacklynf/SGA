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

uint64_t update_humidity(){
    int i, j;
    unsigned long marks[41] = {0};
    unsigned long stops[40] = {0};
    unsigned int highTime, lowTime;
    uint64_t high_time[40] = {0};
    char dataBytes[5] = {0};
    _humidity = 0;

    cli();

    _delay_us(1);
    // Begin state: input HIGH
    DDRD &= ~(1 << PD3); // Set PD3 as input
    PORTD |= (1 << PD3); // Enable pullup
    _delay_ms(100);
    // Start signal: host sets data low, waits 500us, then pulls back up, wait 20-40us
    DDRD |= (1 << PD3); // Set PD3 as output
    PORTD &= ~(1 << PD3); // Send PD3 low
    _delay_us(500); // Wait 500us minimum
    DDRD &= ~(1 << PD3); 
    PORTD |= (1 << PD3); 
    _delay_us(20);
    // Sensor should pull data pin low 80us, then pull back up
    // if (!waitForRHT(LOW, MAX_TIMEOUT))
    //     return errorExit(1);
    // if (!waitForRHT(HIGH, MAX_TIMEOUT))
    //     return errorExit(2);
    // while (~PIND & (1 << PD3))
    //     _delay_us(1);
    
    _delay_us(80);
    while (PIND & (1 << PD3))
        _delay_us(1);

    // return 1;
    // _delay_us(1);

    // Sensor transmits 40 bytes (16 rh, 16 temp, 8 checksum)
    // Each byte starts with a ~50us LOW then a HIGH pulse. The duration of the
    // HIGH pulse determines the value of the bit.
    // If the duration of the received high signal is between 26-28us, the bit is 0
    // If the duration of the received high signal is 70us, the bit is 1
    
    counter = 0;
    for (i = 0; i < 8; i++)
    {
        // while (~PIND & (1 << PD3))
        //     _delay_us(1);
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
        j--;
    }

    return 1;

    // if (checksum(dataBytes[CHECKSUM], dataBytes, 4))
    // {
    //     // _humidity = ((uint16_t)dataBytes[HUMIDITY_H] << 8) | dataBytes[HUMIDITY_L];

    //     // end of mod
    //     return 1;
    // }
    // return 0;
}

_Bool checksum(char check, char *data, unsigned int datalen)
{
    char sum = 0;
    int i;
    for (i = 0; i < datalen; i++)
    {
        sum = sum + data[i];
    }
    if (sum == check)
        return true;

    return false;
}

uint8_t errorExit(int code)
{
    sei();
    return code;
}

// _Bool waitForRHT(int pinState, unsigned int timeout)
// {
    
//     // while (((PIND & (1 << PD3)) == pinState) && (counter++ < timeout))
//     //     _delay_us(1);

//     // if (pinState  == LOW) // Reset counter before high signal
//     //     counter = 0; 

//     // if (counter >= timeout)
//     //     return false;
//     // else
//     //     return true;
// }

unsigned long micros(){
    unsigned long m;
    uint8_t oldSREG = SREG, t;
     
    m = timer0_overflow_count;
    t = TCNT0;
 
    // if ((TIFR0 & _BV(TOV0)) && (t < 255))
    //     m++;
 
    // SREG = oldSREG;
     
    return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}
 
ISR(TIMER0_OVF_vect)
{
    // copy these to local variables so they can be stored in registers
    // (volatile variables must be read from memory on every access)
    unsigned long m = timer0_millis;
    unsigned char f = timer0_fract;
 
    m += MILLIS_INC;
    f += FRACT_INC;
    if (f >= FRACT_MAX) {
        f -= FRACT_MAX;
        m += 1;
    }
 
    timer0_fract = f;
    timer0_millis = m;
    timer0_overflow_count++;
}