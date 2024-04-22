// Adapted from Adafruit GitHub library
// https://github.com/adafruit/Adafruit_TSL2591_Library/blob/master/Adafruit_TSL2591.cpp#L467 


#define TSL2591_COMMAND_BIT                                                    \
  (0xA0) ///< 1010 0000: bits 7 and 5 for 'command normal'

#define TSL2591_ENABLE_POWEROFF (0x00) ///< Flag for ENABLE register to disable
#define TSL2591_ENABLE_POWERON (0x01)  ///< Flag for ENABLE register to enable
#define TSL2591_ENABLE_AEN                                                     \
  (0x02) ///< ALS Enable. This field activates ALS function. Writing a one
         ///< activates the ALS. Writing a zero disables the ALS.
#define TSL2591_ENABLE_AIEN                                                    \
  (0x10) ///< ALS Interrupt Enable. When asserted permits ALS interrupts to be
         ///< generated, subject to the persist filter.
#define TSL2591_ENABLE_NPIEN                                                   \
  (0x80) ///< No Persist Interrupt Enable. When asserted NP Threshold conditions
         ///< will generate an interrupt, bypassing the persist filter


/// TSL2591 Register map
enum {
  TSL2591_REGISTER_ENABLE = 0x00,          // Enable register
  TSL2591_REGISTER_CONTROL = 0x01,         // Control register
  TSL2591_REGISTER_DEVICE_ID = 0x12,      // Device Identification
  TSL2591_REGISTER_CHAN0_LOW = 0x14,      // Channel 0 data, low byte
  TSL2591_REGISTER_CHAN1_LOW = 0x16,      // Channel 1 data, low byte
};

/// Enumeration for the sensor integration timing
typedef enum {
  TSL2591_INTEGRATIONTIME_100MS = 0x00, // 100 millis
  TSL2591_INTEGRATIONTIME_200MS = 0x01, // 200 millis
  TSL2591_INTEGRATIONTIME_300MS = 0x02, // 300 millis
  TSL2591_INTEGRATIONTIME_400MS = 0x03, // 400 millis
  TSL2591_INTEGRATIONTIME_500MS = 0x04, // 500 millis
  TSL2591_INTEGRATIONTIME_600MS = 0x05, // 600 millis
} tsl2591IntegrationTime_t;

/// Enumeration for the sensor gain
typedef enum {
  TSL2591_GAIN_LOW = 0x00,  /// low gain (1x)
  TSL2591_GAIN_MED = 0x10,  /// medium gain (25x)
  TSL2591_GAIN_HIGH = 0x20, /// medium gain (428x)
  TSL2591_GAIN_MAX = 0x30,  /// max gain (9876x)
} tsl2591Gain_t;

uint8_t configure_lightsensor();
uint8_t enable_lightsensor();
uint8_t init_lightsensor();
uint16_t get_luminosity();
float calculate_lux(uint16_t, uint16_t);

uint8_t _integration;

// #endif