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

#define BDIV (F_CPU / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz

volatile _Bool f_pump_on = false; // Initialize fertilizer pump to off
volatile _Bool w_pump_on = false; // Initialize water pump to off
_Bool grow_light = false;

uint8_t Water_Level = 0;
uint8_t Fertilizer_Level = 0;
uint32_t Lux_Level = 0;
uint16_t IR = 0;
uint16_t Visible = 0;

char str[4];           // Array to hold the resulting string

int main(void) {    
    DDRC |= 1 << DDC0;
    
    sei(); // Enable Global Interrupts

    init_reg();
    i2c_init(BDIV);


    LCD_Initialize();
    // End initialization


    setRotation(3);
    fillScreen(ILI9341_DARKGREEN);

    setTextSize(3);
    setTextanBGColor(ILI9341_RED, ILI9341_DARKGREEN);
    setCursor(0, 150);

    printString("WATER SENSOR TEST");
    _delay_ms(1000);

    fillScreen(ILI9341_DARKGREEN);
    
 


    fillScreen(ILI9341_DARKGREEN);
    setCursor(20,20);
    sprintf(str, "%u", Water_Level);
    printString(str);

    setCursor(80,20);
    sprintf(str, "%u", Fertilizer_Level);
    printString(str);

    enum REGOUT led_select1, led_select2; // Declaration w/o initialization leaves LEDs in previous position on restart
   
    while (1){   
    Water_Level = checkWaterLevel(WATER_LEVEL_CHANNEL);
    Fertilizer_Level = checkWaterLevel(FERTILIZER_CHANNEL);
    
    setCursor(20,20);
    sprintf(str, "%u", Water_Level);
    printString(str);

    setCursor(80,20);
    sprintf(str, "%u", Fertilizer_Level);
    printString(str);

    _delay_ms(500);

    }
    return 0;   /* never reached */
}
