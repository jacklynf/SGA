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
#include <stddef.h>

#include "shift_register_control.h"
#include "encoder.h"
#include "interrupt_init.h"
#include "npk.h"
#include "humidity.h"
#include "soil_moisture.h"
#include "i2c.h"
#include "light_sensor.h"

// How often to check sensors. 
// Timer interrupts every 2 seconds, so all count values are multiplied by 2 seconds.
#define MOISTURE_COUNT 2 
#define HUMIDITY_COUNT 2
#define      NPK_COUNT 2
#define    LIGHT_COUNT 1

#define BDIV (F_CPU / 300000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz

// Volatile variables for interrupts
volatile int test_flag=0;

// NPK variables
volatile uint8_t rx_complete = false, check_npk = 0, fertilizer_complete = true; // NPK flags
volatile uint8_t j = 0, npk_counter = 0; // NPK interrupt counters
volatile unsigned char npk_buf[8] = {0}; // NPK RX buffer
volatile _Bool fertilizer = false; // Initialize fertilizer pump to off

// Grow light variables
volatile _Bool light_on = false; // Initialize grow light to off
volatile uint8_t check_light = 0; // Light sensor flag
volatile uint8_t lightsensor_counter = 0; // Light sensor interrupt counter

// Humidity variables
volatile _Bool water = false; // Initialize water pump to off
volatile uint8_t water_complete = true; // Water flags
volatile uint8_t humidity_counter = 0; // Humidity interrupt counter
volatile uint8_t check_humidity = 0; // Humidity flag

// Soil moisture variables
volatile uint8_t check_moisture = 0; // Soil moisture flag
volatile uint8_t moisture_counter = 0; // Soil moisture interrupt counter
volatile unsigned char moisture = 0;


///////////////  End volatile variables


int main(void) {    
    sei(); // Enable Global Interrupts
    
    // Initialize i2c, registers, ports, and sensors    
    i2c_init(BDIV);
    init_encoder();
    init_timer();
    init_reg();
    init_npk();
    init_humidity();
    init_soilmoisture();
    // End initialization

    uint8_t i;
    uint16_t humidity = 0;
    uint8_t water_needs, light_needs;
    uint16_t water_light;

    enum REGOUT led_select1, led_select2; // Declaration w/o initialization leaves LEDs in previous position on restart
   
        led_select1 = GREEN1;
        led_select2 = RED2;
        sendOutput(led_select1, led_select2, water, fertilizer);
        _delay_ms(1000);


   // Begin i2c communication with light sensor
   // https://github.com/adafruit/Adafruit_TSL2591_Library/blob/master/Adafruit_TSL2591.cpp#L467 
    uint8_t dev_id = begin_lightsensor();
    if (dev_id == 0x50){ // Use LEDs to determine response when testing
        led_select1 = GREEN1;
        led_select2 = GREEN2;
        sendOutput(led_select1, led_select2, water, fertilizer);
    }
    else if (dev_id == 0x20){ // Use LEDs to determine response when testing
        led_select1 = RED1;
        led_select2 = RED2;
        sendOutput(led_select1, led_select2, water, fertilizer);
    }

    DDRC |= (1 << PD0);

    

    while (1){
        if (check_light){
            check_light = false;
        }

        if(encoder_changed) { // Set plant needs based on user input
            encoder_changed = false;
            water_light = user_input(encoder_new_state);
            water_needs = (water_light >> 8), light_needs = water_light;
        }

        if (check_moisture){
            check_moisture = false;
            moisture = adc_sample(1); // PC1 is channel 1 in ADC mux
        }

        if(check_humidity){
            check_humidity = false;
            if (update_humidity()){
                _delay_us(1);
                humidity = get_humidity();///10; 
            }
            // else 
            //     humidity = 999; // Else update did not work, no humidity value update

            // if (humidity == 0){
            //     led_select1 = RED1;
            //     led_select2 = RED1;
            //     sendOutput(led_select1, led_select2, water, fertilizer);
            // }
            // else if ((humidity > 0) &&(humidity <= 10)){
            //     led_select1 = RED1;
            //     led_select2 = RED2;
            //     sendOutput(led_select1, led_select2, water, fertilizer);
            // }
            // else if ((humidity > 10) && (humidity <= 50)){ 
            //     led_select1 = YELLOW1;
            //     led_select2 = GREEN2;
            //     sendOutput(led_select1, led_select2, water, fertilizer);
            // }            
            // else if ((humidity >50) && (humidity <= 100)){
            //     led_select1 = GREEN1;
            //     led_select2 = GREEN2;
            //     sendOutput(led_select1, led_select2, water, fertilizer);
            // }       
            // else{
            //     led_select1 = YELLOW1;
            //     led_select2 = YELLOW1;
            //     sendOutput(led_select1, led_select2, water, fertilizer);
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
    moisture_counter++;
    lightsensor_counter++;

    if (lightsensor_counter == LIGHT_COUNT){
        check_light = true;
        lightsensor_counter = 0;
    }

    if (moisture_counter == MOISTURE_COUNT){
        check_moisture = true;
        moisture_counter = 0;
    }

    if (npk_counter == NPK_COUNT){ // Counter value * 2 second timer = time interval to check NPK
        check_npk = true;
        npk_counter = 0;
    }

    if (humidity_counter == HUMIDITY_COUNT){
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

// ISR(PCINT2_vect){
//     changed = 1;
// }