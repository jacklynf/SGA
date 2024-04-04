#include <avr/io.h>
#include <util/delay.h>
#include "shift_register_control.h"
#include <stdbool.h>

void sendOutput(uint16_t* outputs){ 
    for(int i = 0; i < 15; i++){ //Iterate 16 times, one for each of the 16 bits - starting with the LSB
        if(*outputs & (1 << i)){ //Check to see if the current bit is 1 or 0
            PORTD |= SERIAL_DATA_OUT; //If 1, set serial data out pin to HIGH
        }else{
            PORTD &= ~SERIAL_DATA_OUT; //If 0, set serial data out pin to LOW
        }
        //After Serial data out pin has been appropriately set, pulse the Serial Data Clock HIGH, then LOW
        PORTD |= SERIAL_CLK;
        PORTD &= ~SERIAL_CLK;
    }
    //After all 16 bit have been shifted into the register, Pulse the Store Clock HIGH, then LOW, to shift the bits into the output registers
    PORTD |= STORE_CLK;
    PORTD &= ~STORE_CLK;
}

void toggleOutput (_Bool state, uint16_t output, uint16_t* outputs){
    if(state){
        *outputs |= output;
    }else{
        *outputs &= ~output;
    }
}

