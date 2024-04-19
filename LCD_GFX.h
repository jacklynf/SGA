#ifndef LCD_GFX
#define LCD_GFX

#include "LCD.h"
#include "LCD_Macros.h"

void fillScreen(uint16_t color);

void setRotation(uint8_t m);

void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);


#endif  //LCD_GFX