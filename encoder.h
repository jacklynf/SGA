#define ENCODERA    (1 << PC3)
#define ENCODERB    (1 << PC2)

#define WATER_IDX 0
#define LIGHT_IDX 1

// Volatile encoder variables for interrupt
volatile uint8_t encoder_old_state, encoder_new_state; // state machine
volatile uint8_t encoder_changed; // flag
volatile uint8_t encoder_input, encoder_A, encoder_B; //rotary encoder inputs variables

// Set by user via rotary encoder
enum PLANT_NEEDS{
    TROPICAL,  // Humid, sunny
    DESERT,    // Dry, sunny
    TEMPERATE, // Humid, low light 
    ALPINE     // Dry, moderate light
};

enum LIGHT_LEVELS{LOW, MODERATE,HIGH};
enum WATER_LEVELS{DRY,HUMID};

void init_encoder();
void encoderEnable();
void encoderDisable();
uint16_t user_input(uint8_t);
