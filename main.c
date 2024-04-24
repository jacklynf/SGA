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
#include <stdio.h>

#include "shift_register_control.h"
#include "encoder.h"
#include "interrupt_init.h"
#include "npk.h"
#include "humidity.h"
#include "soil_moisture.h"
#include "i2c.h"
#include "light_sensor.h"
#include "LCD.h"
#include "LCD_GFX.h"
#include "i2c_mux.h"
#include "WaterLevel.h"

void compute_needs();

// How often to check sensors. 
// Timer interrupts every 2 seconds, so all count values are multiplied by 2 seconds.
#define   MOISTURE_COUNT 2
#define   HUMIDITY_COUNT 3
#define        NPK_COUNT 2
#define      LIGHT_COUNT 2
#define        MUX_COUNT 2
#define      WATER_COUNT 1
#define FERTILIZER_COUNT 3

#define BDIV (F_CPU / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz

// Volatile variables for interrupts

// NPK variables
volatile uint8_t rx_complete = false, check_npk = 0, fertilizer_complete = true; // NPK flags
volatile uint8_t j = 0, npk_counter = 0; // NPK interrupt counters
volatile unsigned char npk_buf[8] = {0}; // NPK RX buffer
volatile _Bool f_pump_on = false; // Initialize fertilizer pump to off

// Grow light variables
volatile uint8_t check_light = 0; // Light sensor flag
volatile uint8_t lightsensor_counter = 0; // Light sensor interrupt counter

// Humidity variables
volatile _Bool w_pump_on = false; // Initialize water pump to off
volatile uint8_t water_complete = true; // Water flags
volatile uint8_t humidity_counter = 0; // Humidity interrupt counter
volatile uint8_t check_humidity = 0; // Humidity flag

// Soil moisture variables
volatile uint8_t check_moisture = 0; // Soil moisture flag
volatile uint8_t moisture_counter = 0; // Soil moisture interrupt counter
volatile unsigned char moisture = 0;

volatile uint8_t check_water = 0;
volatile uint8_t check_fert = 0;
volatile uint8_t water_counter = 0, fertilizer_counter = 0;

volatile uint8_t test_flag = 0;
///////////////  End volatile variables

uint16_t lum, humidity = 0, water_light;
uint8_t water_needs, light_needs, dev_id, moist_threshold, humidity_threshold, light_threshold;

void compute_needs(){ // Determine watering & grow light needs based on user input
    if (water_needs == HUMID){
        moist_threshold = 200;
        humidity_threshold = 25;
    }
    else if (water_needs == DRY){
        moist_threshold = 100;
        humidity_threshold = 10;
    }

    if (light_needs == HIGH){
        light_threshold = 400;
    }
    else if (light_needs == MODERATE){
        light_threshold = 200;
    }
    else if (light_needs == LOW){
        light_threshold = 75;
    }
}


int main(void) {    
    DDRC |= 1 << DDC0;
    
    sei(); // Enable Global Interrupts
    
    // Initialize i2c, registers, ports, and sensors    
    i2c_init(BDIV);
    uint8_t encoder = init_encoder();
    init_timer();
    init_reg();
    init_npk();
    init_humidity();
    init_mux();
    init_soilmoisture();
    LCD_Initialize();
    setRotation(3);
    fillScreen(ILI9341_DARKGREEN);
    // fillRect(250, 30, 30, 50, ILI9341_R  ED);
    // drawLine(100, 100, 200, 200, ILI9341_ORANGE);

    // End initialization

    _Bool grow_light = false;
    enum REGOUT led_select1, led_select2; // Declaration w/o initialization leaves LEDs in previous position on restart
    led_select1 = GREEN1, led_select2 = GREEN2;
    sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light);
   
    // Startup screen
    setCursor(25,75);
    setTextColor(ILI9341_ORANGE);
    setTextSize(3);
    printString("Smart Gardening");
    setCursor(75,125);
    printString("Assistant");
    _delay_ms(2000); // Pause on start screen
    fillScreen(ILI9341_DARKGREEN);
    // End of startup screen


   // Begin i2c communication with light sensor
    if ((dev_id = init_lightsensor()) == 0x50){
        if (!enable_lightsensor()){
            if (!configure_lightsensor()){
                setCursor(55,50);
                printString("All sensors");
                setCursor(90,100);
                printString("enabled.");
            }
        }
    }
    // End light sensor configuration
    _delay_ms(2000); // Pause on sensor screen
    fillScreen(ILI9341_DARKGREEN); 
    init_base_screen(encoder);

    uint16_t counter = 0;
    int water_lev = -1;
    int fert_lev = -1;
    char buf[16];
    while (1){        
        if (check_fert){
            check_fert = 0;
            fert_lev = checkWaterLevel(TCA_CHANNEL_2);
            ud_lcd_liquids(water_lev, NULL);
            if ((fert_lev < 50) && (fert_lev >= 25))
                sendOutput(led_select1 = YELLOW1, led_select2, w_pump_on, f_pump_on, grow_light);
            else if (fert_lev < 25)
                sendOutput(led_select1 = RED1, led_select2, w_pump_on, f_pump_on, grow_light);
            else 
                sendOutput(led_select1 = GREEN1, led_select2, w_pump_on, f_pump_on, grow_light);
        }

        if (check_water){
            check_water = 0;
            water_lev = checkWaterLevel(TCA_CHANNEL_0);
            ud_lcd_liquids(NULL, fert_lev);
            if ((water_lev < 50) && (water_lev >= 25))
                sendOutput(led_select1 = YELLOW2, led_select2, w_pump_on, f_pump_on, grow_light);
            else if (water_lev < 25)
                sendOutput(led_select1 = RED2, led_select2, w_pump_on, f_pump_on, grow_light);
            else 
                sendOutput(led_select1 = GREEN2, led_select2, w_pump_on, f_pump_on, grow_light);
        }

        if (check_light){ // Check if grow light is needed
            check_light = false;
            lum = get_luminosity();
            ud_lcd_light(lum);
            if (lum < light_threshold) // Adjust this value based on user settings
                sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light = true); // Turn grow light on if light is low 
            else           
                sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light = false); // Turn grow light on if light is low
        }

        if(encoder_changed) { // Set plant needs based on user input
            encoder_changed = false;  
            water_light = user_input(encoder_new_state); // Water needs in upper 8 bits, light needs in lower 8 bits
            water_needs = (water_light >> 8), light_needs = water_light; // values defined in encoder ENUM
            compute_needs();
            ud_lcd_encoder(encoder_new_state);
        }

        if (check_moisture && check_humidity){ // Check if plant needs water
            check_moisture = false;
            check_humidity = false;
            moisture = adc_sample(1); // PC1 is channel 1 in ADC mux
            ud_lcd_moisture(moisture);
            
            if (update_humidity()){
                _delay_us(1);
                humidity = get_humidity(); 
                ud_lcd_humidity(humidity);                
            }   

            if ((moisture < moist_threshold)&&(humidity < humidity_threshold)){ // Adjust thresholds based on plant settings
                sendOutput(led_select1, led_select2, w_pump_on = true, f_pump_on, grow_light); // Turn on water pump
            }
        }
        
        if (check_npk){
            check_npk = false;
            get_npk(); // Request NPK values from soil sensor            
        }

        if (fertilizer_complete){
            fertilizer_complete = false;
            sendOutput(led_select1, led_select2, w_pump_on, f_pump_on = false, grow_light); // Turn fertilizer pump off
        }

        if (water_complete){
            water_complete = false;
            sendOutput(led_select1, led_select2, w_pump_on = false, f_pump_on, grow_light); // Turn water pump off
        }


        if (rx_complete){               // Evaluate NPK values here 
            rx_complete = false;        // Lower flag to allow another data rx
            j = 0;                      // reset j to allow another data rx
            ud_lcd_npk(npk_buf[3], npk_buf[4],npk_buf[5]);
            sendOutput(led_select1, led_select2, w_pump_on, 
                       f_pump_on = fertilizer_needed(npk_buf[3], npk_buf[4], npk_buf[5]), grow_light); // Turn fertilizer on if returned true
        }
    }
    return 0;   /* never reached */
}

ISR(PCINT1_vect) //Interrupt vector for PORTC
{
    //Begin code for Rotary Encoder related Interrupt Handling
    encoder_input = PINC & (ENCODERA|ENCODERB);    //Read inputs simultaneously
    encoder_A = encoder_input & ENCODERA;          //Isolate EncoderA input
    encoder_B = encoder_input & ENCODERB;          //Isolate EncoderB input
    if (encoder_old_state == 0) {
        if(encoder_A){
            //Clockwise Rotation
            encoder_new_state = 1;
        }else if(encoder_B){
            //Counter-Clockwise Rotation
            encoder_new_state = 2;
        }
	}
	else if (encoder_old_state == 1) {
        if(encoder_B){
            //Clockwise Rotation
            encoder_new_state = 3;
        }else if(!encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = 0;
        }
	}
	else if (encoder_old_state == 2) {
        if(!encoder_B){
            //Clockwise Rotation
            encoder_new_state = 0;
        }else if(encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = 3;
        }
	}
	else {   // old_state = ALPINE
        if(!encoder_A){
            //Clockwise Rotation
            encoder_new_state = 2;
        }else if(!encoder_B){
            //Counter-Clockwise Rotation
            encoder_old_state = 1;
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
    water_counter++;
    fertilizer_counter++;

    if (water_counter == WATER_COUNT){
        check_water=true;
        water_counter = 0;
    }

    if (fertilizer_counter == FERTILIZER_COUNT){
        check_fert = true;
        fertilizer_counter=0;
    }

    if (lightsensor_counter == LIGHT_COUNT){ // Check light conditions
        check_light = true;
        lightsensor_counter = 0;
    }

    if (moisture_counter == MOISTURE_COUNT){ // Check soil moisture
        check_moisture = true;
        moisture_counter = 0;
    }

    if (npk_counter == NPK_COUNT){ // Check NPK levels
        check_npk = true;
        npk_counter = 0;
    }

    if (humidity_counter == HUMIDITY_COUNT){ // Check humidity
        check_humidity = true;
        humidity_counter = 0;
    }

    if (f_pump_on){ // If fertilizer pump is on, this will turn it off after 2 seconds
        f_pump_on = false; 
        fertilizer_complete = true; 
    }

    if (w_pump_on){ // If water pump is on, this will turn it off after 2 seconds
        w_pump_on = false; 
        water_complete = true;
    }

    if (test_flag){
        PORTC |= 1 << PC0;      // Set PC0 to a 1
        test_flag = 0;
    }
    else{
        PORTC &= ~(1 << PC0);
        test_flag = 1;
    }

}


ISR(USART_RX_vect) // Interrupt when a byte enters the UDR0 register
{
    char ch;

    ch = UDR0;      // read the received character from the register
    npk_buf[j] = ch; // assign the character to an index
    j++;            // increment j to iterate through npk_buf array


    if (j == 8){   
        rx_complete = 1; // raise flag that data receive is complete
    }
}