#include "LCD_GFX.h"

/**************************************************************************/
/*!
    @brief  Fill the framebuffer completely with one color
    @param  color 8-bit Color to fill with. Only lower byte of uint16_t is used.
*/
/**************************************************************************/

/**************************************************************************/
/*!
    @brief   Set origin of (0,0) and orientation of TFT display
    @param   m  The index for rotation, from 0-3 inclusive
*/
/**************************************************************************/
void setRotation(uint8_t m) {
  rotation = m % 4; // can't be higher than 3
  switch (rotation) {
  case 0:
    m = (MADCTL_MX | MADCTL_BGR);
    _width = WIDTH;
    _height = HEIGHT;
    break;
  case 1:
    m = (MADCTL_MV | MADCTL_BGR);
    _width = HEIGHT;
    _height = WIDTH;
    break;
  case 2:
    m = (MADCTL_MY | MADCTL_BGR);
    _width = WIDTH;
    _height = HEIGHT;
    break;
  case 3:
    m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
    _width = HEIGHT;
    _height = WIDTH;
    break;
  }
  startWrite();
  writeCommand(ILI9341_MADCTL);
  SPIWRITE(m);
  endWrite();
}



void fillScreen(uint16_t color) {
    startWrite();
    setAddrWindow(0, 0, _width, _height);
    for (uint32_t x = 0; x < 76800; x++) {
        SPI_WRITE16(color);
    }
    endWrite();
}