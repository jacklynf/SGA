#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "LCD_Macros.h"

#define SPCR_BITS (1 << SPE) | (1 << MSTR) //SPE==1 (Enable SPI Operation), DORD==0 (DataOrder is MSBFIRST), MSTR==1 (Select Master Mode), SPR1 = SPR0 = 0 (SPICLK = f/2)
#define SPSR_BITS (1 << SPR2X)  //Set SPI Frequency to CLOCK/2
#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height
#define DC (1 << PB1);
#define CS (1 << PB2);
#define SPIWRITE(data) for (SPDR = (data); (!(SPSR & (1 << SPIF)));) //Macro to send SPI command, and wait for SPIF register to set, indicating transfer is finished

//LCD Memory Initialization Command Sequence
extern const uint8_t PROGMEM initcmd[]

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
 void LCD_Initialize() //Setup DDR and PORT registers for SPI and more and send appropiate commands to initalize LCD
 
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
void sendCommand(uint8_t commandByte)

#endif  //LCD_H