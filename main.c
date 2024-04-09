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
#include "interrupt_init.h"
#include "npk.h"

volatile enum REGOUT led_select1 = GREEN1, led_select2 = GREEN2;  
volatile _Bool water_on = false, fertilizer_on = true;
volatile uint8_t encoder_oldState, encoder_newState; //Varibles for rotary encoder state machine
volatile uint8_t encoder_changed = 0; //Rotary encoder changed flag
volatile uint8_t encoderInput, encoderA, encoderB; //rotary encoder inputs variables

int main(void) {
    init_encoder();
    init_timer();
    init_reg();

    // Rotary encoder code begin
    // Intialize rotary encoder state machine:
    encoderInput = PINC & (ENCODERA|ENCODERB);    //Read PINC
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

    while (1){
        sei(); //Enable Global Interrupts
        
        //Rotary Encoder code:
        //if encoder changed flag is set
        if(encoder_changed) {
            encoder_changed = 0;    //reset the flag
            sendOutput(led_select1, led_select2, water_on, fertilizer_on); // this won't stay here, just for testing encoder & shift reg
            //do other stuff needed (to be added)...
        }

    }
    return 0;   /* never reached */
}

ISR(PCINT1_vect) //Interrupt vector for PORTC
{
    //Begin code for Rotary Encoder related Interrupt Handling
    encoderInput = PINC & (ENCODERA|ENCODERB);    //Read inputs simultaneously
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

ISR (TIMER1_OVF_vect) {
    // Dummy code for now, for now using timer to select LEDs based on rotary input
    // This timer will eventually do:
        // Check water levels & fertilizer levels
        // Update water level & fertilizer level variables
        // Check humidity in air & moisture in soil
        // Update humidity & moisture variables
        // Check NPK in soil
        // Update NPK variable(s)
        // Check sunlight
        // update sun variable
    // After returning from this timer, the main function will loop over:
        // calculateSoilNeeds() to determine if water or fertilizer is needed, updating water_on and fertilizer_on as needed
        // calculateSunNeeds() to determine if grow lamp is needed
        // sendOutput() to update LEDs, turn water/fertilizer on
        // growLamp() to turn on/off grow lamp
    // Possibly different values can be used for timer. Maybe check water levels every 1 minute and check sun every 30 seconds? etc.

    if (encoder_newState == 1){
        led_select1 = GREEN1;
        led_select2 = GREEN2;
    }
    else if(encoder_newState == 2){
        led_select1 = YELLOW1;
        led_select2 = YELLOW2;
    }
    else if(encoder_newState == 3){
        led_select1 = RED1;
        led_select2 = RED2;
    }
    // else{
    //     led_select1 = led_select1 + 1;
    //     if (led_select1 > GREEN1)
    //         led_select1=RED1;
    //     led_select2 = led_select2 + 1;
    //     if (led_select2 > GREEN2)
    //         led_select2=RED2;
    //     }
    TCNT1 = 63616; // 200 ms for 9.83MHz clock
}
