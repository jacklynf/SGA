#include <avr/io.h>
#include <util/delay.h>
#include "shift_register_control.h"
#include <stdbool.h>

void init_reg(){
    //Set DDR for Shift Registers (constants defined in shift_register_control.h)
    DDRD |= SERIAL_DATA_OUT;
    DDRD |= SERIAL_CLK;
    DDRD |= STORE_CLK;
    DDRB |= SHIFT_REG_OE; //Enable outputs on Shift Registers

    PORTB &= ~SHIFT_REG_OE; // Enable shift registers (active low)
    // End shift register init
}

void sendOutput(enum REGOUT led_select1, enum REGOUT led_select2, 
                _Bool water_on, _Bool fertilizer_on, _Bool light_on){
    int i; 
    enum REGOUT water_pump = WATER, fertilizer_pump = FERTILIZER, grow_light = GROW_LIGHT;

    for(i = 0; i < 16; i++){ 
        if ((i == led_select1) || (i == led_select2) || // Select which 2 LEDs will be on
            ((i == water_pump) && (water_on == true)) || // Turn on water
            ((i == fertilizer_pump) && (fertilizer_on == true)) ||
            ((i == GROW_LIGHT) && (light_on == true))) // Turn on fertilizer
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

