//MACROS
#include <FastLED.h>

#define NUM_ROWS 3
#define NUM_COLS 7
#define LEDS_PER_ROW 20
#define LEDS_PER_COL 1
#define TOTAL_LEDS (LEDS_PER_ROW * NUM_ROWS)
#define NUM_SECRET_LETTERS 7
#define LED_TYPE WS2812B

//------------GLOBAL VARIABLES------------------
//Immutables variables
const uint8_t brightness = 128;                                                                //half the full brightness
const uint8_t ledsDataPin = 13;                                                                //leds data pin
const uint8_t secretLettersPositions[NUM_SECRET_LETTERS] = { 0, 4, 9, 11, 14, 15, 21 };        //Values corresponding to the location of the secret letters
const uint8_t correctSecretLettersSequence[NUM_SECRET_LETTERS] = { 11, 9, 0, 4, 15, 14, 21 };  //boolean array for tracking the proper selection sequence
const uint8_t xWeight = LEDS_PER_ROW / NUM_COLS;                                               //number of LEDs in a group to illuminate in a row (x)
const uint8_t yWeight = LEDS_PER_COL;                                                          //number of LEDs in a group to illuminate in column(s) (y)
const uint8_t rWeight = LEDS_PER_ROW;                                                          //number of LEDs in each row
const uint8_t rowPins[NUM_ROWS] = { 2, 3, 4 };                                                 //pins capturing/producing the row's level
const uint8_t colPins[NUM_COLS] = { 5, 6, 7, 8, 9, 10, 11};                                    //pins capturing/producing the columsn's level
//Mutables variables
uint8_t rgb[3] = { 255, 255, 255 };                                                            //Default RGB value for all LEDs. White.
uint8_t sequenceOfInputs[NUM_SECRET_LETTERS];                                                  //Array holding the order of the pressed inputs
uint8_t count = 0;                                                                             //Keeps track of the number of inputs, used as index

CRGB leds[TOTAL_LEDS];                                                                          //Creation of array RGB structure to hold each LED value


//----------FUNCTION PROTOTYPES-----------------
void _init();
void gpio_init();
void led_init();
void run();
void setRGB(uint8_t red, uint8_t green, uint8_t blue);
void turnOnLEDs(uint8_t ledPosition);
void turnOnRow(uint8_t row);
void turnOffRow(uint8_t row);
void turnOnColumn(uint8_t col);
void turnOffColumn(uint8_t col);
void turnOnAllLEDs();
void turnOffAllLEDs();
void strobeLEDs(uint8_t ledPosition);
void fadeLEDs(uint8_t ledPosition);
void blinkLEDs(uint8_t ledPosition);
void storeInput(uint8_t pos);

bool isAMatchingSequence();
bool noConflitingSecretLetters();
bool isValidInput(uint8_t pos);

int8_t scanForButtonPress();

uint8_t inputToLEDMapping(uint8_t inputPosition);

//------------------MAIN SETUP-------------------
void setup() {
  _init();
}
//------------------MAIN LOOP--------------------
void loop() {
  run();
}


//---------------FUNCTION DEFINITIONS------------
void _init() {
  gpio_init();
  led_init();
}
void gpio_init() {
  //Setting the LEDs control pin to output
  pinMode(ledsDataPin, OUTPUT);
  //Setting the rows contacts as read/inputs
  for (uint8_t i = 0; i < NUM_ROWS; i++)
    pinMode(rowPins, INPUT);
  //Setting the columns contacts as outputs
  for (uint8_t i = 0; i < NUM_COLS; i++)
    pinMode(colPins, OUTPUT);
}
void led_init() {    
  FastLED.addLeds<LED_TYPE,ledsDataPin, GRB>(leds, TOTAL_LEDS);
  FastLED.setBrightness(brightness);
}
void run() {
  //Check for button press.
  //If no button pressed leave,
  //otherwise use position value
  //for lighting up corresponding
  //leds.
  //Track sequence of pressed
  //buttons.
  int8_t pressedButtonPosition = scanForButtonPress();
  if (pressedButtonPosition < 0)
    return;
  turnOnLEDs(inputToLEDMapping(pressedButtonPosition));
  if (isValidInput(pressedButtonPosition))
    storeInput(pressedButtonPosition);

  //COORDINATE coordinate = getCoordinate(pressedButtonPosition);

  if (count < 6)
    return;
  if (!isAMatchingSequence())
    count = 0;  //reset count to start grabbing new entries

  //may want to add indicator to show invalid sequence
  //such as the pressed corresponding LEDs are blinking red
}
void setRGB(uint8_t red, uint8_t green, uint8_t blue) {
  rgb[0] = red;
  rgb[1] = green;
  rgb[2] = blue;
}
void turnOnLEDs(uint8_t ledPosition) {
  for (uint8_t x = 0; x < xWeight; x++)
    for (uint8_t y = 0; y < yWeight; y++)
      leds[(ledPosition + (x * LEDS_PER_ROW)) + y] = CRGB(rgb[0], rgb[1], rgb[2]);
  FastLED.show();
}
void turnOnRow(uint8_t row) {
  //Make sure all other LEDs are
  //turned off then proceed to
  //turn on LEDs in a single row.
  turnOffAllLEDs();
  for (int8_t y = 0; y < LEDS_PER_ROW; y++)
    leds[y+(row * LEDS_PER_ROW)] = CRGB(rgb[0], rgb[1], rgb[2]);
  FastLED.show();
}
void turnOffRow(uint8_t row) {
  //Change row of LEDs color to
  //black and show.
  for (uint8_t y = 0; y < LEDS_PER_ROW; y++)
    leds[y+(row * LEDS_PER_ROW)] = CRGB::Black;
  FastLED.show();
}
void turnOnColumn(uint8_t col) {
  //Make sure all other LEDs are
  //turned off then proceed to
  //turn on LEDs in a column.
  turnOffAllLEDs();
  for (uint8_t x = 0; x < NUM_ROWS; x++)
    for (uint8_t y = 0; y < xWeight; y++)
      leds[(col * xWeight)+(x*rWeight)+ y] = CRGB(rgb[0], rgb[1], rgb[2]);
  FastLED.show();
}
void turnOffColumn(uint8_t col) {
  //Change columns of LEDs color to
  //black and show.
  for (uint8_t x = 0; x < NUM_ROWS; x++)
    for (uint8_t y = 0; y < xWeight; y++)
      leds[(col * xWeight)+(x*rWeight)+ y] = CRGB::Black;
  FastLED.show();
}
void turnOnAllLEDs() {
  fill_solid(leds, TOTAL_LEDS, CRGB(rgb[0],rgb[1],rgb[2])); // Use fill_solid to set all LEDs
  FastLED.show();
}
void turnOffAllLEDs() {
  FastLED.clear();
  FastLED.show();
}
void strobeLEDs(uint8_t ledPosition) {
  //Start with a single LED
  //move down the path to the
  //next sets of LEDs, while
  //fading the LEDs prior off.
}
void fadeLEDs(uint8_t ledPosition) {
  //Illuminate a group of LED
  //with the highest of brightness
  //and lower the brightness until
  //LEDs are off.
}
void blinkLEDs(uint8_t ledPosition) {
  //Illuminate a group of LED
  //then turn it off and a determine
  //interval.
}
void storeInput(uint8_t pos) {
  sequenceOfInputs[count] = pos;
  count++;
}

bool isAMatchingSequence() {
  for (uint8_t i = 0; i < NUM_SECRET_LETTERS; i++)
    if (correctSecretLettersSequence[i] != sequenceOfInputs[i]) return false;
  return true;
}
bool noConflitingSecretLetters() {
  //if no match is found, something is wrong.
  uint8_t matches = 0;
  for (uint8_t i = 0; i < NUM_SECRET_LETTERS; i++)
    for (uint8_t j = 0; j < NUM_SECRET_LETTERS; j++)
      matches += (correctSecretLettersSequence[i] == secretLettersPositions[j]) ? 1 : 0;
  return (matches < NUM_SECRET_LETTERS) ? false : true;
}
bool isValidInput(uint8_t pos) {
  for (uint8_t i = 0; i < NUM_SECRET_LETTERS; i++)
    if (secretLettersPositions[i] == pos) return true;
  return false;
}
int8_t scanForButtonPress() {
  //Apply voltage to a column and
  //check to each row for voltage.
  //Do this for each column. Return
  //position if voltage is detected,
  //otherwise return a negative value.
  for (uint8_t i = 0; i < NUM_COLS; i++) {
    digitalWrite(colPins[i], HIGH);
    for (uint8_t j = 0; j < NUM_ROWS; j++) {
      bool pressed = (digitalRead(rowPins[j])) ? true : false;
      if (pressed) return j + (i * NUM_COLS);  //returns position
    }
  }
  //nothing is pressed
  return -1;
}
uint8_t inputToLEDMapping(uint8_t inputPosition) {
  return inputPosition * xWeight;
}