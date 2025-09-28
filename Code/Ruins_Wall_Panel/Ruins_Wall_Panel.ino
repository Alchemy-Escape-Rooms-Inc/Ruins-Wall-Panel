//MACROS
#include <FastLED.h>

#define NUM_ROWS 3
#define NUM_COLS 7
#define LEDS_PER_ROW 20
#define LEDS_PER_COL 3
#define NUM_SECRET_LETTERS 7
#define LED_TYPE WS2812B 

//GLOBAL STRUCTS
struct COORDINATE {
    uint8_t row = 0, col = 0;                                                                     //coordinate of input 
};

//GLOBAL VARIABLES
const uint8_t brightness = 128;                                                                   //half the full brightness
const uint8_t rgb[3] = {255,255,255};                                                             //Default RGB value for all LEDs. White.
const uint8_t ledOutputPins[NUM_ROWS] =  {11,12,13};                                              //X rows of led strips to update desired state(s) using designated output pins
const uint8_t secretLettersPositions[NUM_SECRET_LETTERS] = {0,4,9,11,14,15,21}                    //Values corresponding to the location of the secret letters
const uint8_t correctSecretLettersSequence[NUM_SECRET_LETTERS] = {11,9,0,4,15,14,21};             //boolean array for tracking the proper selection sequence
//const uint8_t xWeight = 4, yWeight = 3;                                                         //Weight of 8"x6" letter block (number of leds in one letter block in x and y direction). 
const uint8_t rowPins[NUM_ROWS] = {2,3,4};                                                        //pins capturing/producing the row's level
const uint8_t colPins[NUM_COLS] = {5,6,7,8,9,10,A0};                                              //pins capturing/producing the columsn's level

uint8_t sequenceOfInputs[NUM_SECRET_LETTERS];                                                     //Array holding the order of the pressed inputs
uint8_t count = 0;                                                                                //Keeps track of the number of inputs

CRGB leds[LEDS_PER_ROW][LEDS_PER_COL];                                                            //Creation of array RGB structure to hold each LED value


//FUNCTION PROTOTYPES
void _init();
void gpio_init();
void led_init();
void run();

void setRGB(uint8_t red, uint8_t green, uint8_t blue);
void turnOnLED(uint8_t row, uint8_t col);
void turnOnRow(uint8_t row);
void turnOffRow(uint8_t row);
void turnOnColumn(uint8_t col);
void turnOffColumn(uint8_t col);
void turnOnAllLEDs();
void turnOffAllLEDs();
void strobeLED();
void fadeLED();

COORDINATE getCoordinate(uint8_t pos);

bool isAMatchingSequence(const uint8_t sequence);
bool noConflitingSecretLetters();

int8_t scanForButtonPress();


//MAIN SETUP
void setup() {
  _init();
}

//MAIN LOOP
void loop() {
  run();
}


//FUNCTION DEFINITIONS
void _init(){
  gpio_init();
  led_init();
}


void gpio_init(){
  //Setting the LEDs control pins to output 
  for(uint8_t i = 0; i < NUM_ROWS; i++)
    pinMode(ledOutputPins[i],OUTPUT);
  //Setting the rows contacts as read/inputs
  for(uint8_t i = 0; i < NUM_ROWS; i++)
    pinMode(rowPins,INPUT);
  //Setting the columns contacts as outputs
  for(uint8_t i = 0; i < NUM_COLS; i++)
    pinMode(colPins,OUTPUT);

}
void led_init(){
  //FastLED.addLeds<LED_TYPE,DATA_OUT,GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(brightness);
}

void run(){
  scanForButtonPress();
}

void setRGB(uint8_t red, uint8_t green, uint8_t blue){
  rgb[0] = red;
  rgb[1] = green;
  rgb[2] = blue;
}
void turnOnLED(uint8_t row, uint8_t col);
void turnOnRow(uint8_t row);
void turnOffRow(uint8_t row);
void turnOnColumn(uint8_t col);
void turnOffColumn(uint8_t col);
void turnOnAllLEDs(){
  //set the rgb colors, white.
  setRGB(255,255,255);  
}
void turnOffAllLEDs(){
  //set the rgb colors, black.
  setRGB(0,0,0);
}

void strobeLED();
void fadeLED();

COORDINATE getCoordinate(uint8_t pos) {
  //col + (row * NUM_COLS) = pos
  uint8_t row = 0, col = 0;
  while((row*NUM_COLS) < pos) row++;
  row--;  //go back 1, because 0 is inclusive
  col = (pos - (row * NUM_COLS)) - 1 ; //go back 1, because 0 is inclusive
  return {row,col}
}

bool isAMatchingSequence(const uint8_t sequence);
bool noConflitingSecretLetters(){
  //if no match is found, something is wrong.
  uint8_t matches = 0;
  for(uint8_t i = 0; i < NUM_SECRET_LETTERS; i++)
    for(uint8_t j = 0; j < NUM_SECRET_LETTERSl j++)
      matches += (correctSecretLettersSequence[i] == secretLettersPositions[j]) ?  1 : 0; 
  return (matches < NUM_SECRET_LETTERS) ? false : true;
}

int8_t scanForButtonPress(){
  //Apply voltage to a column and 
  //check to each row for voltage. 
  //Do this for each column. Return 
  //position if voltage is detected,
  //otherwise return a negative value.
  for(uint8_t i = 0; i < NUM_COLS; i++){
    digitalWrite(colPins[i],HIGH);
    for(uint8_t j = 0; j < NUM_ROWS; j++){
      bool pressed = (digitalRead(rowPins[j])) ? true : false;
      if(pressed) return j + (i * NUM_COLS); //returns position
    }
  }
  //nothing is pressed
  return -1;

}

