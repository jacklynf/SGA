#include "i2c_mux.h"
#include "WaterLevel.h"
#include <util/delay.h>
#include <stddef.h>

#define SENSOR_ADDR 0X78

void init_mux(){
    open_channel(TCA_CHANNEL_0); 
    open_channel(TCA_CHANNEL_2); 
    close_all(); // Set baseline
}

void open_channel(uint8_t channel){
    uint8_t buff = 0x00;    
    buff = 1 << channel;
    channels |= buff;

    i2c_io(MUX_ADDR, channels, 1, NULL, 0);
}

void close_channel(uint8_t channel){
    uint8_t buff = 0x00;    
    buff = 1 << channel;    
    channels ^= buff;

    i2c_io(MUX_ADDR, channels, 1, NULL, 0);    
}

void close_all(){ // Base state for i2c mux
    channels = 0x00;

    i2c_io(MUX_ADDR, channels, 1, NULL, 0);    
}

int read_level(uint8_t channel){ // Read 1 sensor from the mux at a time
    uint8_t buff[2] = {channel, SENSOR_ADDR};
    // _delay_ms(10);
    if (!i2c_io(MUX_ADDR, buff, 2, buff, 1)){
        close_channel(channel);
        return buff[0];
    }
    return -1; // If this is reached, there was an error in reading sensor
}

