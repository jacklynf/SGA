#include <stdint.h>

#define MUX_ADDR      0x70
#define MUX_READ      MUX_ADDR + 0x01
#define TCA_CHANNEL_0 0x1 // Water level sensor addr at mux
#define TCA_CHANNEL_2 0x4 // Fertilizer level sensor addr at mux

uint8_t channels;

void init_mux();
void open_channel(uint8_t);
void close_channel(uint8_t);
void close_all();