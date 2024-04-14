#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
// #include <stdbool.h>

// #include "humidity.h"

// #define LOW 0
// #define HIGH 1

// // Humidity sensor code implemented with help from SparkFun source code:
// // https://github.com/sparkfun/SparkFun_RHT03_Arduino_Library/blob/master/src/SparkFun_RHT03.cpp

// // micros() method adapted from Arduino with help from:
// // https://garretlab.web.fc2.com/en/arduino/inside/hardware/arduino/avr/cores/arduino/wiring.c/micros.html



// // the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// // the overflow handler is called every 256 ticks.
// #define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
 
// // the whole number of milliseconds per timer0 overflow
// #define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
 
// // the fractional number of milliseconds per timer0 overflow. we shift right
// // by three to fit these numbers into a byte. (for the clock speeds we care
// // about - 8 and 16 MHz - this doesn't lose precision.)
// #define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
// #define FRACT_MAX (1000 >> 3)
// #define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
// #define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
// #define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )
 
// volatile unsigned long timer0_overflow_count = 0;
// volatile unsigned long timer0_millis = 0;
// static unsigned char timer0_fract = 0;


// volatile uint8_t pin_changed = 0;
// volatile uint8_t us = 0;






// void init_humidity(){
//     DDRD &= ~(1 << PD3);  // Configure PD3 as input pin
//     PORTD |= (1 << PD3);  // Enable pullup resistor on PD3
//     PCMSK2 |=  (1 << PCINT19); // set mask bits to track PD3 pin changes
// }

// uint16_t get_humidity(){
//     uint16_t humidity;
//     uint8_t i;
//     unsigned long marks[41] = {0};
//     unsigned long stops[40] = {0};
//     unsigned int highTime, lowTime;
//     char dataBytes[5] = {0};
    
//     // Begin state: input HIGH
//     DDRD &= ~(1 << PD3); // Input
//     PORTD |= (1 << PD3); // pullup
//     _delay_ms(100);

//     // Start signal: uC sets data low, waits 1 ms, then pulls back up, wait 20-40us
//     DDRD |= (1 << PD3);  // output
//     PORTD &= ~(1 << PD3);  // send low signal for 1 ms
//     _delay_ms(1); // Wait 1 ms minimum
//     DDRD &= ~(1 << PD3);  // input
//     PORTD |= (1 << PD3); // pullup
//     _delay_us(40);
    
//     // Sensor should pull data pin low 80us, then pull back up
//     if (!wait_for_sensor(LOW, 1000))
//         return 51;
//     if (!wait_for_sensor(HIGH, 100000))
//         return 1;

//     /*
//         Sensor transmits 40 bytes (16 rh, 16 temp, 8 checksum)
//         Each byte starts with a ~50us LOW then a HIGH pulse. The duration of the
//         HIGH pulse determines the value of the bit.
//         LOW: 26-28us (<LOW duration)
//         HIGH: 70us (>LOW duration)
//     */

//     for (i = 0; i < 40; i++){
//         if (!wait_for_sensor(LOW, 1000))
//             return 0;
//         marks[i] = micros();
//         if (!wait_for_sensor(HIGH, 1000))
//             return 0;
//         stops[i] = micros();
//     }
//     if (!wait_for_sensor(LOW, 1000))
//         return 0;
//     marks[40] = micros();

    
//     for (i = 0; i < 40; i++)
//     {
//         lowTime = stops[i] - marks[i];
//         highTime = marks[i + 1] - stops[i];
//         if (highTime > lowTime)
//         {
//             dataBytes[i / 8] |= (1 << (7 - i % 8));
//         }
//     }

//     if (checksum(dataBytes[CHECKSUM], dataBytes, 4)){
//         humidity = ((uint16_t)dataBytes[HUMIDITY_H] << 8) | dataBytes[HUMIDITY_L];
//         // end of mod
//         return 1;
//     }
//     else
//         return 0;
// }

// _Bool wait_for_sensor(int pinState, uint16_t timeout){
//     uint16_t counter = 0;
//     while (((PIND & (1 << PD3)) != pinState) && (counter++ < timeout))
//         _delay_us(1);

//     if (counter >= timeout)
//         return false;
//     else
//         return true;
// }

// _Bool checksum(char check, char *data, unsigned int datalen){
//     char sum = 0;
//     int i;
//     for (i = 0; i < datalen; i++)
//     {
//         sum = sum + data[i];
//     }
//     if (sum == check)
//         return true;

//     return false;
// }

// unsigned long micros(){
//     unsigned long m;
//     uint8_t oldSREG = SREG, t;
     
//     m = timer0_overflow_count;
//     t = TCNT0;
 
//     if ((TIFR0 & _BV(TOV0)) && (t < 255))
//         m++;
 
//     SREG = oldSREG;
     
//     return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
// }
 
// ISR(TIMER0_OVF_vect)
// {
//     // copy these to local variables so they can be stored in registers
//     // (volatile variables must be read from memory on every access)
//     unsigned long m = timer0_millis;
//     unsigned char f = timer0_fract;
 
//     m += MILLIS_INC;
//     f += FRACT_INC;
//     if (f >= FRACT_MAX) {
//         f -= FRACT_MAX;
//         m += 1;
//     }
 
//     timer0_fract = f;
//     timer0_millis = m;
//     timer0_overflow_count++;
// }



// ISR(TIMER2_COMPA_vect){
//     us++; // Increment us every 8 microsecond
// }








/*	SparkFunRHT03.cpp
	Jim Lindblom <jim@sparkfun.com>
	August 31, 2015
    
    Ported to Arduino by Shawn Hymel
    October 28, 2016
	https://github.com/sparkfun/SparkFun_RHT03_Arduino_Library
	
	This is the main source file for the SparkFunRHT03 Arduino
	library.
	
	Development environment specifics:
	Arduino IDE v1.6.5
	Distributed as-is; no warranty is given. 
*/

// #include "SparkFun_RHT03.h"
#include "humidity.h"


#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))
 
// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
 
// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)
#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
// #define clockCyclesPerMicrosecond F_CPU * .000001
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )
 
volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;




void begin()
{
    _dataPin = (1 << PD3);
    DDRC &= ~_dataPin;
    PORTD |= _dataPin;
    // pinMode(_dataPin, INPUT_PULLUP);
}


int get_humidity()
{
    return _humidity / 10;
}

int update()
{
    int i;
    const int LOW =0, HIGH =1;
    unsigned long marks[41] = {0};
    unsigned long stops[40] = {0};
    unsigned int highTime, lowTime;
    char dataBytes[5] = {0};

    // noInterrupts();
    cli();

    // Begin state: input HIGH
    DDRD &= ~(1 << PD3); // Input
    PORTD |= (1 << PD3); // pullup
    _delay_ms(100);
    // Start signal: host sets data low, waits 1 ms, then pulls back up, wait 20-40us
    DDRD |= (1 << PD3); // output
    PORTD &= ~(1 << PD3); // lwo
    _delay_ms(2); // Wait 1 ms minimum
    DDRD &= ~(1 << PD3); // Input
    PORTD |= (1 << PD3); // pullup
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
            return errorExit(-i);
        marks[i] = micros();
        if (!waitForRHT(HIGH, 1000))
            return errorExit(-i);
        stops[i] = micros();
    }
    if (!waitForRHT(LOW, 1000))
        return errorExit(-41);
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
    else
    {
        return -43;
    }
}

bool checksum(char check, char *data, unsigned int datalen)
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

int errorExit(int code)
{
    sei();
    return code;
}

bool waitForRHT(int pinState, unsigned int timeout)
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