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
    if (size < 1) {
      head = p;
      tail = p;
    }
    if (size > 0) {
      p->prev = tail;
      tail->next = p;
      tail = p;
    }
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
  }

  iPosition* createIPosition(int position) {
    return new iPosition(position);
  }

  bool isAlreadyQueued(int position) {
    iPosition* temp = head;
    for (int i = 0; i < size; i++) {
      if (temp->position == position) {
        return true;
      }
      temp = temp->next;
    }
    return false;
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
const int fadeFactor = 8;                 //smaller step = smoother fade (was 30, looked choppy)
const unsigned long SCAN_INTERVAL = 120;  //ms between button scans (replaces blocking delay(150))
const unsigned long FADE_INTERVAL = 16;   //ms between fade steps (~60 FPS) for a smooth glide
const unsigned long WIN_EFFECT_MS = 8000; //duration of the win-state sparkle before settling

//Mutables variables
int rgb[3] = { 191, 0, 255 };          //Default RGB value for all LEDs. Bright neon purple.
int fadeStorage[INPUT_MATRIX];         //Array to hold all inputs
int inputStorage[NUM_SECRET_LETTERS];  //Array holding the valid inputs order of input
int fadeIndex = 0;                     //Keeps track of the number of inputs for the fade effect
int storageIndex = 0;                  //Keeps track of the number of valid inputs for the matching sequence

unsigned long lastTime = 0;
unsigned long lastScanTime = 0;  //timestamp of last button scan
unsigned long lastFadeTime = 0;  //timestamp of last fade step

String incoming = "";
bool puzzleSolved = false;

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
void winSparkle(unsigned long durationMs);
void losingResponse();
void handleCommand(String cmd);


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
  Serial.begin(9600);     //baud rate of USB serial communication
  Serial1.begin(115200);  //baud rate between Mega and ESP8266
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
  setRGB(191, 0, 255);  //Bright neon purple ambient color
}
void run() {
  
  while(Serial1.available()){
    char c = Serial1.read();
    if(c == '\n') {
      incoming.trim();
      handleCommand(incoming);
      incoming = "";
    } else {
      incoming += c;
    }
  }
  unsigned long now = millis();

  //Advance the fade on its own fast cadence so it glides smoothly
  //instead of jumping one big step per blocking loop.
  if (now - lastFadeTime >= FADE_INTERVAL) {
    lastFadeTime = now;
    fadeLEDs();
  }

  //Scan for button presses on a slower cadence (debounce) without
  //blocking the fade. Replaces the old delay(150).
  if (now - lastScanTime >= SCAN_INTERVAL) {
    lastScanTime = now;
    int pressedButtonPosition = scanForButtonPress();
    if (pressedButtonPosition >= 0) {
      turnOnLEDs(inputToLEDMapping(pressedButtonPosition));
      storeInput(pressedButtonPosition);

      if (haveWon()) {
        winningResponse();
        resetAll();
      }
    }
  }
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
  for (int y = 0; y < yWeight; y++)
    for (int x = 3; x < xWeight-2; x++) {             //added filter: start on 3, end on 11
      int idx = (position + (y * LEDS_PER_ROW)) + x;
      if (idx >= 0 && idx < TOTAL_LEDS)               //guard: never index leds[] out of bounds
        func(idx);
    }
  FastLED.show();
}
void setColorToLEDs(int ledPosition) {
  leds[ledPosition] = CRGB(rgb[0], rgb[1], rgb[2]);
}
void fadeOutLEDs(int ledPosition) {
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

  //Walk the queue. We must capture each node's successor BEFORE any
  //pop(), because pop() may delete the node temp points at (it pops
  //the head, FIFO) and dereferencing a freed node is undefined
  //behaviour. The loop count is also snapshotted up front so it is
  //not affected by pop() shrinking posQueue.size mid-walk.
  iPosition* temp = posQueue.head;
  int count = posQueue.size;
  for (int i = 0; i < count && temp != nullptr; i++) {
    iPosition* nextNode = temp->next;  //capture before a possible pop()

    int position = inputToLEDMapping(temp->position);
    updateLEDs(position, fadeOutLEDs);

    if (leds[position] == CRGB::Black) {
      posQueue.pop();  //pop the head, FIFO, therefore assuming head would be faded out
    }
    temp = nextNode;
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
  setRGB(255, 200, 30);  //Shimmering gold for the correct answer

  //8-second gold shimmer/sparkle reveal, then settle on solid gold.
  winSparkle(WIN_EFFECT_MS);
  turnOnAllLEDs();

  puzzleSolved = true;
  sendCommand("SOLVED");
}

//Animated gold shimmer: a slowly pulsing gold base with random
//bright sparkles flickering across the panel. Runs for durationMs.
void winSparkle(unsigned long durationMs) {
  const CRGB goldBase   = CRGB(255, 200, 30);   //shimmering gold
  const CRGB sparkleHot = CRGB(255, 245, 200);  //hot near-white gold for the sparkle pop
  unsigned long start = millis();

  while (millis() - start < durationMs) {
    //Gentle brightness pulse so the whole panel "breathes".
    uint8_t pulse = 150 + (sin8(millis() / 6) >> 2);  //~150-213 range
    FastLED.setBrightness(pulse);

    //Lay down the gold base every frame.
    fill_solid(leds, TOTAL_LEDS, goldBase);

    //Scatter a handful of bright sparkles each frame.
    for (int i = 0; i < 25; i++) {
      leds[random16(TOTAL_LEDS)] = sparkleHot;
    }

    FastLED.show();
    delay(20);  //~50 FPS shimmer
  }

  FastLED.setBrightness(brightness);  //restore normal brightness
}

void losingResponse() {
  Serial.println("You lose!");
  setRGB(255, 0, 0);
  for (int i = 0; i < 5; i++)
    blinkAllLEDs();
}

void sendCommand(String cmd){
  Serial1.println(cmd);
}
void handleCommand(String cmd){
  if(cmd == "PING"){
    sendCommand("PONG");
  } else if (cmd == "STATUS") {
    sendCommand(puzzleSolved ? "SOLVED" : "READY");
  } else if (cmd == "PUZZLE_RESET") {
    resetAll();
    sendCommand("RESET");
  }else if (cmd == "SOLVE") {
    puzzleSolved = true;
    sendCommand("MANUALLY_SOLVED");
  }

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
  puzzleSolved = false;
  sendCommand("RESET");
  //fadeIndex = 0;
}

void resetOutputPins() {
  for (int i = 0; i < NUM_COLS; i++) {
    digitalWrite(colPins[i], LOW);
  }
}
void resetAll() {
  setRGB(191, 0, 255);  //Bright neon purple ambient color
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
        //IMPORTANT: drive this column LOW before returning. The old
        //code returned with colPins[y] still HIGH; with the slow
        //delay(150) loop that cleared itself before it mattered, but
        //the faster decoupled loop leaves the column stuck HIGH so
        //the next scan sees TWO columns energised and reads a shifted
        //row/col -> press maps to the wrong LED block.
        digitalWrite(colPins[y], LOW);
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
  return (y + (x * yWeight * NUM_COLS)) * xWeight;

}
