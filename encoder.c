#include <avr/io.h>
#include <avr/interrupt.h>
#include "encoder.h"

void init_encoder(){
    DDRC   &= ~(ENCODERA|ENCODERB); //Set EncoderA and EncoderB pins as inputs (constants defined in encoder.h)
    PORTC  |=  (ENCODERA|ENCODERB); //Enable Pull-Up resistor on EncoderA and EncoderB
    PCICR  |=  (1 << PCIE1); //Enable Pin Change interrupts on PORTC
    PCMSK1 |=  (1 << PCINT10)|(1 << PCINT11); // set mask bits to track PC2 and PC3 pin changes

    encoder_changed = 0; // initialize flag

    // Intialize rotary encoder state machine:
    encoder_input = PINC & (ENCODERA|ENCODERB);    //Read PINC
    encoder_A = encoder_input & ENCODERA;     //Isolate EncoderA input
    encoder_B = encoder_input & ENCODERB;     //Isolate EncoderB input

    if (!encoder_B && !encoder_A)         //If encoder input is 00
	    encoder_old_state = 0;
    else if (!encoder_B && encoder_A)     //If encoder input is 01
	    encoder_old_state = 1;
    else if (encoder_B && !encoder_A)     //If encoder input is 10
	    encoder_old_state = 2;
    else                                //If encoder input is 11
	    encoder_old_state = 3;

    encoder_old_state = encoder_new_state;
}

uint16_t user_input(uint8_t encoder_new_state){
    uint8_t water_needs, light_needs;
    uint16_t needs;
    switch(encoder_new_state){
        case TROPICAL:
            water_needs = HUMID;
            light_needs = HIGH;
            break;
        case DESERT:
            water_needs = DRY;
            light_needs = HIGH;
            break;
        case TEMPERATE:
            water_needs = HUMID;
            light_needs = LOW;
            break;
        case ALPINE:
            water_needs = DRY;
            light_needs = MODERATE;
            break;
        default:
            water_needs = HUMID;
            light_needs = HIGH;
            break;
    }

    needs = (water_needs << 8) | light_needs;
    return needs;
}

void encoderEnable(){
    PCMSK1 |= (1 << ENCODERA) + (1 << ENCODERB);
}
void encoderDisable(){
    PCMSK1 &= ~((1 << ENCODERA) + (1 << ENCODERB));
}

