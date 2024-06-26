#define SERIAL_DATA_OUT (1 << PD7) //Output Pin for Serial Data line to Shift Registers
#define SERIAL_CLK      (1 << PD5) //Output Pin for Serial Data Read Clock for Shift Registers
#define STORE_CLK        (1 << PD6) //Output Pin for Store Data Clock for Shift Registers; Pulse High once to shift the 16 bits in the shift register to output registers.
#define SHIFT_REG_OE    (1 << PB0) //Output Pin for Output Enable (Active Low). Pushing HIGH disables all the outputs.

#define GREEN_LED_1    (1 << 15) //Green_LED_1 is connected to Bit 15 of the shift registers
#define YELLOW_LED_1   (1 << 1) //Yellow_LED_1 is connected to Bit 14 of the shift registers
#define RED_LED_1      (1 << 2) //Red_LED_1 is connected to Bit 13 of the shift registers
#define GREEN_LED_2    (1 << 3) //Green_LED_2 is connected to Bit 12 of the shift registers
#define YELLOW_LED_2   (1 << 4) //Yellow_LED_2 is connected to Bit 11 of the shift registers
#define RED_LED_2      (1 << 5) //Red_LED_2 is connected to Bit 10 of the shift registers
#define PUMP_1         (1 << 6) //Pump_1 is connected to Bit 9 of the shift registers
#define PUMP_2         (1 << 7) //Pump_2 is connected to Bit 8 of the shift registers
//The remaining 8 bits of the shift registers are unused at this moment.

// Enum adds readability when selecting LEDs to turn on/off and which pumps to turn on/off
enum REGOUT {
    GROW_LIGHT = 7,
    FERTILIZER,
    WATER,
    RED2,   
    YELLOW2, 
    GREEN2, 
    RED1,   
    YELLOW1,
    GREEN1,
};

/*
The sendOutput function is the function that handles taking the 16-bit "outputs" varible,
representing the 16 bits we want to push into the shift registers,
and sending those bits 1 by 1 into the shift registers with the appropriate clock pulses at the correct time.

This function is called every time an I/O device needs a bit change.
*/
void sendOutput(enum REGOUT, enum REGOUT, _Bool, _Bool, _Bool);
void init_reg();