// MACROS
#include <FastLED.h>

#define NUM_ROWS 5
#define NUM_COLS 7
#define INPUT_MATRIX (NUM_ROWS * NUM_COLS)
#define LEDS_PER_ROW 91
#define LEDS_PER_COL (LEDS_PER_ROW / NUM_COLS)
#define TOTAL_LEDS (LEDS_PER_ROW * NUM_ROWS * 2)
#define NUM_SECRET_LETTERS 7
#define LEDS_DATA_PIN A0
#define LED_TYPE WS2812B
#define DEBUG_ENABLED true

//-------------DEBUG HELPERS----------------

void debugLine(const String& msg) {
  if (!DEBUG_ENABLED) return;
  Serial.print("[DBG ");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.println(msg);
}

void debugKV(const String& label, int value) {
  if (!DEBUG_ENABLED) return;
  Serial.print("[DBG ");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.print(label);
  Serial.print(": ");
  Serial.println(value);
}

//-------------DATA STRUCTURE----------------

struct iPosition {
  int position;
  unsigned long pushedAt;
  iPosition *next, *prev;
  iPosition() = delete;
  iPosition(int pos)
    : position(pos), pushedAt(millis()), next(nullptr), prev(nullptr) {}
};

struct sQueue {
  iPosition* head = nullptr;
  iPosition* tail = nullptr;
  int size = 0;

  ~sQueue() {
    clear();
  }

  void push(int position) {
    if (isAlreadyQueued(position)) {
      debugLine(String("Queue already has position ") + position + "; refreshing fade timer.");
      pop(position);
    }

    iPosition* p = new iPosition(position);
    if (!p) {
      debugLine("ERROR: Failed to allocate queue node.");
      return;
    }

    if (size == 0) {
      head = p;
      tail = p;
    } else {
      p->prev = tail;
      tail->next = p;
      tail = p;
    }

    size++;
    debugLine(String("Queued fade position ") + position + ", queue size now " + size + ".");
  }

  void pop() {
    if (size < 1 || head == nullptr) return;

    iPosition* temp = head;
    int removedPosition = temp->position;
    head = head->next;

    if (head) {
      head->prev = nullptr;
    } else {
      tail = nullptr;
    }

    delete temp;
    size--;
    debugLine(String("Popped queue head position ") + removedPosition + ", queue size now " + size + ".");
  }

  void pop(int position) {
    iPosition* temp = head;
    while (temp && temp->position != position) {
      temp = temp->next;
    }

    if (!temp) {
      debugLine(String("Queue pop ignored; position not found: ") + position);
      return;
    }

    if (temp->prev) {
      temp->prev->next = temp->next;
    } else {
      head = temp->next;
    }

    if (temp->next) {
      temp->next->prev = temp->prev;
    } else {
      tail = temp->prev;
    }

    delete temp;
    size--;
    debugLine(String("Removed queued position ") + position + ", queue size now " + size + ".");
  }

  void clear() {
    while (size > 0) {
      pop();
    }
    head = nullptr;
    tail = nullptr;
    size = 0;
    debugLine("Fade queue cleared.");
  }

  bool isAlreadyQueued(int position) {
    for (iPosition* temp = head; temp != nullptr; temp = temp->next) {
      if (temp->position == position) return true;
    }
    return false;
  }

  void printQueue() {
    if (size < 1) {
      debugLine("Queue is empty.");
      return;
    }

    Serial.print("[DBG ");
    Serial.print(millis());
    Serial.print(" ms] Queue contents: ");
    for (iPosition* temp = head; temp != nullptr; temp = temp->next) {
      Serial.print(temp->position);
      Serial.print(" ");
    }
    Serial.println();
  }
};

//------------GLOBAL VARIABLES------------------

const int brightness = 128;
const int correctSecretLettersSequence[NUM_SECRET_LETTERS] = { 8, 22, 10, 26, 4, 13, 31 };
const char* const correctSecretLettersSymbols[NUM_SECRET_LETTERS] = { "Tail", "Turtle", "Monkey", "Seahorse", "Star", "Ship", "Trident" };
const int xWeight = LEDS_PER_COL;
const int yWeight = 2;
const int rowPins[NUM_ROWS] = { 9, 10, 11, 12, 13 };
const int colPins[NUM_COLS] = { 2, 3, 4, 5, 6, 7, 8 };
const int fadeFactor = 12;
const unsigned long fadeIntervalMs = 20;
const unsigned long fadeDurationMs = 600;

int rgb[3] = { 191, 0, 255 };
int inputStorage[NUM_SECRET_LETTERS];
int storageIndex = 0;
int lastPressedPosition = -1;

unsigned long lastTime = 0;

String incoming = "";
bool puzzleSolved = false;

sQueue posQueue;

CRGB leds[TOTAL_LEDS];

//----------FUNCTION PROTOTYPES-----------------
void _init();
void param_init();
void gpio_init();
void led_init();

void run();

void setRGB(int red, int green, int blue);
void turnOnLEDs(int ledPosition);
void updateLEDs(int ledPosition, void (*func)(int), bool showNow = true);
void setColorToLEDs(int ledPosition);
void fadeOutLEDs(int ledPosition);
void forceBlackLEDs(int ledPosition);
void fadeLEDs();
void turnOnAllLEDs();
void turnOffAllLEDs();
void blinkAllLEDs();

void storeInput(int pos);

void shimmerGold();
void winningResponse();
void losingResponse();
void sendCommand(String cmd);
void handleCommand(String cmd);

void resetInputStorage();
void resetParameters(bool solvedState = false, bool notifyEsp = true);
void resetOutputPins();
void resetAll();

bool haveWon();

int scanForButtonPress();
int inputToLEDMapping(int inputPosition);
bool isLedBlockInBounds(int ledPosition);

//------------------MAIN SETUP-------------------
void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);
  debugLine("Booting puzzle controller.");
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
  debugLine("Initialization complete.");
}

void param_init() {
  resetParameters(false, false);
}

void gpio_init() {
  pinMode(LEDS_DATA_PIN, OUTPUT);

  for (int i = 0; i < NUM_ROWS; i++) {
    pinMode(rowPins[i], INPUT);
    digitalWrite(rowPins[i], LOW);
  }

  for (int i = 0; i < NUM_COLS; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW);
  }

  debugLine("GPIO initialized.");
}

void led_init() {
  FastLED.addLeds<LED_TYPE, LEDS_DATA_PIN, GRB>(leds, TOTAL_LEDS);
  FastLED.setBrightness(brightness);
  turnOffAllLEDs();
  setRGB(191, 0, 255);
  debugLine(String("LEDs initialized. Total LEDs: ") + TOTAL_LEDS);
}

void run() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      incoming.trim();
      debugLine(String("Received ESP command: ") + incoming);
      handleCommand(incoming);
      incoming = "";
    } else {
      incoming += c;
    }
  }

  if (puzzleSolved) {
    return;
  }

  fadeLEDs();

  int pressedButtonPosition = scanForButtonPress();
  if (pressedButtonPosition < 0) {
    return;
  }

  int mappedLedPosition = inputToLEDMapping(pressedButtonPosition);
  debugLine(String("Button ") + pressedButtonPosition + " maps to LED block start " + mappedLedPosition + ".");
  turnOnLEDs(mappedLedPosition);
  storeInput(pressedButtonPosition);

  if (haveWon()) {
    winningResponse();
    resetInputStorage();
    storageIndex = 0;
    posQueue.clear();
  }

  delay(150);
}

void setRGB(int red, int green, int blue) {
  rgb[0] = red;
  rgb[1] = green;
  rgb[2] = blue;
  debugLine(String("Active LED color set to RGB(") + red + ", " + green + ", " + blue + ").");
}

void turnOnLEDs(int ledPosition) {
  updateLEDs(ledPosition, setColorToLEDs);
}

void updateLEDs(int position, void (*func)(int), bool showNow) {
  if (!isLedBlockInBounds(position)) {
    debugLine(String("ERROR: LED block out of bounds at start index ") + position + ".");
    return;
  }

  for (int y = 0; y < yWeight; y++) {
    for (int x = 3; x < xWeight - 2; x++) {
      func((position + (y * LEDS_PER_ROW)) + x);
    }
  }

  if (showNow) {
    FastLED.show();
  }
}

void setColorToLEDs(int ledPosition) {
  leds[ledPosition] = CRGB(rgb[0], rgb[1], rgb[2]);
}

void fadeOutLEDs(int ledPosition) {
  leds[ledPosition].fadeToBlackBy(fadeFactor);
}

void forceBlackLEDs(int ledPosition) {
  leds[ledPosition] = CRGB::Black;
}

void fadeLEDs() {
  if (posQueue.size < 1) return;
  if (millis() - lastTime < fadeIntervalMs) return;

  lastTime = millis();
  unsigned long now = millis();
  bool changed = false;

  while (posQueue.size > 0 && (now - posQueue.head->pushedAt) >= fadeDurationMs) {
    int expiredPosition = posQueue.head->position;
    int ledStart = inputToLEDMapping(expiredPosition);
    debugLine(String("Fade expired for button ") + expiredPosition + "; forcing block black.");
    updateLEDs(ledStart, forceBlackLEDs, false);
    posQueue.pop();
    changed = true;
  }

  for (iPosition* temp = posQueue.head; temp != nullptr; temp = temp->next) {
    updateLEDs(inputToLEDMapping(temp->position), fadeOutLEDs, false);
    changed = true;
  }

  if (changed) {
    FastLED.show();
  }
}

void turnOnAllLEDs() {
  fill_solid(leds, TOTAL_LEDS, CRGB(rgb[0], rgb[1], rgb[2]));
  FastLED.show();
  debugLine("All LEDs turned on.");
}

void turnOffAllLEDs() {
  FastLED.clear();
  FastLED.show();
  debugLine("All LEDs turned off.");
}

void blinkAllLEDs() {
  turnOnAllLEDs();
  delay(500);
  turnOffAllLEDs();
  delay(500);
}

void storeInput(int pos) {
  posQueue.push(pos);

  if (storageIndex >= NUM_SECRET_LETTERS) {
    debugLine("Input ignored because sequence is already complete.");
    return;
  }

  if (pos == correctSecretLettersSequence[storageIndex]) {
    inputStorage[storageIndex] = pos;
    debugLine(String("Correct input at sequence index ") + storageIndex + ": " + correctSecretLettersSymbols[storageIndex]);
    sendCommand(String("BTN:") + correctSecretLettersSymbols[storageIndex]);
    storageIndex++;
    debugKV("Sequence progress", storageIndex);
  } else {
    debugLine(String("Wrong input ") + pos + " at sequence index " + storageIndex + "; resetting sequence progress.");
    storageIndex = 0;
    resetInputStorage();
  }
}

void shimmerGold() {
  debugLine("Starting gold shimmer win animation.");
  const CRGB gold = CRGB(255, 180, 0);
  for (int frame = 0; frame < 120; frame++) {
    for (int i = 0; i < TOTAL_LEDS; i++) {
      leds[i] = gold;
      leds[i].nscale8(random8(120, 256));
    }
    FastLED.show();
    delay(35);
  }
  debugLine("Gold shimmer complete.");
}

void winningResponse() {
  Serial.println("You won.");
  debugLine("Puzzle solved. Latching solved state until PUZZLE_RESET.");
  shimmerGold();
  puzzleSolved = true;
  sendCommand("SOLVED");
}

void losingResponse() {
  Serial.println("You lose!");
  debugLine("Running losing LED response.");
  setRGB(255, 0, 0);
  for (int i = 0; i < 5; i++) {
    blinkAllLEDs();
  }
}

void sendCommand(String cmd) {
  debugLine(String("Sending ESP command: ") + cmd);
  Serial1.println(cmd);
}

void handleCommand(String cmd) {
  if (cmd == "PING") {
    sendCommand("PONG");
  } else if (cmd == "STATUS") {
    sendCommand(puzzleSolved ? "SOLVED" : "READY");
  } else if (cmd == "PUZZLE_RESET") {
    resetAll();
    sendCommand("RESET");
  } else if (cmd == "SOLVE") {
    debugLine("Manual solve command received.");
    winningResponse();
    sendCommand("MANUALLY_SOLVED");
  } else if (cmd.length() > 0) {
    debugLine(String("Unknown ESP command ignored: ") + cmd);
  }
}

void resetInputStorage() {
  for (int i = 0; i < NUM_SECRET_LETTERS; i++) {
    inputStorage[i] = -1;
  }
  debugLine("Input sequence storage reset.");
}

void resetParameters(bool solvedState, bool notifyEsp) {
  resetInputStorage();
  storageIndex = 0;
  lastPressedPosition = -1;
  puzzleSolved = solvedState;
  debugLine(String("Parameters reset. Solved state is ") + (puzzleSolved ? "true." : "false."));

  if (notifyEsp) {
    sendCommand("RESET");
  }
}

void resetOutputPins() {
  for (int i = 0; i < NUM_COLS; i++) {
    digitalWrite(colPins[i], LOW);
  }
  debugLine("Column output pins reset LOW.");
}

void resetAll() {
  debugLine("Full puzzle reset requested.");
  setRGB(191, 0, 255);
  resetOutputPins();
  posQueue.clear();
  turnOffAllLEDs();
  resetParameters(false, false);
}

bool haveWon() {
  return storageIndex >= NUM_SECRET_LETTERS;
}

int scanForButtonPress() {
  int detected = -1;

  for (int y = 0; y < NUM_COLS; y++) {
    digitalWrite(colPins[y], HIGH);
    if (detected < 0) {
      for (int x = 0; x < NUM_ROWS; x++) {
        if (digitalRead(rowPins[x])) {
          detected = y + (x * NUM_COLS);
          break;
        }
      }
    }
    digitalWrite(colPins[y], LOW);
  }

  if (detected < 0) {
    lastPressedPosition = -1;
    return -1;
  }

  if (detected == lastPressedPosition) {
    return -1;
  }

  lastPressedPosition = detected;
  debugLine(String("Button press detected: ") + detected);
  return detected;
}

int inputToLEDMapping(int inputPosition) {
  int row = inputPosition / NUM_COLS;
  int col = inputPosition % NUM_COLS;
  return (col + (row * yWeight * NUM_COLS)) * xWeight;
}

bool isLedBlockInBounds(int ledPosition) {
  int highestTouched = ledPosition + ((yWeight - 1) * LEDS_PER_ROW) + (xWeight - 3);
  return ledPosition >= 0 && highestTouched < TOTAL_LEDS;
}
