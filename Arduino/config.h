//////////////////////////// ------ ////////////////////////////
#define FIRST_PIN 4     // Pino onde está o primeiro relê
#define PINS_COUNT 1    // Quantos pinos serão utilizados


// Comenta essa linha se nao for usar Anodo_Comum
#define COMMON_ANODE

#ifdef COMMON_ANODE
  #define LED_ON LOW
  #define LED_OFF HIGH
#else
  #define LED_ON HIGH
  #define LED_OFF LOW
#endif


// Set Led Pins
#define redLed 7    
#define greenLed 6
#define blueLed 5


// Set Relay Pin
#define relay 4


// Button pin for WipeMode
#define wipeB 3


// MFRC522 instance pin config
#define SS_PIN 10
#define RST_PIN 9
