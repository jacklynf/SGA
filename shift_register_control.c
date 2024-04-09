#include <avr/io.h>
#include <util/delay.h>
#include "shift_register_control.h"
#include <stdbool.h>

void sendOutput(enum REGOUT led_select1, enum REGOUT led_select2, 
                _Bool water_on, _Bool fertilizer_on){
    int i; 
    enum REGOUT water_pump = WATER, fertilizer_pump = FERTILIZER;

    for(i = 0; i < 8; i++){ 
        if ((i == led_select1) || (i == led_select2) || // Select which 2 LEDs will be on
            ((i == water_pump) && (water_on == true)) || // Turn on water
            ((i == fertilizer_pump) && (fertilizer_on == true))) // Turn on fertilizer
            PORTD |= SERIAL_DATA_OUT;
        else
            PORTD &= ~SERIAL_DATA_OUT;
        PORTD |= SERIAL_CLK;
        PORTD &= ~SERIAL_CLK;
    }

    //After all bit have been shifted into the register, Pulse the Store Clock HIGH, then LOW, to shift the bits into the output registers
    PORTD |= STORE_CLK;
    PORTD &= ~STORE_CLK;
}

// void toggleOutput (_Bool state, uint16_t output, uint16_t* outputs){
//     if(state){
//         *outputs |= output;
//     }else{
//         *outputs &= ~output;
//     }
// }

