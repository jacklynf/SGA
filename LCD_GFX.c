#include "LCD_GFX.h"

/**************************************************************************/
/*!
    @brief  Fill the framebuffer completely with one color
    @param  color 8-bit Color to fill with. Only lower byte of uint16_t is used.
*/
/**************************************************************************/

void fillScreen(uint16_t color) {
    startWrite();
    setAddrWindow(0, 0, _width, _height);
    for (uint32_t x = 0; x < 76800; x++) {
        SPI_WRITE16(color);
    }
    endWrite();
}