
#include <FastLED.h>

#define LEDS_PER_ROW 35
#define NUM_ROWS 3
#define NUM_LEDS (LEDS_PER_ROW * NUM_ROWS)
#define LEDS_PIN 2


int brightness = 128;

CRGB leds[NUM_LEDS];

void test1() {
  //blink LEDs
  for (int i = 0; i < 5; i++) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();
    delay(500);
    FastLED.clear();
    FastLED.show();
    delay(500);
  }
  delay(500);
}

void test2() {
  //Check each LED

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
    FastLED.show();
    delay(25);
    FastLED.clear();
    FastLED.show();
    delay(25);
  }

  delay(500);
}


void test3() {
  //Check colors
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(1000);
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(1000);
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(1000);
}

void test4() {
    EVERY_N_MILLISECONDS(50) {
      for (int i = 0; i < LEDS_PER_ROW/2; i++) {
        leds[i].fadeToBlackBy(random(40));
      }
      FastLED.show();
      Serial.println("50 milli");
    }
    Serial.println("Outside of 50 milli");
  
}

void setup() {
  FastLED.addLeds<WS2812B, LEDS_PIN, GRB>(leds, NUM_LEDS);
  pinMode(LEDS_PIN, OUTPUT);

  Serial.begin(9600);
  FastLED.setBrightness(brightness);

  randomSeed(analogRead(A0));
  for (int i = 0; i < LEDS_PER_ROW/2; i++) {
    leds[i].setRGB(random(256), random(256), random(256));
  }
  FastLED.show();
}

void loop() {

  //FastLED.clear();
  //FastLED.show();
  //test1();
  //test2();
  //test3();
  test4();
}
