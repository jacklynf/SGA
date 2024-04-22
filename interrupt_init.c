#include <avr/io.h>
#include <avr/interrupt.h>
#include "interrupt_init.h"

void init_timer(){ // Set up timer to check sensors
    /*
    To calculate timer value:
        1. Clock freq/prescalar = 7372800Hz/1024 = 7200Hz
        2. Multiply (clock freq/prescalar) by desired wait time:  7200 * 2 second = 14400
        3. Set upper limit on timer to this value. Timer will interrupt at intervals of the desired wait time.
    */

    TCCR1B = (1<<CS10) | (1<<CS12); // Set 1024 prescaler for timer 1
    TCCR1B |= (1 << WGM12); // Initialize CTC (restart counting when desired value is reached)
    TIMSK1 |= (1 << OCIE1A); // Initialize output compare to generate a timer interupt
    

    // Using clock prescalar 1024: timer needs to count
    // to 18,000 to reach 2.5s with 7.3728 MHz clock 
    OCR1A = 18000; // 2.5 seconds
    // OCR1A = 36000;
}