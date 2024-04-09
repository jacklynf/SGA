#define FOSC    9830400            // Clock frequency
#define BAUD    9600               // Baud rate used
#define MYUBRR  (FOSC/16/BAUD-1)   // Value for UBRR0

#define RE (1 << PD2) // RE pin - active low
#define DE (1 << PD2) // DE pin - active high

void init_npk();
char rx_char();
void tx_char(char);