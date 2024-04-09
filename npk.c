#include <avr/io.h>
#include <avr/interrupt.h>
#include "npk.h"

// Modbus RTU requests for reading NPK values
const char nitro[] = {0x01,0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const char phos[] = {0x01,0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const char pota[] = {0x01,0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// A variable used to store NPK values
char values[11];

void init_npk(){
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0);  // Enable RX and TX
    UCSR0C = (3 << UCSZ00);               // Async., no parity, // 1 stop bit, 8 data bits
}

char rx_char() { 
    // Wait for receive complete flag to go high 
    while ( !(UCSR0A & (1 << RXC0)) ) {} 
    
    return UDR0; 
}

void tx_char(char ch) { 
    // Wait for transmitter data register empty 
    while ((UCSR0A & (1<<UDRE0)) == 0) {} 
    
    UDR0 = ch; 
}