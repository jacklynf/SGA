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
#include "WaterLevel.h"

void compute_needs();

// How often to check sensors. 
// Timer interrupts every 2 seconds, so all count values are multiplied by 2 seconds.
#define   MOISTURE_COUNT 2 // Check soil moisture
#define   HUMIDITY_COUNT 3 // Check environment humidity
#define        NPK_COUNT 2 // Check soil nutrients
#define      LIGHT_COUNT 2 // Check environment light levels
#define      WATER_COUNT 1 // Check water reservoir
#define FERTILIZER_COUNT 3 // Check fertilizer reservoir

#define BDIV (F_CPU / 100000 - 16) / 2 + 1    // Puts I2C rate just below 100kHz

// Volatile variables for interrupts
volatile uint8_t ud_encoder;
volatile int8_t raw_encoder_count = 0;
volatile bool encoder_changed_flag;
volatile uint8_t LCD_state;

// NPK variables
volatile uint8_t rx_complete = false, check_npk = 0, fertilizer_complete = true; // NPK flags
volatile uint8_t j = 0, npk_counter = 0;                    // NPK interrupt counters
volatile unsigned char npk_buf[8] = {0};                    // NPK RX buffer
volatile _Bool f_pump_on = false;                           // Initialize fertilizer pump to off

// Grow light variables
volatile uint8_t check_light = 0;                           // Light sensor flag
volatile uint8_t lightsensor_counter = 0;                   // Light sensor interrupt counter

// Humidity variables
volatile _Bool w_pump_on = false;                           // Initialize water pump to off
volatile uint8_t water_complete = true;                     // Water flags
volatile uint8_t humidity_counter = 0;                      // Humidity interrupt counter
volatile uint8_t check_humidity = 0;                        // Humidity flag

// Soil moisture variables
volatile uint8_t check_moisture = 0;                        // Soil moisture flag
volatile uint8_t moisture_counter = 0;                      // Soil moisture interrupt counter
volatile unsigned char moisture = 0;

volatile uint8_t check_water = 0;                           // Check water levels flag
volatile uint8_t check_fert = 0;                            // Check fertilizer levels flag
volatile uint8_t water_counter = 0, fertilizer_counter = 0; // Water/fertilizer interrupt counters

volatile uint8_t test_flag = 0;
///////////////  End volatile variables

uint16_t lum, humidity = 0, water_light;
uint8_t water_needs, light_needs, dev_id, moist_threshold, humidity_threshold, light_threshold;

// Determine watering & grow light needs based on user input
void compute_needs(){ 
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
    LCD_state = init_encoder();
    init_timer();
    init_reg();
    init_npk();
    init_humidity();
    init_soilmoisture();
    LCD_Initialize();
    setRotation(3);
    fillScreen(ILI9341_DARKGREEN);
    // End initialization

    _Bool grow_light = false;
    uint16_t counter = 0;
    uint8_t screen_flag = 0, encoder_sensivity = 7;
    int water_lev = -1, fert_lev = -1;
    char buf[16];
    enum REGOUT led_select1, led_select2; // Declaration w/o initialization leaves LEDs in previous position on restart
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
    if (!init_lightsensor()){
        setCursor(55,50);
        printString("All sensors");
        setCursor(90,100);
        printString("enabled.");
    }
    // End light sensor configuration
    _delay_ms(2000); // Pause on sensor screen
    fillScreen(ILI9341_DARKGREEN); 
    init_base_screen(LCD_state);


    while (1){  
        if (check_fert){ // Check if fertilizer levels are OK
            check_fert = 0;
            fert_lev = checkWaterLevel(FERTILIZER_CHANNEL);
            ud_lcd_liquids(fert_lev, NULL);
            if ((fert_lev < 50) && (fert_lev >= 25))
                sendOutput(led_select1 = YELLOW1, led_select2, w_pump_on, f_pump_on, grow_light);
            else if (fert_lev < 25)
                sendOutput(led_select1 = RED1, led_select2, w_pump_on, f_pump_on, grow_light);
            else 
                sendOutput(led_select1 = GREEN1, led_select2, w_pump_on, f_pump_on, grow_light);
        }

        if (check_water){ // Check if water levels are OK
            check_water = 0;
            water_lev = checkWaterLevel(WATER_LEVEL_CHANNEL);
            ud_lcd_liquids(NULL, water_lev);
            if ((water_lev < 50) && (water_lev >= 25))
                sendOutput(led_select1, led_select2 = YELLOW2, w_pump_on, f_pump_on, grow_light);
            else if (water_lev < 25)
                sendOutput(led_select1, led_select2 = RED2, w_pump_on, f_pump_on, grow_light);
            else 
                sendOutput(led_select1, led_select2 = GREEN2, w_pump_on, f_pump_on, grow_light);
        }

        if (check_light){ // Check if grow light is needed
            check_light = false;
            lum = get_luminosity();
            ud_lcd_light(lum);
            if (lum < light_threshold) // Adjust this value based on user settings
                sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light = false); // Turn grow light on if light is low 
            else           
                sendOutput(led_select1, led_select2, w_pump_on, f_pump_on, grow_light = true); // Turn grow light on if light is low
        }

        if(encoder_changed_flag) { // Set plant needs based on user input
            if(abs(raw_encoder_count) >= encoder_sensivity){
                screen_flag = true;
                if(raw_encoder_count < 0){
                    if(LCD_state <= 0){
                        LCD_state = 3;
                    }else{
                        LCD_state--;
                    }
                }else{
                    if(LCD_state >= 3){
                        LCD_state = 0;
                    }else{
                        LCD_state++;
                    }
                }
                raw_encoder_count = 0;
            }
            encoder_changed_flag = false;  
        }

        if (screen_flag){ // Check if the encoder values are signaling an interrupt
            screen_flag = false;
            water_light = user_input(LCD_state); // Water needs in upper 8 bits, light needs in lower 8 bits
            water_needs = (water_light >> 8), light_needs = water_light; // values defined in encoder ENUM
            compute_needs(); 
            ud_lcd_encoder(LCD_state);
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

            if ((moisture < moist_threshold)&&(humidity < humidity_threshold)){ // Check if readings fall below thresholds
                sendOutput(led_select1, led_select2, w_pump_on = false, f_pump_on, grow_light); // Turn on water pump
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
            sendOutput(led_select1, led_select2, w_pump_on = true, f_pump_on, grow_light); // Turn water pump off
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
            raw_encoder_count++;
        }else if(encoder_B){
            //Counter-Clockwise Rotation
            encoder_new_state = 2;
            raw_encoder_count--;
        }
	}
	else if (encoder_old_state == 1) {
        if(encoder_B){
            //Clockwise Rotation
            encoder_new_state = 3;
            raw_encoder_count++;
        }else if(!encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = 0;
            raw_encoder_count--;
        }
	}
	else if (encoder_old_state == 2) {
        if(!encoder_B){
            //Clockwise Rotation
            encoder_new_state = 0;
            raw_encoder_count++;
        }else if(encoder_A){
            //Counter-Clockwise Rotation
            encoder_new_state = 3;
            raw_encoder_count--;
        }
	}
	else {   // old_state = ALPINE
        if(!encoder_A){
            //Clockwise Rotation
            encoder_new_state = 2;
            raw_encoder_count++;
        }else if(!encoder_B){
            //Counter-Clockwise Rotation
            encoder_old_state = 1;
            raw_encoder_count--;
        }
	}

    //If state has changed, change oldstate to newstate and set changed flag to report that the encoder was turned.
	if (encoder_new_state != encoder_old_state) {
	    encoder_changed_flag = 1;
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