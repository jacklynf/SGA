#include "LCD_GFX.h"

/**************************************************************************/
/*!
    @brief  Fill the framebuffer completely with one color
    @param  color 8-bit Color to fill with. Only lower byte of uint16_t is used.
*/
/**************************************************************************/
void fillScreen(uint16_t color) {
  fillRect(0, 0, _width, _height, color);
}