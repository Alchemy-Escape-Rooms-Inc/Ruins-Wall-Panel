//MACROS
#include <FastLED.h>

#define NUM_ROWS 5
#define NUM_COLS 7
#define LEDS_PER_ROW NUM_COLS
#define LEDS_PER_COL NUM_ROWS
#define NUM_SECRET_LETTERS 7
#define LED_TYPE WS2812B 

//GLOBAL STRUCTS
struct SLED{
    uint8_t row = 0, col = 0;                                                                     //coordinate of centered LED 
    uint8_t cInput;                                                                               //corresponding input pin of the associated secret letter (a value from secretLetters array)
};

//GLOBAL VARIABLES
const uint8_t brightness = 128;                                                                   //half the full brightness
const uint8_t rgb[3] = {255,255,255};                                                             //Default RGB value for LEDs. White.
const uint8_t ledDataOutputPins[NUM_ROWS] =  {9,10,11,12,13};                                     //5 rows of led strips to update desired state(s) using designated output pins
const uint8_t secretLettersInputPins[NUM_SECRET_LETTERS] = {2,3,4,5,6,7,8};                       //7 secret letters corresponding to the 7 designated input pins              
const uint8_t xWeight = 4, yWeight = 3;                                                           //Weight of 8"x6" letter block (number of leds in one letter block in x and y direction). 

const SLED secretLetters[NUM_SECRET_LETTERS];                                                                         

bool correctLetterSequence[NUM_SECRET_LETTERS] = {false,false,false,false,false,false,false};     //boolean array for tracking the proper selection sequence

CRGB leds[LEDS_PER_ROW][LEDS_PER_COL];                                                            //Creation of array RGB structure to hold each LED value


//FUNCTION PROTOTYPES
void _init();
void gpio_init();
void led_init();

void turnOnLED(uint8_t row, uint8_t col);
void turnOnRow(uint8_t row);
void turnOffRow(uint8_t row);
void turnOnColumn(uint8_t col);
void turnOffColumn(uint8_t col);
void turnOnAllLEDs();
void turnOffAllLEDs();

//MAIN SETUP
void setup() {
  _init();
}

//MAIN LOOP
void loop() {
}


//FUNCTION DEFINITIONS
void _init(){
  gpio_init();
  led_init();
}
void gpio_init(){
  for(uint8_t i = 0; i < NUM_ROWS; i++)
    pinMode(ledDataOutputPins[i],OUTPUT);
  for(uint8_t i = 0; i < NUM_SECRET_LETTERS; i++)
    pinMode(secretLettersInputPins[i],INPUT);

}
void led_init(){
  //FastLED.addLeds<LED_TYPE,DATA_OUT,GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(brightness);
}

