#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "LCD_Macros.h"

#define SPCR_BITS ((1 << SPE) | (1 << MSTR)) //SPE==1 (Enable SPI Operation), DORD==0 (DataOrder is MSBFIRST), MSTR==1 (Select Master Mode), SPR1 = SPR0 = 0 (SPICLK = f/2)
#define SPSR_BITS (1 << SPR2X)  //Set SPI Frequency to CLOCK/2
#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height
#define DC (1 << PB1)
#define CS (1 << PB2)
#define SPIWRITE(data) for (SPDR = (data); (!(SPSR & (1 << SPIF)));) //Macro to send SPI command, and wait for SPIF register to set, indicating transfer is finished
#define SCK PB5
#define MISO PB4
#define MOSI PB3

//LCD Memory Initialization Command Sequence
extern const uint8_t PROGMEM initcmd[];

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

//Functions
 void LCD_Initialize(void); //Setup DDR and PORT registers for SPI and more and send appropiate commands to initalize LCD
 
 /*!
 @brief   TRANSLATED FROM Adafruit_SPITFT sendCommand()
 @brief   Adafruit_SPITFT Send Command handles complete sending of commands and data
 @param   commandByte       The Command Byte
 @param   dataBytes         A pointer to the Data bytes to send
 @param   numDataBytes      The number of bytes we should send
 */
void sendCommandAndData(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes);

/*!
 @brief   TRANSLATED FROM Adafruit_SPITFT sendCommand()
 @brief   Adafruit_SPITFT Send Command handles complete sending of commands and
 data
 @param   commandByte       The Command Byte
 */
void sendCommand(uint8_t commandByte);

/*!
    @brief  Call before issuing command(s) or data to display. Performs
            chip-select (if required) and starts an SPI transaction (if
            using hardware SPI and transactions are supported). Required
            for all display types; not an SPI-specific function.
*/
static inline void startWrite(void) {
  //SPI_BEGIN_TRANSACTION();
  cli(); //Disable interrupts while SPI Command is being sent
  PORTB &= ~(CS); //Set CS and pin Low (Select Slave)
}


/*!
    @brief  Call after issuing command(s) or data to display. Performs
            chip-deselect (if required) and ends an SPI transaction (if
            using hardware SPI and transactions are supported). Required
            for all display types; not an SPI-specific function.
*/
static inline void endWrite(void) {
  PORTB |= (CS); //Set CS and pin High (Deselect Slave)
  sei(); //Re-enable interrupts after SPI Command is done being sent
  //SPI_END_TRANSACTION();
}

/*!
    @brief  Write a single command byte to the display. Chip-select and
            transaction must have been previously set -- this ONLY sets
            the device to COMMAND mode, issues the byte and then restores
            DATA mode. There is no corresponding explicit writeData()
            function -- just use spiWrite().
    @param  cmd  8-bit command to write.
*/
static inline void writeCommand(uint8_t cmd) {
  PORTB &= ~(DC);
  SPIWRITE(cmd);
  PORTB |= (DC);
}

/*!
    @brief  Issue a single 16-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the word. Despite the name,
            this function is used even if display connection is parallel;
            name was maintaned for backward compatibility. Naming is also
            not consistent with the 8-bit version, spiWrite(). Sorry about
            that. Again, staying compatible with outside code.
    @param  w  16-bit value to write.
*/
static inline void SPI_WRITE16(uint16_t w) {
    SPIWRITE(w >> 8);
    SPIWRITE(w);
}



void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);


/*!
    @brief   Read a single 8-bit value from the display. Chip-select and
             transaction must have been previously set -- this ONLY reads
             the byte. This is another of those functions in the library
             with a now-not-accurate name that's being maintained for
             compatibility with outside code. This function is used even if
             display connection is parallel.
    @return  Unsigned 8-bit value read (always zero if USE_FAST_PINIO is
             not supported by the MCU architecture).
*/
uint8_t spiRead(void);


/*drawPixel Function*/
static inline void writePixel(int16_t x, int16_t y, uint16_t color) {
    // Clip first...
    if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
        setAddrWindow(x, y, 1, 1);
        SPI_WRITE16(color);
    }
}







/*!
    @brief  Call before issuing command(s) or data to display. Performs
            chip-select (if required) and starts an SPI transaction (if
            using hardware SPI and transactions are supported). Required
            for all display types; not an SPI-specific function.
*/
// uint8_t readcommand8(uint8_t commandByte, uint8_t index);



/*!
    @brief  A lower-level version of writeFillRect(). This version requires
            all inputs are in-bounds, that width and height are positive,
            and no part extends offscreen. NO EDGE CLIPPING OR REJECTION IS
            PERFORMED. If higher-level graphics primitives are written to
            handle their own clipping earlier in the drawing process, this
            can avoid unnecessary function calls and repeated clipping
            operations in the lower-level functions.
    @param  x      Horizontal position of first corner. MUST BE WITHIN
                   SCREEN BOUNDS.
    @param  y      Vertical position of first corner. MUST BE WITHIN SCREEN
                   BOUNDS.
    @param  w      Rectangle width in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  h      Rectangle height in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  color  16-bit fill color in '565' RGB format.
    @note   This is a new function, no graphics primitives besides rects
            and horizontal/vertical lines are written to best use this yet.
*/
// static inline void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
//   setAddrWindow(x, y, w, h);
//   writeColor(color, (uint32_t)(w * h));
// }

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
// void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

// /*!
//     @brief  Issue a series of pixels, all the same color. Not self-
//             contained; should follow startWrite() and setAddrWindow() calls.
//     @param  color  16-bit pixel color in '565' RGB format.
//     @param  len    Number of pixels to draw.
// */
// void writeColor(uint16_t color, uint32_t len);
void init_base_screen(uint8_t);
void ud_lcd_light(int);
void ud_lcd_humidity(uint16_t);
void ud_lcd_npk(unsigned char, unsigned char, unsigned char);
void ud_lcd_moisture(unsigned char);
void ud_lcd_encoder(uint8_t);
void ud_lcd_liquids(uint8_t water, uint8_t nutrients);

#endif  //LCD_H