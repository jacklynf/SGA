#include "LCD.h"

//LCD Variables
extern int16_t _width;       ///< Display width as modified by current rotation
extern int16_t _height;      ///< Display height as modified by current rotation
extern int16_t cursor_x;     ///< x location to start print()ing text
extern int16_t cursor_y;     ///< y location to start print()ing text
extern uint16_t textcolor;   ///< 16-bit background color for print()
extern uint16_t textbgcolor; ///< 16-bit text color for print()
extern uint8_t textsize_x;   ///< Desired magnification in X-axis of text to print()
extern uint8_t textsize_y;   ///< Desired magnification in Y-axis of text to print()
extern uint8_t rotation;     ///< Display rotation (0 thru 3)
extern bool wrap;            ///< If set, 'wrap' text at right edge of display
extern bool _cp437;          ///< If set, use correct CP437 charset (default is off)
//GFXfont *gfxFont;     ///< Pointer to special font


//LCD Memory Initialization Command Sequence
const uint8_t PROGMEM initcmd[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2
  ILI9341_MADCTL  , 1, 0x48,             // Memory Access Control
  ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
  ILI9341_DISPON  , 0x80,                // Display on
  0x00                                   // End of list
};


void LCD_Initialize() {
    //Segment from Adafruit_GFX() function
    _width = WIDTH;
    _height = HEIGHT;
    rotation = 0;
    cursor_y = cursor_x = 0;
    textsize_x = textsize_y = 1;
    textcolor = textbgcolor = 0xFFFF;
    wrap = true;
    _cp437 = false;
    //gfxFont = NULL;
    //End segment from Adafruit_GFX() function

    //Begin code translated from Adafruit_ILI9341.begin()
    //Begin code segment from initSPI(freq, spiMode)
    //Set data direction and inital value for CS and DC pin
    DDRB |= CS;   //Set CS Pin as Output
    PORTB |= CS; // Set CS high -- Deselect
    DDRB |= DC; //Set DC Pin as output
    PORTB |= DC; //Set DC Pin to high (Data Mode)
    //Set Data Direction for SCK and MOSI and  pins
    DDRB |= (1 << SCK); //Set SCK Pin as output
    DDRB |= (1 << MOSI); //Set MOSI Pin as output (Master Slave Out)
    DDRB &= ~(1 << MISO); //Set MISO Pin as input (Master Slave In) (Might not use/need this functionality)

    SPCR |= SPCR_BITS; //Set appropriate bits in SPCR Register to initalize
    //End code segment from initSPI(freq, spiMode)

    sendCommand(ILI9341_SWRESET); // Engage software reset
    _delay_ms(150);

    //Begin LCD Memory initialization sequence
    uint8_t cmd, x, numArgs;
    const uint8_t *addr = initcmd; //Get the pointer to the first address of the memory initialization commands for LCD stored in Program Memory 
    while ((cmd = pgm_read_byte(addr++)) > 0) { //Loop through the entire array of commands
        x = pgm_read_byte(addr++);
        numArgs = x & 0x7F;
        sendCommandAndData(cmd, addr, numArgs);    //Write each data byte
        addr += numArgs;
        if (x & 0x80)
            _delay_ms(150);
    }
    //End LCD Memory initialization sqeuence

  _width = ILI9341_TFTWIDTH;
  _height = ILI9341_TFTHEIGHT;
  //End code segment translated from Adafruit_ILI9341.begin()

}

/*!
 @brief   TRANSLATED FROM Adafruit_SPITFT sendCommand()
 @brief   Adafruit_SPITFT Send Command handles complete sending of commands and data
 @param   commandByte       The Command Byte
 @param   dataBytes         A pointer to the Data bytes to send
 @param   numDataBytes      The number of bytes we should send
 */
void sendCommandAndData(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes) {
    // SPI_BEGIN_TRANSACTION();
    cli(); //Disable interrupts while SPI Command is being sent
    PORTB &= ~(CS | DC); //Set CS and DC pin Low (Select Slave (CS = LOW) and enter Command Mode (DC = LOW))

    SPIWRITE(commandByte); // Send the command byte

    PORTB |= (DC);     //Set DC high (enter Data Mode)

    for (int i = 0; i < numDataBytes; i++) {
            SPIWRITE(*dataBytes); // Send the data bytes
            dataBytes++;
    }
    PORTB |= (CS); //Set CS pin HIGH (Deselect Slave)
    sei(); //Re-enable interrupts after SPI Command is done being sent
    // SPI_END_TRANSACTION();
}

/*!
 @brief   TRANSLATED FROM Adafruit_SPITFT sendCommand()
 @brief   Adafruit_SPITFT Send Command handles complete sending of commands and
 data
 @param   commandByte       The Command Byte
 */
void sendCommand(uint8_t commandByte) {
    // SPI_BEGIN_TRANSACTION();
    cli(); //Disable interrupts while SPI Command is being sent
    PORTB &= ~(CS | DC); //Set CS and DC pin Low (Select Slave (CS = LOW) and enter Command Mode (DC = LOW))

    SPIWRITE(commandByte); // Send the command byte

    PORTB |= (CS | DC); //Set CS and DC pin high (Deselect Slave and put in Data Mode)
    sei(); //Re-enable interrupts after SPI Command is done being sent
    // SPI_END_TRANSACTION();
}

/*!
 @brief   Translated from readCommand8() in Adafruit_SPITFT
 @brief   Read 8 bits of data from display configuration memory (not RAM).
 This is highly undocumented/supported and should be avoided,
 function is only included because some of the examples use it.
 @param   commandByte
 The command register to read data from.
 @param   index
 The byte index into the command to read from.
 @return  Unsigned 8-bit data read from display register.
 */
/**************************************************************************/
uint8_t readcommand8(uint8_t commandByte, uint8_t index) {
  uint8_t result;
  startWrite();
  PORTB &= ~(DC); // Command mode
  SPIWRITE(commandByte);
  PORTB |= (DC); // Data mode
  do {
    result = spiRead();
  } while (index--); // Discard bytes up to index'th
  endWrite();
  return result;
}





//Shape Drawing Commands
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
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
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
            startWrite();
            writeFillRectPreclipped(x, y, w, h, color);
            endWrite();
          }
        }
      }
    }
  }
}


/**************************************************************************/
/*!
    @brief   Set the "address window" - the rectangle we will write to RAM with
   the next chunk of      SPI data writes. The ILI9341 will automatically wrap
   the data as each row is filled
    @param   x1  TFT memory 'x' origin
    @param   y1  TFT memory 'y' origin
    @param   w   Width of rectangle
    @param   h   Height of rectangle
*/
/**************************************************************************/
void setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
  static uint16_t old_x1 = 0xffff, old_x2 = 0xffff;
  static uint16_t old_y1 = 0xffff, old_y2 = 0xffff;

  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
  if (x1 != old_x1 || x2 != old_x2) {
    writeCommand(ILI9341_CASET); // Column address set
    SPI_WRITE16(x1);
    SPI_WRITE16(x2);
    old_x1 = x1;
    old_x2 = x2;
  }
  if (y1 != old_y1 || y2 != old_y2) {
    writeCommand(ILI9341_PASET); // Row address set
    SPI_WRITE16(y1);
    SPI_WRITE16(y2);
    old_y1 = y1;
    old_y2 = y2;
  }
  writeCommand(ILI9341_RAMWR); // Write to RAM
}

/*!
    @brief  Issue a series of pixels, all the same color. Not self-
            contained; should follow startWrite() and setAddrWindow() calls.
    @param  color  16-bit pixel color in '565' RGB format.
    @param  len    Number of pixels to draw.
*/
void writeColor(uint16_t color, uint32_t len) {

  if (!len)
    return; // Avoid 0-byte transfers

  uint8_t hi = color >> 8, lo = color;
                      // !ESP32

  // All other cases (non-DMA hard SPI, bitbang SPI, parallel)...
    while (len--) {
      SPIWRITE(hi);
      SPIWRITE(lo);

    }
}