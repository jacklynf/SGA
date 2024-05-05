#define FOSC    7372800             // Clock frequency
#define BAUD    4800                // Default baud for NPK sensor
#define MYUBRR  FOSC/16/BAUD-1    // Value for UBRR0

#define RE_DE (1 << PD2) // RE pin - active low, DE pin - active high

void init_npk();
char rx_char();
void tx_char(char);

// Initiates transmission to sensor to request NPK values
void get_npk();
// Calculates whether to turn on the fertilizer pump based on NPK reading
// Returns true if fertilizer pump should be turned on
_Bool fertilizer_needed(unsigned char, unsigned char, unsigned char);