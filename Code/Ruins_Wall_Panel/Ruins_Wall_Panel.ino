//MACROS
#include <FastLED.h>

#define NUM_ROWS 3
#define NUM_COLS 7
#define LEDS_PER_ROW 35
#define LEDS_PER_COL LEDS_PER_ROW/NUM_COLS
#define TOTAL_LEDS (LEDS_PER_ROW * NUM_ROWS)
#define NUM_SECRET_LETTERS 7
#define LED_TYPE WS2812B

//------------GLOBAL VARIABLES------------------
//Immutables variables
const int brightness = 128;                                                                //half the full brightness
const int ledsDataPin = 2;                                                                //leds data pin
const int secretLettersPositions[NUM_SECRET_LETTERS] = { 0, 4, 9, 11, 14, 15, 20 };        //Values corresponding to the location of the secret letters
const int correctSecretLettersSequence[NUM_SECRET_LETTERS] = { 11, 9, 0, 4, 15, 14, 20 };  //boolean array for tracking the proper selection sequence
const int xWeight = LEDS_PER_COL;                                               //number of LEDs in a group to illuminate in a row (x)
const int yWeight = 1;                                                                      //number of LEDs in a group to illuminate in column(s) (y)
const int rWeight = LEDS_PER_ROW;                                                          //number of LEDs in each row
const int rowPins[NUM_ROWS] = { 3, 4, 5 };                                                 //pins capturing/producing the row's level
const int colPins[NUM_COLS] = { 6, 7, 8, 9, 10, 11, 12 };                                  //pins capturing/producing the columsn's level
//Mutables variables
int rgb[3] = { 255, 255, 255 };            //Default RGB value for all LEDs. White.
int sequenceOfInputs[NUM_SECRET_LETTERS];  //Array holding the order of the pressed inputs
int count = 0;                             //Keeps track of the number of inputs, used as index

CRGB leds[TOTAL_LEDS];  //Creation of array RGB structure to hold each LED value


//----------FUNCTION PROTOTYPES-----------------
void _init();
void gpio_init();
void led_init();
void run();
void setRGB(int red, int green, int blue);
void turnOnLEDs(int ledPosition);
void turnOnRow(int row);
void turnOffRow(int row);
void turnOnColumn(int col);
void turnOffColumn(int col);
void turnOnAllLEDs();
void turnOffAllLEDs();
void strobeLEDs(int ledPosition);
void fadeLEDs(int ledPosition);
void blinkLEDs(int ledPosition);
void blinkAllLeds();
void storeInput(int pos);
void winningResponse();
void losingResponse();
void resetAllParameters();
void resetAllOutputPins();

bool isAMatchingSequence();
bool noConflitingSecretLetters();
bool isValidInput(int pos);
bool isAlreadyStored(int pos);

int scanForButtonPress();

int inputToLEDMapping(int inputPosition);

//------------------MAIN SETUP-------------------
void setup() {
  Serial.begin(9600);
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
  for (int i = 0; i < NUM_ROWS; i++)
    pinMode(rowPins[i], INPUT);
  //Setting the columns contacts as outputs
  for (int i = 0; i < NUM_COLS; i++)
    pinMode(colPins[i], OUTPUT);


  //Setting the rows contacts as read/inputs
  for (int i = 0; i < NUM_ROWS; i++)
    digitalWrite(rowPins[i], LOW);
  //Setting the columns contacts as outputs
  for (int i = 0; i < NUM_COLS; i++)
    digitalWrite(colPins[i], LOW);


}
void led_init() {
  FastLED.addLeds<LED_TYPE, ledsDataPin, GRB>(leds, TOTAL_LEDS);
  FastLED.setBrightness(brightness);

  turnOffAllLEDs();
}
void run() {
  //Check for button press.
  //If no button pressed leave,
  //otherwise use position value
  //for lighting up corresponding
  //leds.
  //Track sequence of pressed
  //buttons.
  int pressedButtonPosition = scanForButtonPress();
  if (pressedButtonPosition < 0)
    return;

  Serial.print("Button pressed: ");
  Serial.println((int)pressedButtonPosition);

  turnOnLEDs(inputToLEDMapping(pressedButtonPosition));
  if (isValidInput(pressedButtonPosition))
    storeInput(pressedButtonPosition);


  if (count < 7){
    delay(1000);   //delay for a second
    return;
  }
  if (!isAMatchingSequence()) {
    //you lose
    losingResponse();
  } else {
    winningResponse();  //reset count to start grabbing new entries
  }

  resetAllParameters();
}
void setRGB(int red, int green, int blue) {
  rgb[0] = red;
  rgb[1] = green;
  rgb[2] = blue;
}
void turnOnLEDs(int ledPosition) {
  Serial.print("Turning on LEDs: ");
  for (int y = 0; y < yWeight; y++) {
    for (int x = 0; x < xWeight; x++) {
      Serial.print((ledPosition + (y * LEDS_PER_ROW)) + x);
      Serial.print(" ");
      leds[(ledPosition + (y * LEDS_PER_ROW)) + x] = CRGB(rgb[0], rgb[1], rgb[2]);
    }
  }
  Serial.println("");
  FastLED.show();
}
void turnOnRow(int row) {
  //Make sure all other LEDs are
  //turned off then proceed to
  //turn on LEDs in a single row.
  turnOffAllLEDs();
  for (int y = 0; y < LEDS_PER_ROW; y++) {
    leds[y + (row * LEDS_PER_ROW)] = CRGB(rgb[0], rgb[1], rgb[2]);
  }
  FastLED.show();
}
void turnOffRow(int row) {
  //Change row of LEDs color to
  //black and show.
  for (int y = 0; y < LEDS_PER_ROW; y++) {
    leds[y + (row * LEDS_PER_ROW)] = CRGB::Black;
  }
  FastLED.show();
}
void turnOnColumn(int col) {
  //Make sure all other LEDs are
  //turned off then proceed to
  //turn on LEDs in a column.
  turnOffAllLEDs();
  for (int x = 0; x < NUM_ROWS; x++) {
    for (int y = 0; y < xWeight; y++) {
      leds[(col * xWeight) + (x * rWeight) + y] = CRGB(rgb[0], rgb[1], rgb[2]);
    }
  }
  FastLED.show();
}
void turnOffColumn(int col) {
  //Change columns of LEDs color to
  //black and show.
  for (int x = 0; x < NUM_ROWS; x++) {
    for (int y = 0; y < xWeight; y++) {
      leds[(col * xWeight) + (x * rWeight) + y] = CRGB::Black;
    }
  }
  FastLED.show();
}
void turnOnAllLEDs() {
  fill_solid(leds, TOTAL_LEDS, CRGB(rgb[0], rgb[1], rgb[2]));  // Use fill_solid to set all LEDs
  FastLED.show();
}
void turnOffAllLEDs() {
  FastLED.clear();
  FastLED.show();
}
void strobeLEDs(int ledPosition) {
  //Start with a single LED
  //move down the path to the
  //next sets of LEDs, while
  //fading the LEDs prior off.
}
void fadeLEDs(int ledPosition) {
  //Illuminate a group of LED
  //with the highest of brightness
  //and lower the brightness until
  //LEDs are off.
}
void blinkLEDs(int ledPosition) {
  //Illuminate a group of LED
  //then turn it off and a determine
  //interval.
}
void blinkAllLEDs() {
  turnOnAllLEDs();
  delay(500);
  turnOffAllLEDs();
  delay(500);
}
void storeInput(int pos) {
  if (isAlreadyStored(pos)) {
    return;
  }
  Serial.print(pos);
  Serial.println(" is being stored.");
  sequenceOfInputs[count] = pos;
  count++;
}

void winningResponse() {
  Serial.println("You won.");
  setRGB(0, 255, 0);
  for (int i = 0; i < 5; i++) { blinkAllLEDs(); }
}

void losingResponse() {
  Serial.println("You lose!");
  setRGB(255, 0, 0);
  for (int i = 0; i < 5; i++) { blinkAllLEDs(); }
}

void resetAllParameters() {
  setRGB(255, 255, 255);
  count = 0;
  resetAllOutputPins();

}

void resetAllOutputPins() {
  for (int i = 0; i < NUM_COLS; i++) {
    digitalWrite(colPins[i], LOW);
  }
}

bool isAMatchingSequence() {
  for (int i = 0; i < NUM_SECRET_LETTERS; i++)
    if (correctSecretLettersSequence[i] != sequenceOfInputs[i]) return false;
  return true;
}
bool noConflitingSecretLetters() {
  //if no match is found, something is wrong.
  int matches = 0;
  for (int i = 0; i < NUM_SECRET_LETTERS; i++)
    for (int j = 0; j < NUM_SECRET_LETTERS; j++)
      matches += (correctSecretLettersSequence[i] == secretLettersPositions[j]) ? 1 : 0;
  return (matches < NUM_SECRET_LETTERS) ? false : true;
}
bool isValidInput(int pos) {
  for (int i = 0; i < NUM_SECRET_LETTERS; i++) {
    if (secretLettersPositions[i] == pos) {
      Serial.println("A valid input");
      return true;
    }
  }
  Serial.println("Is not a valid input");

  return false;
}
bool isAlreadyStored(int pos) {
  for (int i = 0; i < count; i++) {
    if (pos == sequenceOfInputs[i]) {
      Serial.print(pos);
      Serial.println(" was already stored.");
      return true;
    }
  }
  return false;
}
int scanForButtonPress() {
  //Apply voltage to a column and
  //check to each row for voltage.
  //Do this for each column. Return
  //position if voltage is detected,
  //otherwise return a negative value.
  for (int y = 0; y < NUM_COLS; y++) {
   
    digitalWrite(colPins[y], HIGH);
     /*
    Serial.print("Column: ");
    Serial.print(y);
    Serial.println(((digitalRead(colPins[y])) ? " is now HIGH." : " still LOW."));
    */
    for (int x = 0; x < NUM_ROWS; x++) {
      /*
      Serial.print("Col: ");
      Serial.print(y);
      Serial.print(" Row: ");
      Serial.print(x);
      */
      bool pressed = (digitalRead(rowPins[x])) ? true : false;
      if (pressed) {
        Serial.print("Button pressed: ");
        Serial.println(y + (x * NUM_COLS));
        return y + (x * NUM_COLS);  //returns position
      }
      //Serial.println("");
      delay(50);
    }

    digitalWrite(colPins[y], LOW);

    /*
    Serial.print("Column: ");
    Serial.print(y);
    Serial.println(((digitalRead(colPins[y])) ? " is still HIGH." : " is now LOW."));
    */
  }
  resetAllOutputPins();
  //nothing is pressed
  return -1;
}
int inputToLEDMapping(int inputPosition) {
  return inputPosition * xWeight;
}