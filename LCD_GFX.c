#include "LCD_GFX.h"
#include "glcdfont.c"
#include <string.h>

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

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
    uint32_t x;
    for (x = 0; x < 76800; x++) {
        SPI_WRITE16(color);
    }
    endWrite();
}


/**************************************************************************/
/*!
   @brief   Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) at the specified
   (x,y) position. For 16-bit display devices; no color reduction performed.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with 16-bit color bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[],
                                 int16_t w, int16_t h) {
  startWrite();
  uint16_t i,j;
  for (j = 0; j < h; j++, y++) {
    for (i = 0; i < w; i++) {
      writePixel(x + i, y, pgm_read_word(&bitmap[j * w + i]));
    }
  }
  endWrite();
}

/*!
    @brief  Draw a filled rectangle to the display. Self-contained and
            provides its own transaction as needed (see writeFillRect() or
            writeFillRectPreclipped() for lower-level variants). Edge
            clipping and rejection is performed here.
    @param  x      Horizontal position of first corner.
    @param  y      Vertical position of first corner.
    @param  w      Rectangle width in pixels (positive = right of first
                   corner, negative = left of first corner).
    @param  h      Rectangle height in pixels (positive = below first
                   corner, negative = above first corner).
    @param  color  16-bit fill color in '565' RGB format.
    @note   This repeats the writeFillRect() function almost in its entirety,
            with the addition of a transaction start/end. It's done this way
            (rather than starting the transaction and calling writeFillRect()
            to handle clipping and so forth) so that the transaction isn't
            performed at all if the rectangle is rejected. It's really not
            that much code.
*/
void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  if (w && h) {   // Nonzero width and height?
    if (w < 0) {  // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width) { // Not off right
      if (h < 0) {    // If negative height...
        y += h + 1;   //   Move Y to top edge
        h = -h;       //   Use positive height
      }
      if (y < _height) { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0) { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0) { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width) {
              w = _width - x;
            } // Clip right
            if (y2 >= _height) {
              h = _height - y;
            } // Clip bottom
            setAddrWindow(x, y, w, h);
            uint32_t len = (uint32_t)(w*h);
            if (!len)
              return; // Avoid 0-byte transfers
            while (len--) {
              SPI_WRITE16(color);
            }
          }
        }
      }
    }
  }
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    startWrite();
    writeFillRect(x,y,w,h,color);
    endWrite();
}



/**************************************************************************/
/*!
   @brief    Draw a line
    @param    x0  Start point x coordinate
    @param    y0  Start point y coordinate
    @param    x1  End point x coordinate
    @param    y1  End point y coordinate
    @param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            uint16_t color) {
  // Update in subclasses if desired!
  if (x0 == x1) {
    if (y0 > y1)
      _swap_int16_t(y0, y1);
    drawFastVLine(x0, y0, y1 - y0 + 1, color);
  } else if (y0 == y1) {
    if (x0 > x1)
      _swap_int16_t(x0, x1);
    drawFastHLine(x0, y0, x1 - x0 + 1, color);
  } else {
    startWrite();
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
      _swap_int16_t(x0, y0);
      _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
      _swap_int16_t(x0, x1);
      _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
      ystep = 1;
    } else {
      ystep = -1;
    }
    for (; x0 <= x1; x0++) {
      if (steep) {
        writePixel(y0, x0, color);
      } else {
        writePixel(x0, y0, color);
      }
      err -= dy;
      if (err < 0) {
        y0 += ystep;
        err += dx;
      }
    }
    endWrite();
  }
}

/*!
    @brief  Draw a vertical line on the display. Self-contained and provides
            its own transaction as needed (see writeFastHLine() for a lower-
            level variant). Edge clipping and rejection is performed here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  h      Line height in pixels (positive = below first point,
                   negative = above first point).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastVLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastVLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void writeFastVLine(int16_t x, int16_t y, int16_t h,
                                    uint16_t color) {
  if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
    if (h < 0) {                       // If negative height...
      y += h + 1;                      //   Move Y to top edge
      h = -h;                          //   Use positive height
    }
    if (y < _height) { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0) { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0) {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height) {
          h = _height - y;
        } // Clip bottom
        setAddrWindow(x, y, 1, h);
        if (!h)
          return; // Avoid 0-byte transfers
        while (h--) {
          SPI_WRITE16(color);
        }
      }
    }
  }
}



/*!
    @brief  Draw a horizontal line on the display. Self-contained and
            provides its own transaction as needed (see writeFastHLine() for
            a lower-level variant). Edge clipping and rejection is performed
            here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  w      Line width in pixels (positive = right of first point,
                   negative = point of first corner).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastHLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastHLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void writeFastHLine(int16_t x, int16_t y, int16_t w,
                                    uint16_t color) {
  if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
    if (w < 0) {                        // If negative width...
      x += w + 1;                       //   Move X to left edge
      w = -w;                           //   Use positive width
    }
    if (x < _width) { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0) { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0) {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width) {
          w = _width - x;
        } // Clip right
        setAddrWindow(x, y, w, 1);
        if (!w)
          return; // Avoid 0-byte transfers
        while (w--) {
          SPI_WRITE16(color);
        }
      }
    }
  }
}


// TEXT- AND CHARACTER-HANDLING FUNCTIONS ----------------------------------

// Draw a character
/**************************************************************************/
/*!
   @brief   Draw a single character
    @param    x   Bottom left corner x coordinate
    @param    y   Bottom left corner y coordinate
    @param    c   The 8-bit font-indexed character (likely ascii)
    @param    color 16-bit 5-6-5 Color to draw chraracter with
    @param    bg 16-bit 5-6-5 Color to fill background with (if same as color,
   no background)
    @param    size  Font magnification level, 1 is 'original' size
*/
/**************************************************************************/
void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
    //if (!gfxFont) { // 'Classic' built-in font

    if ((x >= _width) ||              // Clip right
        (y >= _height) ||             // Clip bottom
        ((x + 6 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0))   // Clip top
      return;

    if (!_cp437 && (c >= 176))
      c++; // Handle 'classic' charset behavior

    startWrite();
    uint8_t i, j;
    for (i = 0; i < 5; i++) { // Char bitmap = 5 columns
      uint8_t line = pgm_read_byte(&font[c * 5 + i]);
      for (j = 0; j < 8; j++, line >>= 1) {
        if (line & 1) {
          if (size == 1 && size == 1)
            writePixel(x + i, y + j, color);
          else
            writeFillRect(x + i * size, y + j * size, size, size,
                          color);
        } else if (bg != color) {
          if (size == 1 && size == 1)
            writePixel(x + i, y + j, bg);
          else
            writeFillRect(x + i * size, y + j * size, size, size, bg);
        }
      }
    }
    if (bg != color) { // If opaque, draw vertical line for last column
      if (size == 1 && size == 1)
        writeFastVLine(x + 5, y, 8, bg);
      else
        writeFillRect(x + 5 * size, y, size, 8 * size, bg);
    }
    endWrite();

  // } else { // Custom font

  //   // Character is assumed previously filtered by write() to eliminate
  //   // newlines, returns, non-printable characters, etc.  Calling
  //   // drawChar() directly with 'bad' characters of font may cause mayhem!

  //   c -= (uint8_t)pgm_read_byte(&gfxFont->first);
  //   GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c);
  //   uint8_t *bitmap = pgm_read_bitmap_ptr(gfxFont);

  //   uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
  //   uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
  //   int8_t xo = pgm_read_byte(&glyph->xOffset),
  //          yo = pgm_read_byte(&glyph->yOffset);
  //   uint8_t xx, yy, bits = 0, bit = 0;
  //   int16_t xo16 = 0, yo16 = 0;

  //   if (size > 1 || size > 1) {
  //     xo16 = xo;
  //     yo16 = yo;
  //   }

  //   // Todo: Add character clipping here

  //   // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
  //   // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
  //   // has typically been used with the 'classic' font to overwrite old
  //   // screen contents with new data.  This ONLY works because the
  //   // characters are a uniform size; it's not a sensible thing to do with
  //   // proportionally-spaced fonts with glyphs of varying sizes (and that
  //   // may overlap).  To replace previously-drawn text when using a custom
  //   // font, use the getTextBounds() function to determine the smallest
  //   // rectangle encompassing a string, erase the area with fillRect(),
  //   // then draw new text.  This WILL infortunately 'blink' the text, but
  //   // is unavoidable.  Drawing 'background' pixels will NOT fix this,
  //   // only creates a new set of problems.  Have an idea to work around
  //   // this (a canvas object type for MCUs that can afford the RAM and
  //   // displays supporting setAddrWindow() and pushColors()), but haven't
  //   // implemented this yet.

  //   startWrite();
  //   for (yy = 0; yy < h; yy++) {
  //     for (xx = 0; xx < w; xx++) {
  //       if (!(bit++ & 7)) {
  //         bits = pgm_read_byte(&bitmap[bo++]);
  //       }
  //       if (bits & 0x80) {
  //         if (size == 1 && size == 1) {
  //           writePixel(x + xo + xx, y + yo + yy, color);
  //         } else {
  //           writeFillRect(x + (xo16 + xx) * size, y + (yo16 + yy) * size,
  //                         size, size, color);
  //         }
  //       }
  //       bits <<= 1;
  //     }
  //   }
  //   endWrite();

  // } // End classic vs custom font
}

/**************************************************************************/
/*!
    @brief  Print one byte/character of data, used to support print()
    @param  c  The 8-bit ascii character to write
*/
/**************************************************************************/
size_t write(uint8_t c) {
  // if (!gfxFont) { // 'Classic' built-in font

    if (c == '\n') {              // Newline?
      cursor_x = 0;               // Reset x to zero,
      cursor_y += textsize_y * 8; // advance y one line
    } else if (c != '\r') {       // Ignore carriage returns
      if (wrap && ((cursor_x + textsize_x * 6) > _width)) { // Off right?
        cursor_x = 0;                                       // Reset x to zero,
        cursor_y += textsize_y * 8; // advance y one line
      }
      drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x);
      cursor_x += textsize_x * 6; // Advance x one char
    }

  // } else { // Custom font

  //   if (c == '\n') {
  //     cursor_x = 0;
  //     cursor_y +=
  //         (int16_t)textsize_y * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
  //   } else if (c != '\r') {
  //     uint8_t first = pgm_read_byte(&gfxFont->first);
  //     if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
  //       GFXglyph *glyph = pgm_read_glyph_ptr(gfxFont, c - first);
  //       uint8_t w = pgm_read_byte(&glyph->width),
  //               h = pgm_read_byte(&glyph->height);
  //       if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
  //         int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
  //         if (wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
  //           cursor_x = 0;
  //           cursor_y += (int16_t)textsize_y *
  //                       (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
  //         }
  //         drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x,
  //                  textsize_y);
  //       }
  //       cursor_x +=
  //           (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize_x;
  //     }
  //   }
  // }
  return 1;
}

/**************************************************************************/
/*!
    @brief   Set text 'magnification' size. Each increase in s makes 1 pixel
   that much bigger.
    @param  s  Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
*/
/**************************************************************************/
void setTextSize(uint8_t s) {
  textsize_x = (s > 0) ? s : 1;
  textsize_y = (s > 0) ? s : 1;
}

void printString(const char* string){
  uint8_t i;
  for(i = 0; i < strlen(string); i++){
    write(string[i]);
  }
}

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
  void setTextColor(uint16_t c) { textcolor = textbgcolor = c; }

  /**********************************************************************/
  /*!
    @brief   Set text font color with custom background color
    @param   c   16-bit 5-6-5 Color to draw text with
    @param   bg  16-bit 5-6-5 Color to draw background/fill with
  */
  /**********************************************************************/
  void setTextanBGColor(uint16_t c, uint16_t bg) {
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