
#include <FastLED.h>

#define NUM_ROWS 5
#define SENSORS_PER_ROW 7
#define LEDS_PER_STRIP 70
#define LEDS_PER_COLUMN 10
const unsigned long pulseInterval = 15;

const int touchPins[NUM_ROWS][SENSORS_PER_ROW] = {
  {2, 3, 4, 5, 6, 7, 8},
  {9, 10, 11, 12, 13, 14, 15},
  {16, 17, 18, 19, 20, 21, 22},
  {23, 24, 25, 26, 27, 28, 29},
  {30, 31, 32, 33, 34, 35, 36}
};

const int ledPins[NUM_ROWS] = {A0, A1, A2, A3, A4};
CRGB leds[NUM_ROWS][LEDS_PER_STRIP];

uint8_t brightness[NUM_ROWS][SENSORS_PER_ROW] = {0};
int direction[NUM_ROWS][SENSORS_PER_ROW] = {0}; // 0 = not started, 1 = up, -1 = down
unsigned long lastUpdate = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Setup complete. Starting LED pulse system...");
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < SENSORS_PER_ROW; col++) {
      pinMode(touchPins[row][col], INPUT);
    }
    FastLED.addLeds<WS2812B, GRB>(leds[row], LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);
    FastLED.clear(true);
  }
}

void loop() {
  unsigned long currentTime = millis();

  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < SENSORS_PER_ROW; col++) {
      int sensorValue = digitalRead(touchPins[row][col]);
      int startLED = col * LEDS_PER_COLUMN;
      int endLED = startLED + LEDS_PER_COLUMN;

      if (sensorValue == HIGH) {
        if (currentTime - lastUpdate >= pulseInterval) {
          if (direction[row][col] == 0) direction[row][col] = 1;
          brightness[row][col] += direction[row][col];
          if (brightness[row][col] >= 255) direction[row][col] = -1;
          else if (brightness[row][col] <= 0) direction[row][col] = 1;

          Serial.print("Row "); Serial.print(row);
          Serial.print(", Sensor "); Serial.print(col);
          Serial.print(" touched. Brightness: ");
          Serial.println(brightness[row][col]);
        }
      } else {
        if (brightness[row][col] != 0) {
          Serial.print("Row "); Serial.print(row);
          Serial.print(", Sensor "); Serial.print(col);
          Serial.println(" released. LEDs off.");
        }
        brightness[row][col] = 0;
        direction[row][col] = 1;
      }

      for (int i = startLED; i < endLED; i++) {
        leds[row][i] = (sensorValue == HIGH) ? CRGB(0, 0, brightness[row][col]) : CRGB::Black;
      }
    }
    FastLED.show(leds[row], LEDS_PER_STRIP);
  }

  if (currentTime - lastUpdate >= pulseInterval) {
    lastUpdate = currentTime;
  }
}
