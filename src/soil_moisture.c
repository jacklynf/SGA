#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "soil_moisture.h"


#define MUX_BITS ((1 << MUX3)|(1 << MUX2)|(1 << MUX1)|(1 << MUX0))

void init_soilmoisture(){
    // Initialize the ADC

    ADMUX |= (1 << REFS0)|(1 << ADLAR); //set REF bits (0-5V VCC) & 8bit accuracy (ADLAR bit)
    ADMUX &= ~(1 << REFS1);
    
    ADCSRA |= (1 << ADPS2)|(1 << ADPS1)|(1 << ADPS0); //Initialize clock prescalar to 128

    ADCSRA |= (1 << ADEN); //turn on enable bit
}

unsigned char adc_sample(unsigned char channel)
{
    unsigned char result;
    // Set ADC input mux bits to 'channel' value
    ADMUX &= ~MUX_BITS; // clear mux[3:0] values
    ADMUX |= (channel & MUX_BITS); // copy channel selection into mux

    // Convert an analog input and return the 8-bit result
    ADCSRA |= (1 << ADSC);
    while ((ADCSRA & (1 << ADSC)) != 0){
    }
    
    result = ADCH;
    
    return result;
}