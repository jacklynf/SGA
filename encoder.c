#include <avr/io.h>
#include <avr/interrupt.h>
#include "encoder.h"

void init_encoder(){
    DDRC   &= ~(ENCODERA|ENCODERB); //Set EncoderA and EncoderB pins as inputs (constants defined in encoder.h)
    PORTC  |=  (ENCODERA|ENCODERB); //Enable Pull-Up resistor on EncoderA and EncoderB
    PCICR  |=  (1 << PCIE1); //Enable Pin Change interrupts on PORTC
    PCMSK1 |=  (1 << PCINT10)|(1 << PCINT11); // set mask bits
    //PCMSK1 register will be controled based on when we have selected an item to be edited, and therefore want the rotary encoder to be enabled
    //(so it isn't triggering interrupts when it isn't being utilized)
}

void encoderEnable(){
    PCMSK1 |= (1 << ENCODERA) + (1 << ENCODERB);
}
void encoderDisable(){
    PCMSK1 &= ~((1 << ENCODERA) + (1 << ENCODERB));
}

