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
#define LOW         0
#define HIGH        1
 
volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;
// End micros() function variables

unsigned int counter = 0;

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
    int i, j;
    unsigned long marks[41] = {0};
    unsigned long stops[40] = {0};
    unsigned int highTime, lowTime;
    unsigned int high_time[40] = {0};
    char dataBytes[5] = {0};

    cli();

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
    if (!waitForRHT(LOW, MAX_TIMEOUT))
        return errorExit(0);
    if (!waitForRHT(HIGH, MAX_TIMEOUT))
        return errorExit(0);

    // Sensor transmits 40 bytes (16 rh, 16 temp, 8 checksum)
    // Each byte starts with a ~50us LOW then a HIGH pulse. The duration of the
    // HIGH pulse determines the value of the bit.
    // If the duration of the received high signal is between 26-28us, the bit is 0
    // If the duration of the received high signal is 70us, the bit is 1
    for (i = 0; i < 40; i++)
    {
        if (!waitForRHT(LOW, MAX_TIMEOUT))
            return errorExit(0);
        // marks[i] = micros();
        if (!waitForRHT(HIGH, MAX_TIMEOUT)) 
            return errorExit(0);
        high_time[i] = counter;
        counter = 0;
        // stops[i] = micros();
    }
    if (!waitForRHT(LOW, MAX_TIMEOUT))
        return errorExit(0);
    // marks[16] = micros();
    sei();
    
    
    // for (i = 0; i < 40; i++)
    // {
    //     lowTime = stops[i] - marks[i];
    //     highTime = marks[i + 1] - stops[i];
    //     if (highTime > lowTime)
    //     {
    //         dataBytes[i / 8] |= (1 << (7 - i % 8));
    //     }
    // }
    uint8_t val;
    for (i = 0, j = 7; i < 8, j >= 0; i++, j--){
        if ((high_time[i] > 25)&&(high_time[i] < 35))
            val = 0;
        else if ((high_time[i] > 65)&&(high_time[i] < 75))
            val = 1;
        _humidity = _humidity | (val << j);
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

_Bool waitForRHT(int pinState, unsigned int timeout)
{
    
    while (((PIND & (1 << PD3)) != pinState) && (counter++ < timeout))
        _delay_us(1);

    if (pinState  == LOW) // Reset counter before high signal
        counter = 0; 

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