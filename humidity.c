#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <limits.h>
#include "humidity.h"

// Humidity sensor code adapted from SparkFun source code SparkFun_RHT03.cpp:
// https://github.com/sparkfun/SparkFun_RHT03_Arduino_Library/blob/master/src/SparkFun_RHT03.cpp

// micros() method adapted from Arduino with help from:
// // https://garretlab.web.fc2.com/en/arduino/inside/hardware/arduino/avr/cores/arduino/wiring.c/micros.html

// Begin macros and variables for micros() function
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )
 
volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;
// End micros() function macros and variables

void init_humidity(){
    _dataPin = (1 << PD3);
    DDRC &= ~_dataPin;
    PORTD |= _dataPin;
    _delay_ms(1000); // Sensor needs 1s after power on to stabilize
}

uint16_t get_humidity(){
    return _humidity ;
}

uint8_t update_humidity(){
    int i;
    const unsigned int MAX_TIMEOUT = UINT_MAX; // no timeouts occur with this value but it takes too long, need scope 
    const int LOW =0, HIGH =1;
    unsigned long marks[41] = {0};
    unsigned long stops[40] = {0};
    unsigned int highTime, lowTime;
    char dataBytes[5] = {0};
    cli();

    // Begin state: input HIGH
    DDRD &= ~(1 << PD3); // Set PD3 as input
    PORTD |= (1 << PD3); // Enable pullup
    _delay_ms(100);
    // Start signal: host sets data low, waits 1 ms, then pulls back up, wait 20-40us
    DDRD |= (1 << PD3); // Set PD3 as output
    PORTD &= ~(1 << PD3); // Send PD3 low
    _delay_ms(2); // Wait 1 ms minimum
    DDRD &= ~(1 << PD3); 
    PORTD |= (1 << PD3); 
    _delay_us(20);
    // Sensor should pull data pin low 80us, then pull back up
    if (!waitForRHT(LOW, 1000))
        return errorExit(0);
    if (!waitForRHT(HIGH, 1000))
        return errorExit(0);

    // Sensor transmits 40 bytes (16 rh, 16 temp, 8 checksum)
    // Each byte starts with a ~50us LOW then a HIGH pulse. The duration of the
    // HIGH pulse determi3.nes the value of the bit.
    // LOW: 26-28us (<LOW duration)
    // HIGH: 70us (>LOW duration)
    for (i = 0; i < 40; i++)
    {
        if (!waitForRHT(LOW, 1000))
            return errorExit(0);
        marks[i] = micros();
        if (!waitForRHT(HIGH, 1000))
            return errorExit(0);
        stops[i] = micros();
    }
    if (!waitForRHT(LOW, 1000))
        return errorExit(-0);
    marks[40] = micros();

    sei();

    
    for (i = 0; i < 40; i++)
    {
        lowTime = stops[i] - marks[i];
        highTime = marks[i + 1] - stops[i];
        if (highTime > lowTime)
        {
            dataBytes[i / 8] |= (1 << (7 - i % 8));
        }
    }

    if (checksum(dataBytes[CHECKSUM], dataBytes, 4))
    {
        _humidity = ((uint16_t)dataBytes[HUMIDITY_H] << 8) | dataBytes[HUMIDITY_L];

        // end of mod
        return 1;
    }
    return 0;
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

_Bool waitForRHT(int pinState, unsigned int timeout)
{
    unsigned int counter = 0;
    while (((PIND & (1 << PD3)) != pinState) && (counter++ < timeout))
        _delay_us(1);

    if (counter >= timeout)
        return false;
    else
        return true;
}

unsigned long micros(){
    unsigned long m;
    uint8_t oldSREG = SREG, t;
     
    m = timer0_overflow_count;
    t = TCNT0;
 
    if ((TIFR0 & _BV(TOV0)) && (t < 255))
        m++;
 
    SREG = oldSREG;
     
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