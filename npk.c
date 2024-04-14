#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#include "npk.h"

void init_npk(){
    UBRR0   =  MYUBRR;                                       // Set baud rate
    UCSR0B  =  (1 << TXEN0)   | (1 << RXEN0) | (1 << RXCIE0 );   // Enable RX and TX, enables RX interrupts
    UCSR0C  =  (3 << UCSZ00);                                // Async., no parity, 1 stop bit, 8 data bits
    DDRD   |=  (1 << PD2);                                   // Set PD2 to output (DE/~RE pins on transceiver)
}

char rx_char() { 
    while ( !(UCSR0A & (1 << RXC0))); // Wait for receive complete flag to go high     
    return UDR0; 
}

void tx_char(char ch) {    
    while ((UCSR0A & (1<<UDRE0)) == 0); // Wait for transmitter data register empty    
    UDR0 = ch; 
}

void get_npk(){
    const char request_npk[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x03, 0x65, 0xcd}; // request to send to transceiver
    int i = 0;

    PORTD |= RE_DE; // Set DE bit on transceiver high to TX to it

    while (i  < 8){
        tx_char(request_npk[i]);
        ++i;
    }
    while ( !(UCSR0A & (1 << TXC0))); // Wait until all bytes are sent from the atmega
    PORTD &= ~RE_DE; // Set RE bit on transceiver low (active low) to RX from it
}


/*
Last measured NPK values:
    Phosphorous: >= 0xe0, < 0xf0 (somewhere between 224 and 240 mg)
    Nitrogen: 0 (no reading)
    Potassium: >= 0, < 0x03 (somewhere around 3mg)
*/
_Bool fertilizer_needed(unsigned char nitrogen, unsigned char phosphorus, unsigned char potassium){
    _Bool fert_needed = false;
    unsigned char nit_thresh = 0x20, phos_thresh = 0xa0, pot_thresh = 0x30; 
    uint8_t nit_idx = 0, phos_idx = 1, pot_idx = 2;
    uint8_t low_npk[3] = {0};   
    uint8_t i, sum;

    // Evaluate NPK levels here 
    if (nitrogen < nit_thresh)
        low_npk[nit_idx] = 1;
    if (phosphorus < phos_thresh)
        low_npk[phos_idx] = 1;
    if (potassium < pot_thresh)
        low_npk[pot_idx] = 1;

    for (i = 0; i < 3; i++)
        sum += low_npk[i];

    if (sum >= 2)
        fert_needed = true; // If 2 elements are below the threshold, fertilizer is needed

    return fert_needed;
}