#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "npk.h"

void init_npk(){
    UBRR0   =  MYUBRR;                                       // Set baud rate
    UCSR0B  =  (1 << TXEN0   | 1 << RXEN0 | 1 << RXCIE0 );   // Enable RX and TX, enables RX interrupts
    UCSR0C  =  (1 << UMSEL00 | 3 << UCSZ00);                 // Async., no parity, 1 stop bit, 8 data bits
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

void USART_Flush( void ) { 
    unsigned char dummy; 
    while ( UCSR0A & (1<<RXC0) ) dummy = UDR0; // flush buffer
}