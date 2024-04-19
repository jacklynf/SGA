#include "LCD_GFX.h"

/**************************************************************************/
/*!
    @brief  Fill the framebuffer completely with one color
    @param  color 8-bit Color to fill with. Only lower byte of uint16_t is used.
*/
/**************************************************************************/

void fillScreen(uint16_t color) {
    startWrite();
    for (int16_t x = 0; x < _width; x++) {
        for (int16_t y = 0; y < _height; y++){
            drawPixel(x, y, color);
        }
    }
    endWrite();
}