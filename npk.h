#define FOSC    7372800             // Clock frequency
#define BAUD    4800                // Default baud for NPK sensor
#define MYUBRR  FOSC/16/BAUD-1    // Value for UBRR0

#define RE_DE (1 << PD2) // RE pin - active low, DE pin - active high

void init_npk();
char rx_char();
void tx_char(char);
void USART_Flush(void);