#include "Touchscreen.h"
#include "LCD_GFX.h"
#include <stdio.h>
#include <stdint.h>

    uint8_t touches;
    uint16_t touchX[2], touchY[2], touchID[2];

    static uint8_t rxBuffer[BUFFER_LENGTH];
    static uint8_t rxBufferIndex = 0;
    static uint8_t rxBufferLength = 0;

    static uint8_t txAddress = 0;
    static uint8_t txBuffer[BUFFER_LENGTH];
    static uint8_t txBufferIndex = 0;
    static uint8_t txBufferLength = 0;



  bool beginTouchScreen(uint8_t thresh) { 

    DDRD   &= ~(1 << PD4);
    //PORTD  |= (1 << PD4);
    PCICR  |=  (1 << PCIE2); //Enable Pin Change interrupts on PORTD
    PCMSK2 |= (1 << PD4);
    //PORTD  |=  (1 << PD4);

    txBuffer[0] = FT62XX_REG_THRESHHOLD;
    txBuffer[1] = thresh;
    if(i2c_io(FT62XX_DEFAULT_ADDR, txBuffer, 2, NULL, 0)){
         return false;
    }

    _delay_us(3);
    txBuffer[0] = FT62XX_REG_VENDID;
    if(i2c_io(FT62XX_DEFAULT_ADDR, txBuffer, 1, rxBuffer, 1) || rxBuffer[0] != FT62XX_VENDID)
        return false;
    return true;

}

bool touchScreenTouched(){
    txBuffer[0] = FT62XX_REG_NUMTOUCHES;
    i2c_io(FT62XX_DEFAULT_ADDR, txBuffer, 1, rxBuffer, 1);
    _delay_us(3);
    if(rxBuffer[0] == 1){
        return true;
    }
    return false;
}

