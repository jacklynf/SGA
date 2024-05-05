#define ENCODERA    (1 << PC2)
#define ENCODERB    (1 << PC3)

#define WATER_IDX 0
#define LIGHT_IDX 1

// Volatile encoder variables for interrupt
volatile extern uint8_t encoder_old_state, encoder_new_state; // state machine
volatile extern uint8_t encoder_changed; // flag
volatile extern uint8_t encoder_input, encoder_A, encoder_B; //rotary encoder inputs variables

/*
    Enums are used to add readability when setting SGA's water and grow light frequency.
*/
enum PLANT_NEEDS{
    TROPICAL,  // Humid, sunny
    DESERT,    // Dry, sunny
    TEMPERATE, // Humid, low light 
    ALPINE     // Dry, moderate light
};

enum LIGHT_LEVELS{LOW, MODERATE,HIGH};
enum WATER_LEVELS{DRY,HUMID};

uint8_t init_encoder();
void encoderEnable();
void encoderDisable();

/*
    user_input() function takes the encoder state as input and uses that value to
    evaluate the plant needs.
    
    Returns: 16 bit integer with the plant's water needs in upper 8 bits and
             and light needs in the lower 8 bits.
*/
uint16_t user_input(uint8_t);
