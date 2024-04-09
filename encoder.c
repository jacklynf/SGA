#include <avr/io.h>
#include <avr/interrupt.h>
#include "encoder.h"

void encoderEnable(){
    PCMSK1 |= (1 << ENCODERA) + (1 << ENCODERB);
}
void encoderDisable(){
    PCMSK1 &= ~((1 << ENCODERA) + (1 << ENCODERB));
}
