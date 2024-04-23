#include <stdint.h>

#define MUX_ADDR (0x70 << 1)
#define TCA_CHANNEL_0 1 // Water level sensor addr at mux
#define TCA_CHANNEL_2 4 // Fertilizer level sensor addr at mux

uint8_t channels;

void init_mux();
void open_channel(uint8_t);
void close_channel(uint8_t);
void close_all();
int read_level(uint8_t);