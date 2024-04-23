#include "LCD.h"
#include "LCD_GFX.h"
#include "encoder.h"

//LCD Variables
int16_t _width;       ///< Display width as modified by current rotation
int16_t _height;      ///< Display height as modified by current rotation
int16_t cursor_x = 0;     ///< x location to start print()ing text
int16_t cursor_y = 0;     ///< y location to start print()ing text
uint16_t textcolor = 0xFFFF;   ///< 16-bit background color for print()
uint16_t textbgcolor = 0xFFFF; ///< 16-bit text color for print()
uint8_t textsize_x = 1;   ///< Desired magnification in X-axis of text to print()
uint8_t textsize_y = 1;   ///< Desired magnification in Y-axis of text to print()
uint8_t rotation = 0;     ///< Display rotation (0 thru 3)
bool wrap = true;            ///< If set, 'wrap' text at right edge of display
bool _cp437 = false;          ///< If set, use correct CP437 charset (default is off)
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
    uint8_t i;
    for (i = 0; i < numDataBytes; i++) {
            SPIWRITE(pgm_read_byte(dataBytes)); // Send the data bytes
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
  //static uint16_t old_x1 = 0xffff, old_x2 = 0xffff;
  //static uint16_t old_y1 = 0xffff, old_y2 = 0xffff;

  uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
  //if (x1 != old_x1 || x2 != old_x2) {
    writeCommand(ILI9341_CASET); // Column address set

    SPI_WRITE16(x1);
    SPI_WRITE16(x2);
    //old_x1 = x1;
    //old_x2 = x2;
  //}
  // if (y1 != old_y1 || y2 != old_y2) {
    writeCommand(ILI9341_PASET); // Row address set
    SPI_WRITE16(y1);
    SPI_WRITE16(y2);
    //old_y1 = y1;
    //old_y2 = y2;
  // }
  writeCommand(ILI9341_RAMWR); // Write to RAM
}

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
uint8_t spiRead(void) {
    SPDR = ((uint8_t)0);
    return SPDR;
}

void init_base_screen(uint8_t user_setting){
  char buf[16];

  setTextSize(3);
  setCursor(50,5);
  printString("SGA Readings");

  setCursor(70,35);
  setTextSize(2);
  printString("Luminosity: ");  

  setCursor(80,60);
  setTextSize(2);
  printString("Humidity: "); 
  setCursor(240,60);
  printString("%");

  setCursor(50,85);
  printString("Soil moisture: ");

  setCursor(50,110);
  printString("N: ");
  setCursor(125,110);
  printString("P: ");
  setCursor(200,110);
  printString("K: ");


  setCursor(10,135);
  printString("H20: ");

  setCursor(100,135);
  printString("%");

  setCursor(130,135);  
  printString("Nutrients: ");

  setCursor(285,135);
  printString("%");


  setTextSize(3);
  setCursor(30,165);
  printString("Plant Settings");
  ud_lcd_encoder(user_setting);
}

void ud_lcd_encoder(uint8_t user_setting){
  char * climate;
  char * settings;
  uint8_t center;

  switch (user_setting){
    case TROPICAL: 
      climate = "Tropical Climate";
      settings = "Humid, sunny";
      center = 75;
      break;
    case DESERT:
      climate = "  Desert Climate";
      settings = "Dry, sunny";
      center = 100;
      break;
    case TEMPERATE:
      climate = "Temperate Climate";
      settings = "Humid, low light";
      center = 50;
      break;
    case ALPINE:
      climate = "  Alpine Climate";
      settings = "Dry, moderate light";
      center = 40;
      break;
  }
  fillRect(5, 190, 300, 50, ILI9341_DARKGREEN);
  setTextSize(2);
  setCursor(50,200);
  printString(climate);
  setCursor(center,225);
  printString(settings);

}

void ud_lcd_light(int lum){
  char buf[16];
  setTextSize(2);        
  setCursor(210,35);
  fillRect(210, 35, 100, 25, ILI9341_DARKGREEN);
  if (lum == 0){
      printString("saturated.");
  }else{
      itoa(lum, buf, 10); 
      printString(buf);
  }
}

void ud_lcd_humidity(uint16_t humidity){
  char buf[16];
  
  // Clear previous values
  fillRect(200, 60, 35, 25, ILI9341_DARKGREEN);
  itoa(humidity, buf, 10); 
  setTextSize(2);
  setCursor(200,60);
  printString(buf);
}

void ud_lcd_moisture(unsigned char moisture){
  char buf[16];

  fillRect(225, 85, 50, 25, ILI9341_DARKGREEN);
  
  setTextSize(2);
  itoa(moisture, buf, 10);
  setCursor(225,85);
  printString(buf);
}

void ud_lcd_npk(unsigned char nitrogen, unsigned char phosphorus, unsigned char potassium){
  char buf[16];
  
  setTextSize(2);
  // Clear previous values 
  fillRect(75, 110, 50, 25, ILI9341_DARKGREEN);
  fillRect(150, 110, 50, 25, ILI9341_DARKGREEN);
  fillRect(225, 110, 50, 25, ILI9341_DARKGREEN);
  
  // Print nitrogen
  itoa(nitrogen, buf, 10); 
  setCursor(75,110);
  printString(buf);

  // Print phosphorus
  itoa(phosphorus, buf, 10);
  setCursor(150,110);
  printString(buf);

  // Print potassium
  itoa(potassium, buf, 10);
  setCursor(225,110);
  printString(buf);
}

void ud_lcd_liquids(uint8_t water, uint8_t nutrients){
  char buf[16];
  setTextSize(2);

  if (water == NULL){
    fillRect(250, 135, 35, 25, ILI9341_DARKGREEN);
    itoa(nutrients, buf, 10);
    setCursor(250,135);
    printString(buf);
  }

  if (nutrients == NULL){
    fillRect(60, 135, 35, 25, ILI9341_DARKGREEN);
    itoa(water, buf, 10);
    setCursor(60,135);
    printString(buf);
  }




}