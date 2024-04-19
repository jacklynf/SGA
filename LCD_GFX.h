#ifndef LCD_GFX
#define LCD_GFX

#include "LCD.h"
#include "LCD_Macros.h"

void fillScreen(uint16_t color);

void setRotation(uint8_t m);

void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);

void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color); 

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    startWrite();
    writeFillRect(x,y,w,h,color);
    endWrite();
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);

void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

static inline void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){
  startWrite();
  writeFastVLine(x,y,h,color);
  endWrite();
}


static inline void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color){
  startWrite();
  writeFastHLine(x,y,w,color);
  endWrite();
}


void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);

size_t write(uint8_t c);

void setTextSize(uint8_t s);

 /**********************************************************************/
  /*!
    @brief  Set text cursor location
    @param  x    X coordinate in pixels
    @param  y    Y coordinate in pixels
  */
  /**********************************************************************/
  void setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
  }

  /**********************************************************************/
  /*!
    @brief   Set text font color with transparant background
    @param   c   16-bit 5-6-5 Color to draw text with
    @note    For 'transparent' background, background and foreground
             are set to same color rather than using a separate flag.
  */
  /**********************************************************************/
  void setTextColor(uint16_t c) { 
    textcolor = c;
    textbgcolor = c; 
  }

  /**********************************************************************/
  /*!
    @brief   Set text font color with custom background color
    @param   c   16-bit 5-6-5 Color to draw text with
    @param   bg  16-bit 5-6-5 Color to draw background/fill with
  */
  /**********************************************************************/
<<<<<<< HEAD
  void setTextandBGColor(uint16_t c, uint16_t bg) {
=======
  void setTextColor(uint16_t c, uint16_t bg) {
>>>>>>> parent of 8617010 (LCD Update #7.1)
    textcolor = c;
    textbgcolor = bg;
  }

  /**********************************************************************/
  /*!
  @brief  Set whether text that is too long for the screen width should
          automatically wrap around to the next line (else clip right).
  @param  w  true for wrapping, false for clipping
  */
  /**********************************************************************/
  void setTextWrap(bool w) { wrap = w; }

  void printString(const char* string);
  
#endif  //LCD_GFX

