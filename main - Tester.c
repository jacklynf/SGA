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
#include "LCD.h"
#include "LCD_GFX.h"
#include "i2c.h"
#include "WaterLevel.h"
#include "light_sensor.h"
#include "npk.h"
#include "soil_moisture.h"
#include "humidity.h"
#include "encoder.h"

#define BDIV (F_CPU / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz


volatile _Bool f_pump_on = false; // Initialize fertilizer pump to off
volatile _Bool w_pump_on = false; // Initialize water pump to off
_Bool grow_light = false;

uint8_t Water_Level = 0;
uint8_t Fertilizer_Level = 0;
uint32_t Lux_Level = 0;
uint16_t IR = 0;
uint16_t Visible = 0;
uint8_t Soil_Moisture = 0;

uint16_t Temperature = 0;
uint16_t Humidity = 0;


// NPK variables
volatile uint8_t rx_complete = false, check_npk = 0, fertilizer_complete = true; // NPK flags
volatile uint8_t j = 0, npk_counter = 0; // NPK interrupt counters
volatile uint8_t npk_buf[8] = {0}; // NPK RX buffer
volatile int16_t n = 0;
volatile int16_t sesitivity = 25;
volatile int16_t count = 0; 

char str[12];           // Array to hold the resulting string

int main(void) {    
    DDRC |= 1 << PC0;
    
    sei(); // Enable Global Interrupts

    init_reg();
    i2c_init(BDIV);
    init_soilmoisture();
    init_humidity();
    init_encoder();
    init_npk();




    LCD_Initialize();
    // End initialization


    setRotation(3);
    fillScreen(ILI9341_DARKGREEN);

    setTextSize(3);
    setTextanBGColor(ILI9341_RED, ILI9341_DARKGREEN);
    setCursor(20, 20);

    printString("TEST");
    _delay_ms(1000);

    fillScreen(ILI9341_DARKGREEN);
    
    // setCursor(20, 20);
    // printString("Initializing Light Sensor");
    // _delay_ms(1000);

    // setCursor(20, 20);
    // fillScreen(ILI9341_DARKGREEN);
    // if(init_lightsensor()) {
    //     printString("Light Sensor Error");
    // } else {
    //     printString("Light Sensor Success");
    // }
    // _delay_ms(1000);



    // fillScreen(ILI9341_DARKGREEN);
    // setCursor(120,20);
    // sprintf(str, "%u", Water_Level);
    // printString(str);

    // setCursor(120,50);
    // sprintf(str, "%u", Fertilizer_Level);
    // printString(str);

    // setCursor(50,20);
    // sprintf(str, "%u", IR);
    // printString(str);

    // setCursor(50,80);
    // sprintf(str, "%u", Visible);
    // printString(str);

    // setCursor(120, 120);
    // sprintf(str, "%u", Soil_Moisture);
    // printString(str);

    // enum REGOUT led_select1, led_select2; // Declaration w/o initialization leaves LEDs in previous position on restart

    setCursor(120,120);
    itoa(n, str, 10);
    printString(str);
   
    while (1){

        if(encoder_changed){
            encoder_changed = 0;
            if(abs(n) >= sensitivity){
                if(n < 0){
                    count--;
                }else{
                    count++;
                }
            }
        }
    // led_select1 = GREEN1, led_select2 = GREEN2;
    // sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light);
    
    // led_select1 = YELLOW1, led_select2 = YELLOW2;
    // _delay_ms(1000);

    // sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light);
    
    // led_select1 = RED1, led_select2 = RED2;
    // _delay_ms(1000);
    
    // sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light);


    // Water_Level = checkWaterLevel(WATER_LEVEL_CHANNEL);
    // Fertilizer_Level = checkWaterLevel(FERTILIZER_CHANNEL);
    
    // fillScreen(ILI9341_DARKGREEN);

    // setCursor(120,20);
    // sprintf(str, "%u", Water_Level);
    // printString(str);

    // setCursor(120,50);
    // sprintf(str, "%u", Fertilizer_Level);
    // printString(str);

    // Lux_Level = get_luminosity();
    // IR = (Lux_Level >> 16);
    // Visible = (Lux_Level & 0xFFFF) - (Lux_Level >> 16);

    // setCursor(50,20);
    // sprintf(str, "%u", IR);
    // printString(str);

    // setCursor(50,80);
    // sprintf(str, "%u", Visible);
    // printString(str);

    // Soil_Moisture = adc_sample(1);

    // setCursor(120, 120);
    // sprintf(str, "%u", Soil_Moisture);
    // printString(str);

    //uint8_t temp = update_humidity();
    // if(!update_humidity()){
    //     setCursor(50,50);
    //     printString("Error");
    // }
    // Humidity = get_humidity();
    // Temperature = get_temperature();


    // setCursor(120, 180);
    // itoa(Humidity, str, 10);
    // printString(str);

    // setCursor(120, 80);
    // itoa(Temperature, str, 10);
    // printString(str);
    
    _delay_ms(500);

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
            n++;
        }else if(encoder_B){
            //Counter-Clockwise Rotation
            encoder_new_state = 2;
            n--;       
        }
	}
	else if (encoder_old_state == 1) {
        if(encoder_B){
            //Clockwise Rotation
            encoder_new_state = 3;
            n++;
        }else if(!encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = 0;
            n--;
        }
	}
	else if (encoder_old_state == 2) {
        if(!encoder_B){
            //Clockwise Rotation
            encoder_new_state = 0;
            n++;
        }else if(encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = 3;
            n--;
        }
	}
	else {   // old_state = ALPINE
        if(!encoder_A){
            //Clockwise Rotation
            encoder_new_state = 2;
            n++;
        }else if(!encoder_B){
            //Counter-Clockwise Rotation
            encoder_old_state = 1;
            n--;
        }
	}

    //If state has changed, change oldstate to newstate and set changed flag to report that the encoder was turned.
	if (encoder_new_state != encoder_old_state) {
	    encoder_changed = 1;
	    encoder_old_state = encoder_new_state;
	}
    //End code for Rotary Encoder related Interrupt Handling
} 
