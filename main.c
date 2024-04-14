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
#include "humidity.h"


// Volatile variables for interrupts
volatile int test_flag=0;
volatile int changed =0;

// NPK variables
volatile uint8_t rx_complete = false, check_npk = 0, fertilizer_complete = true; // NPK flags
volatile uint8_t j = 0, npk_counter = 0; // NPK interrupt counters
volatile unsigned char npk_buf[8] = {0}; // NPK RX buffer
volatile _Bool fertilizer = false; // Initialize fertilizer pump to off

// Grow light variables
volatile _Bool grow_light = false; // Initialize grow light to off

// Humidity and water variables
volatile _Bool water = false; // Initialize water pump to off
volatile uint8_t water_complete = true; // Water flags
volatile uint8_t humidity_counter = 0; // Humidity interrupt counter
volatile uint8_t check_humidity = 0; // Humidity flag


///////////////  End volatile variables

// typedef struct{ // Structure to hold pumps array
//     _Bool *wat_fert_needed;
//     uint8_t num_pumps;
// } user_struct;
 
// Set by user via rotary encoder
enum PLANT_NEEDS{
    TROPICAL,  // Humid, sunny
    DESERT,    // Dry, sunny
    TEMPERATE, // Humid, low light 
    ALPINE     // Dry, moderate light
};

enum LIGHT_LEVELS{LOW, MODERATE,HIGH};
enum WATER_LEVELS{DRY,HUMID};

int main(void) {    
    sei(); // Enable Global Interrupts
    
    // Initialize registers, ports, and sensors
    init_encoder();
    init_timer();
    init_reg();
    init_npk();
    // init_humidity();
    begin();
    // End initialization

    uint8_t water_needs, light_needs;
    uint64_t humidity = 0;
    enum REGOUT led_select1, led_select2; // Declaration w/o initialization leaves LEDs in previous position on restart
    // _Bool wat_fert_needed[NUM_PUMPS] = {false,false};
    // pumps_struct pump_states = {.wat_fert_needed = {false,false}, .num_pumps = 2}; // Assign pointer to initial pump states and number of pumps

    DDRC |= (1 << PD0);
    

    while (1){
        if(encoder_changed) { // Set plant needs based on user input
            encoder_changed = false;    
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

            // led_select1 = GREEN1;
            // led_select2 = GREEN2;
            // sendOutput(led_select1, led_select2, water, fertilizer); // this won't stay here, just for testing encoder & shift reg
        }

        if (changed){
            changed = 0;
            // led_select1 = GREEN1;
            // led_select2 = RED2;
            // sendOutput(led_select1, led_select2, water, fertilizer);
        }
                // led_select1 = GREEN1;
                // led_select2 = GREEN2;
                // sendOutput(led_select1, led_select2, water, fertilizer);

        if(check_humidity){
            check_humidity = false;
            // cli();
            update;

            humidity = get_humidity();
            // sei();

            // if (humidity > 10000){
            //     led_select1 = GREEN1;
            //     led_select2 = GREEN2;
            //     sendOutput(led_select1, led_select2, water, fertilizer);
            // }
            if ((humidity > 50)){
                led_select1 = RED1;
                led_select2 = RED2;
                sendOutput(led_select1, led_select2, water, fertilizer);
            }
            else if ((humidity >0) &&(humidity <= 50)){
                led_select1 = YELLOW1;
                led_select2 = YELLOW2;
                sendOutput(led_select1, led_select2, water, fertilizer);
            }
            else if (humidity == 0){
                led_select1 = RED1;
                led_select2 = GREEN2;
                // led_select1 = led_select1 + 1;
                // led_select2 = led_select2 + 1;
                // if (led_select1 > GREEN1)
                //     led_select1 = RED1;
                // if (led_select2 > GREEN2)
                //     led_select2 = RED2;
                sendOutput(led_select1, led_select2, water, fertilizer);
            }
            else{
                led_select1 = GREEN1;
                led_select2 = GREEN2;
                sendOutput(led_select1, led_select2, water, fertilizer);
            }
            // if (humidity == 2){
            //     led_select1 = YELLOW1;
            //         led_select2 = YELLOW2;
            //         sendOutput(led_select1, led_select2, water, fertilizer);
            // }
            // int k, m = 0;
            // for (k = humidity/10; k < humidity; k+10){
            //     // if (m%2 == 0)
            //     //     PORTC &= ~(1<<PC0);
            //     // if (m%2 == 1)
            //     //     PORTC |= (1 << PC0);
            //     m++;
            //     _delay_ms(300);
            //     if (humidity > 100){
            //         led_select1 = GREEN1;
            //         led_select2 = RED2;
            //         sendOutput(led_select1, led_select2, water, fertilizer);
            //     }  
            // } 
        }

        if (check_npk){
            check_npk = false;
            get_npk(); // Request NPK values from soil sensor
        }

        if (fertilizer_complete){
            fertilizer_complete = false;
            sendOutput(led_select1, led_select2, water, fertilizer = false); // Turn fertilizer pump off
        }

        if (water_complete){
            water_complete = false;
            sendOutput(led_select1, led_select2, water = false, fertilizer); // Turn water pump off
        }


        if (rx_complete){               // Evaluate NPK values here
            rx_complete = false;        // Lower flag to allow another data rx
            j = 0;                      // reset j to allow another data rx
            fertilizer = fertilizer_needed(npk_buf[3], npk_buf[4], npk_buf[5]); // Evaluate NPK levels, determine if fertilizer is needed
            
        }
    }
    return 0;   /* never reached */
}

ISR(PCINT1_vect) //Interrupt vector for PORTC
{
    //Begin code for Rotary Encoder related Interrupt Handling
    encoder_input = PINC & (ENCODERA|ENCODERB);    //Read inputs simultaneously
    encoder_A = encoder_input & ENCODERA;     //Isolate EncoderA input
    encoder_B = encoder_input & ENCODERB;     //Isolate EncoderB input
    if (encoder_old_state == TROPICAL) {
        if(encoder_A){
            //Clockwise Rotation
            encoder_new_state = DESERT;
        }else if(encoder_B){
            //Counter-Clockwise Rotation
            encoder_new_state = TEMPERATE;
        }
	}
	else if (encoder_old_state == DESERT) {
        if(encoder_B){
            //Clockwise Rotation
            encoder_new_state = ALPINE;
        }else if(!encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = TROPICAL;
        }
	}
	else if (encoder_old_state == TEMPERATE) {
        if(!encoder_B){
            //Clockwise Rotation
            encoder_new_state = TROPICAL;
        }else if(encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = ALPINE;
        }
	}
	else {   // old_state = ALPINE
        if(!encoder_A){
            //Clockwise Rotation
            encoder_new_state = TEMPERATE;
        }else if(!encoder_B){
            //Counter-Clockwise Rotation
            encoder_old_state = DESERT;
        }
	}

    //If state has changed, change oldstate to newstate and set changed flag to report that the encoder was turned.
	if (encoder_new_state != encoder_old_state) {
	    encoder_changed = 1;
	    encoder_old_state = encoder_new_state;
	}
    //End code for Rotary Encoder related Interrupt Handling
} 

ISR (TIMER1_COMPA_vect) {

    npk_counter++; 
    humidity_counter++;

    if (npk_counter == 1){ // Counter value * 2 second timer = time interval to check NPK
        check_npk = true;
        npk_counter = 0;
    }

    if (humidity_counter == 2){
        check_humidity = true;
        humidity_counter = 0;
    }

    if (fertilizer){
        fertilizer = false; // Turn off fertilizer pump after returning from interrupt
        fertilizer_complete = true; 
    }

    if (water){
        water = false; // Turn off water pump after returning from interrupt
        water_complete = true;
    }

    if (test_flag){
        PORTC &= ~(1<<PC0);
        test_flag=0;
    }
    else{
        PORTC |= 1<<PC0;
        test_flag=1;
    }


    /*
    This interrupt is empty for now but has been tested. Currently interrupting every 5 seconds.
    This timer will eventually do:
        Check water levels & fertilizer levels
        Update water level & fertilizer level variables
        Check humidity in air & moisture in soil
        Update humidity & moisture variables
        Check NPK in soil
        Update NPK variable(s)
        Check sunlight
        update sun variable
    After returning from this timer, the main function will loop over:
        calculateSoilNeeds() to determine if water or fertilizer is needed, updating water_on and fertilizer_on as needed
        calculateSunNeeds() to determine if grow lamp is needed
        sendOutput() to update LEDs, turn water/fertilizer on
        growLamp() to turn on/off grow lamp
    Possibly different values can be used for timer. Maybe check water levels every 1 minute and check sun every 30 seconds? etc.
    */

}


ISR(USART_RX_vect) // Interrupt when a byte enters the UDR0 register
{
    char ch;
         
    ch = UDR0; // read the received character from the register
    npk_buf[j] = ch; // assign the character to an index
    j++;            // increment j to scroll through intBuf array

    if (j == 8){   
        rx_complete = 1; // raise flag that data receive is complete
    }
}

ISR(PCINT2_vect){
    changed = 1;
}