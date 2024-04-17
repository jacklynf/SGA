#include "LCD.h"


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
    gfxFont = NULL;
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
        sendCommand(cmd, addr, numArgs);    //Write each data byte
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

    SPIWRITE(commandByte);; // Send the command byte

    PORTB |= (CS | DC); //Set CS and DC pin high (Deselect Slave and put in Data Mode)
    sei(); //Re-enable interrupts after SPI Command is done being sent
    // SPI_END_TRANSACTION();
}