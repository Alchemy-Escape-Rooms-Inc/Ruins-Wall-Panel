//MACROS
#include <FastLED.h>

#define NUM_ROWS 5
#define NUM_COLS 7
#define INPUT_MATRIX (NUM_ROWS * NUM_COLS)
#define LEDS_PER_ROW 91
#define LEDS_PER_COL LEDS_PER_ROW / NUM_COLS
#define TOTAL_LEDS (LEDS_PER_ROW * NUM_ROWS * 2)
#define NUM_SECRET_LETTERS 7
#define LEDS_DATA_PIN A0
#define LED_TYPE WS2812B




//-------------DATA STRUCTURE----------------

struct iPosition {
  int position;
  iPosition *next, *prev;
  iPosition() = delete;
  iPosition(int pos)
    : position(pos) {}
};

struct sQueue {
  iPosition* head = nullptr;
  iPosition* tail = nullptr;
  int size = 0;
  ~sQueue() {
    int s = size;
    for (int i = 0; i < s; i++)
      pop();
  }
  /*
  void swap(iPosition & p1, iPosition & p2){
    //if 1 or less elements in queue, the arguments aren't valid, leave.
    if(size < 2)
      return;

    iPosition temp;

    temp.next = p1.next;
    temp.prev = p1.prev;

    //get the surrounding positions to point at the new position
    p1.prev.next = p2; 
    p1.next.prev = p2;
    p2.next.prev = p1;
    p2.prev.next = p1;

    //swap out the position's pointers
    p1.next = p2.next;
    p1.prev = p2.prev;
    p2.next = temp.next;
    p2.prev = temp.prev;


  }
  */
  void push(int position) {
    if (isAlreadyQueued(position))
      pop(position);
    iPosition * p = createIPosition(position);
    Serial.print((p->position == position) ? "Successfully " : "Unsuccessfully ");
    Serial.print("created the iPosition for position: ");
    Serial.println(position);
    if (size < 1) {
      head = p;
      tail = p;
    }
    if (size > 0) {
      p->prev = tail;
      tail->next = p;
      tail = p;
    }
    Serial.print("Pushed position ");
    Serial.print(position);
    Serial.println(" into the queue.");
    size++;
  }

  void pop() {
    if (size < 1)
      return;
    if (size == 1) {
      delete head;
    }
    //assign the head to the next in line, a drop the first
    if (size > 1) {
      iPosition* temp = head;
      head = head->next;
      head->prev->next = nullptr;
      head->prev = nullptr;
      delete temp;
    }
    size--;
    Serial.println("Popped head from queue.");
  }

  void pop(int position) {
    //sQueue size is 0, position can't be in the queue
    if (size < 1)
      return;
    //first find the iPosition
    iPosition* temp = head;
    for (int i = 0; i < size; i++) {
      if (temp->position == position)
        break;  //leaves for loop scope
      temp = temp->next;
    }
    //now remove iPosition from queue
    if (temp == head) {
      pop();
    } else {
      temp->next->prev = temp->prev;
      temp->prev->next = temp->next;
      temp->next = nullptr;
      temp->prev = nullptr;
      delete temp;  //may not be necessary wasn't dynamically allocated
    }
    size--;
    Serial.print("Popped position ");
    Serial.print(position);
    Serial.println(" from queue.");
  }

  iPosition* createIPosition(int position) {
    Serial.print("Creating iPosition for position: ");
    Serial.println(position);
    return new iPosition(position);
  }

  bool isAlreadyQueued(int position) {
    iPosition* temp = head;
    for (int i = 0; i < size; i++) {
      Serial.print("Temp position: ");
      Serial.println(temp->position);
      if (temp->position == position) {
        Serial.print("Position ");
        Serial.print(position);
        Serial.println(" is already queued.");
        return true;
      }
      temp = temp->next;
    }
    Serial.print("Position ");
    Serial.print(position);
    Serial.println(" is not already queued.");
    return false;
  }

  void printQueue() {
    if (size < 1)
      return;
    iPosition* temp = head;
    Serial.print("Queues content: ");
    for (int i = 0; i < size; i++) {
      Serial.print(temp->position);
      Serial.print(" ");
      temp = temp->next;
    }
    Serial.println(" ");
  }
};


//------------GLOBAL VARIABLES------------------
//Immutables variables
const int brightness = 128;                                                                //half the full brightness
const int correctSecretLettersSequence[NUM_SECRET_LETTERS] = { 8, 22, 10, 26, 4, 13, 31};  //boolean array for tracking the proper selection sequence
const int xWeight = LEDS_PER_COL;                                                         //number of LEDs in a group to illuminate in a row (x)
const int yWeight = 2;                                                                     //number of LEDs in a group to illuminate in column(s) (y)
const int rWeight = LEDS_PER_ROW;                                                          //number of LEDs in each row
const int rowPins[NUM_ROWS] = { 9, 10, 11, 12, 13 };                                       //pins capturing/producing the row's level
const int colPins[NUM_COLS] = { 2, 3, 4, 5, 6, 7, 8 };                                  //pins capturing/producing the columsn's level
const int fadeFactor = 30;

//Mutables variables
int rgb[3] = { 255, 255, 255 };        //Default RGB value for all LEDs. White.
int fadeStorage[INPUT_MATRIX];         //Array to hold all inputs
int inputStorage[NUM_SECRET_LETTERS];  //Array holding the valid inputs order of input
int fadeIndex = 0;                     //Keeps track of the number of inputs for the fade effect
int storageIndex = 0;                  //Keeps track of the number of valid inputs for the matching sequence

unsigned long lastTime = 0;


sQueue posQueue;

CRGB leds[TOTAL_LEDS];  //Creation of array RGB structure to hold each LED value

//----------FUNCTION PROTOTYPES-----------------
void _init();
void param_init();
void gpio_init();
void led_init();

void run();

void setRGB();
void turnOnLEDs(int ledPosition);
void updateLEDs(int ledPosition, void (*func)(int));
void setColorToLEDs(int ledPosition);
void fadeOutLEDs();
void fadeLEDs();
void turnOnAllLEDs();
void turnOffAllLEDs();
void blinkAllLeds();

void storeInput(int pos);

void winningResponse();
void losingResponse();

void resetInputStorage();
void resetFadeStorage();
void resetParameters();
void resetOutputPins();
void resetAll();

bool haveWon();

int scanForButtonPress();
int inputToLEDMapping(int inputPosition);
int inputToXY(int inputPosition);

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
  param_init();
  gpio_init();
  led_init();
}

void param_init() {
  resetParameters();
}

void gpio_init() {
  //Setting the LEDs control pin to output
  pinMode(LEDS_DATA_PIN, OUTPUT);
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
  FastLED.addLeds<LED_TYPE, LEDS_DATA_PIN, GRB>(leds, TOTAL_LEDS);
  FastLED.setBrightness(128);
  turnOffAllLEDs();
  setRGB(255, 255, 255);
}
void run() {
  
  fadeLEDs();
  
  posQueue.printQueue();
  int pressedButtonPosition = scanForButtonPress();
  if (pressedButtonPosition < 0)
    return;
  turnOnLEDs(inputToLEDMapping(pressedButtonPosition));
  storeInput(pressedButtonPosition);

  if (haveWon()) {
    winningResponse();
    resetAll();
  }
  delay(150);
}

void setRGB(int red, int green, int blue) {
  rgb[0] = red;
  rgb[1] = green;
  rgb[2] = blue;
}

void turnOnLEDs(int ledPosition) {
  updateLEDs(ledPosition, setColorToLEDs);
}

void updateLEDs(int position, void (*func)(int)) {
  Serial.print("Updating LEDs: ");
  for (int y = 0; y < yWeight; y++)
    for (int x = 3; x < xWeight-2; x++)               //added filter: start on 3, end on 11
      func((position + (y * LEDS_PER_ROW)) + x);
  FastLED.show();
  Serial.println("");
}
void setColorToLEDs(int ledPosition) {
  Serial.print(" s");
  Serial.print(ledPosition);
  leds[ledPosition] = CRGB(rgb[0], rgb[1], rgb[2]);
}
void fadeOutLEDs(int ledPosition) {
  Serial.print(" f");
  Serial.print(ledPosition);
  leds[ledPosition].fadeToBlackBy(fadeFactor);
}

void fadeLEDs() {
  /*
  for(int i = 0; i < INPUT_MATRIX; i++){
    if (fadeStorage[i] < 0)
      return;
    updateLEDs(inputToLEDMapping(fadeStorage[i]), fadeOutLEDs);
  }
  */
  //No positions are in the queue, therefore leave
  if (posQueue.size < 1)
    return;
  iPosition* temp = posQueue.head;
  for (int i = 0; i < posQueue.size; i++) {
    int position = inputToLEDMapping(temp->position);
    updateLEDs(position, fadeOutLEDs);

    if (leds[position] == CRGB::Black) {
      Serial.println("Attempting to pop the head of queue.");
      posQueue.pop();  //pop the head, FIFO, therefore assuming head would be faded out
      //posQueue.pop(position);
      Serial.println("Successfully popped the head of the queue.");
    }
    temp = temp->next;
    Serial.println((int)leds[position].getAverageLight());
  }
}

void turnOnAllLEDs() {
  fill_solid(leds, TOTAL_LEDS, CRGB(rgb[0], rgb[1], rgb[2]));
  FastLED.show();
}

void turnOffAllLEDs() {
  FastLED.clear();
  FastLED.show();
}

void blinkAllLEDs() {
  turnOnAllLEDs();
  delay(500);
  turnOffAllLEDs();
  delay(500);
}
void storeInput(int pos) {
  //store the input position in the sQueue for fade effect
  posQueue.push(pos);

  //store the input position in the input storage for fade effect
  /*
  if(fadeIndex == (INPUT_MATRIX-1)) {
    fadeIndex = 0;
    resetFadeStorage();
  }
  fadeStorage[fadeIndex] = pos;
  fadeIndex++;
  */
  //check of the matching position for the current storage index
  //and update if necessary
  if (pos == correctSecretLettersSequence[storageIndex]) {
    inputStorage[storageIndex] = pos;
    storageIndex++;
  } else {
    storageIndex = 0;
    resetInputStorage();
  }
}

void winningResponse() {
  Serial.println("You won.");
  setRGB(0, 255, 0);
  for (int i = 0; i < 5; i++)
    blinkAllLEDs();
}

void losingResponse() {
  Serial.println("You lose!");
  setRGB(255, 0, 0);
  for (int i = 0; i < 5; i++)
    blinkAllLEDs();
}

void resetInputStorage() {
  for (int i = 0; i < NUM_SECRET_LETTERS; i++)
    inputStorage[i] = -1;
}
/*
void resetFadeStorage() {
  for (int i = 0; i < INPUT_MATRIX; i++)
    fadeStorage[i] = -1;
}*/
void resetParameters() {
  void resetInputStorage();
  //void resetFadeStorage();
  storageIndex = 0;
  //fadeIndex = 0;
}

void resetOutputPins() {
  for (int i = 0; i < NUM_COLS; i++) {
    digitalWrite(colPins[i], LOW);
  }
}
void resetAll() {
  setRGB(255, 255, 255);
  resetOutputPins();
  resetParameters();
}

bool haveWon() {
  return (storageIndex < NUM_SECRET_LETTERS) ? false : true;
}

int scanForButtonPress() {
  //Apply voltage to a column and
  //check to each row for voltage.
  //Do this for each column. Return
  //position if voltage is detected,
  //otherwise return a negative value.
  for (int y = 0; y < NUM_COLS; y++) {
    digitalWrite(colPins[y], HIGH);
    for (int x = 0; x < NUM_ROWS; x++) {
      bool pressed = (digitalRead(rowPins[x])) ? true : false;
      if (pressed) {
        Serial.print("Button pressed: ");
        Serial.println(y + (x * NUM_COLS));
        return y + (x * NUM_COLS);
      }
    }
    digitalWrite(colPins[y], LOW);
  }
  resetOutputPins();
  //nothing is pressed
  return -1;
}

int inputToLEDMapping(int inputPosition) {
  //x = row y = col
  int x=0, y=0;
  for(int i = 0; i < NUM_ROWS; i++){
    if((i+1)* NUM_COLS > inputPosition){
      x = i;
      break;
    }
  }
  y = inputPosition - (x * NUM_COLS); 
  Serial.print("Row: ");
  Serial.print(x);
  Serial.print(" Col: ");
  Serial.println(y);
  return (y + (x * yWeight * NUM_COLS)) * xWeight;  

}
