#include <avr/io.h>
#include <avr/interrupt.h>
#include "interrupt_init.h"

void init_timer(){ // Set up timer to check water & fertilizer levels
    /*
    To calculate timer value:
        freq/prescalar = 9830400Hz/1024 = 9600Hz
        tick time = 1/9600Hz = 104.2us
        16 bit timer = 65536
        To calculate upper timer value, input desired wait time, t: 65536 - (t/104.2us)
        TCNT1 = 65536 - (t/(1/9600)) 
        Ex: for t = 200ms, TCNT1 = 65536 - (0.2 / (1/9600)) = 63616
    */

    TCNT1 = 63616; // 200ms for 9.83MHz clock
    TCCR1A = 0x00; // Set normal counter mode
    TCCR1B = (1<<CS10) | (1<<CS12); // Set 1024 pre-scaler
    TIMSK1 = (1 << TOIE1); // Set overflow interrupt enable bit
}