#define ENCODERA    (1 << PC3)
#define ENCODERB    (1 << PC2)

// Volatile encoder variables for interrupt
volatile uint8_t encoder_old_state, encoder_new_state; // state machine
volatile uint8_t encoder_changed; // flag
volatile uint8_t encoder_input, encoder_A, encoder_B; //rotary encoder inputs variables

void init_encoder();
void encoderEnable();
void encoderDisable();
