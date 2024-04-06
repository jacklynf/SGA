/*************************************************************
*       at328-2.c - Demonstrate simple I/O functions of ATmega328P
*
*       This program will cause a 7-segment display to either count up in
*       hexadecimal (0,1,2,...,E,F,0,1,...) or count down in decimal
*       (9,8,...,1,0,9,8,..) depending on whether or not a switch is
*       pressed.
*
*       Port C, bit 1 - input from switch (0 = pressed, 1 = not pressed)
*               When the switch is not pressed, the 7-segment display
*               counts up in hexadecimal. When the switch is pressed,
*               the 7-segment display counts down in decimal.
*       Port B, bits 0-1 and Port D, bits 2-6 - Outputs to data inputs of
*               the 74LS374 register.
*               Bit 6 -> segment A, 5->B, ... , 1->F, 0->G
*               A low output bit will cause the LED segment to light up.
*       Port C, bit 2 - Output to positive edge-triggered clock input
*               of 74LS374 register.
*
* Revision History
* Date     Author      Description
* 01/03/05 M. Redekopp Initial Release for MC68HC908JL3
* 09/05/05 A. Weber    Modified for JL8 processor
* 01/13/06 A. Weber    Modified for CodeWarrior 5.0
* 08/25/06 A. Weber    Modified for JL16 processor
* 04/23/07 A. Weber    Split example 2 into 2a and 2b
* 08/17/07 A. Weber    Incorporated changes to demo board
* 04/22/08 A. Weber    Added "one" variable to make warning go away
* 04/23/08 A. Weber    Adjusted wait_100ms for 9.8304MHz clock
* 04/03/11 A. Weber    Adapted for ATmega168
* 06/06/13 A. Weber    Cleaned it up a bit
* 11/18/13 A. Weber    Renamed for ATmega328P
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/interrupt.h>


#include "shift_register_control.h"
#include "encoder.h"   

uint8_t encoder_oldState, encoder_newState; //Varibles for rotary encoder state machine
uint8_t encoder_changed; //Rotary encoder changed flag
uint8_t encoderInput, encoderA, encoderB; //rotary encoder inputs variables

int main(void) {
    sei(); //Enable Global Interrupts

    //Rotary Encoder Code Begin
    DDRC &= ~(ENCODERA + ENCODERB); //Set EncoderA and EncoderB pins as inputs (constants defined in encoder.h)
    PORTC |= (ENCODERA + ENCODERB); //Enable Pull-Up resistor on EncoderA and EncoderB
    PCICR |= (1 << PCIE1); //Enable Pin Change interrupts on PORTC
    //PCMSK1 register will be controled based on when we have selected an item to be edited, and therefore want the rotary encoder to be enabled
    //(so it isn't triggering interrupts when it isn't being utilized)
    
    encoder_changed = 0; //Set encoder changed flag to 0

    //Intialize rotary encoder state machine:
    encoderInput = PINC;    //Read PINC
    encoderA = encoderInput & ENCODERA;     //Isolate EncoderA input
    encoderB = encoderInput & ENCODERB;     //Isolate EncoderB input

    if (!encoderB && !encoderA)         //If encoder input is 00
	    encoder_oldState = 0;
    else if (!encoderB && encoderA)     //If encoder input is 01
	    encoder_oldState = 1;
    else if (encoderB && !encoderA)     //If encoder input is 10
	    encoder_oldState = 2;
    else                                //If encoder input is 11
	    encoder_oldState = 3;

    encoder_oldState = encoder_newState;
    //Rotary Encoder Code End



    //Shift Register Code Begin
    uint16_t shift_register_outputs = 0x0000; //16 bit varible to hold 16 bits to be loaded into shift register

    //Set DDR for Shift Registers (constants defined in shift_register_control.h)
    DDRD |= SERIAL_DATA_OUT;
    DDRD |= SERIAL_CLK;
    DDRD |= STORE_CLK;
    DDRD |= SHIFT_REG_OE;

    PORTD |= SHIFT_REG_OE; //Enable outputs on Shift Registers
    //Shift Register Code End


    while (1) {

        //Test Function Calls to test Shift Registers
        toggleOutput(true, GREEN_LED_1, &shift_register_outputs); //Test toggle output function; Set bit corresponding bit for GREEN_LED_1 to '1'    
        sendOutput(&shift_register_outputs); //Test send output function


        //Rotary Encoder code:
        //if encoder changed flag is set
        if(encoder_changed) {
            encoder_changed = 0;    //reset the flag

            //do other stuff needed (to be added)...
        }

    }
    
    return 0;   /* never reached */
}

ISR(PCINT1_vect) //Interrupt vector for PORTC
{
    //Begin code for Rotary Encoder related Interrupt Handling
    encoderInput = PINC;    //Read PINC
    encoderA = encoderInput & ENCODERA;     //Isolate EncoderA input
    encoderB = encoderInput & ENCODERB;     //Isolate EncoderB input
    if (encoder_oldState == 0) {
        if(encoderA){
            //Clockwise Rotation
            encoder_newState = 1;
        }else if(encoderB){
            //Counter-Clockwise Rotation
            encoder_newState = 2;
        }
	}
	else if (encoder_oldState == 1) {
        if(encoderB){
            //Clockwise Rotation
            encoder_newState = 3;
        }else if(!encoderA){
            //Counter-Clockwise Rotation
            encoder_newState = 0;
        }
	}
	else if (encoder_oldState == 2) {
        if(!encoderB){
            //Clockwise Rotation
            encoder_newState = 0;
        }else if(encoderA){
            //Counter-Clockwise Rotation
            encoder_newState = 3;
        }
	}
	else {   // old_state = 3
        if(!encoderA){
            //Clockwise Rotation
            encoder_newState = 2;
        }else if(!encoderB){
            //Counter-Clockwise Rotation
            encoder_oldState = 1;
        }
	}

    //If state has changed, change oldstate to newstate and set changed flag to report that the encoder was turned.
	if (encoder_newState != encoder_oldState) {
	    encoder_changed = 1;
	    encoder_oldState = encoder_newState;
	}
    //End code for Rotary Encoder related Interrupt Handling

} 

