
#include <FastLED.h>

#define NUM_LEDS 92
#define LEDS_PIN 2


CRGB leds[NUM_LEDS];

int count = 0;

void test1() {
  //blink LEDs
  FastLED.setBrightness(64);
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
    delay(50);
    FastLED.clear();
    FastLED.show();
    delay(50);
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
  FastLED.clear();
  FastLED.show();
  delay(1000);
}

void setup() {
  FastLED.addLeds<WS2812B, LEDS_PIN, GRB>(leds, NUM_LEDS);
  pinMode(LEDS_PIN, OUTPUT);
}

void loop() {
  if (count < 3) {
    FastLED.clear();
    FastLED.show();
    test1();
    test2();
    test3();
    FastLED.clear();
  } 

  count++;
  return;
}
