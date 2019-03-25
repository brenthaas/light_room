#include "FastLED.h"

// Pride2015
// Animated, ever-changing rainbows.
// by Mark Kriegsman

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define PIN_DATA_VR  19
#define PIN_CLOCK_VR 18

#define PIN_DATA_VL  23
#define PIN_CLOCK_VL 22

#define PIN_DATA_H  12
#define PIN_CLOCK_H 14

#define LED_TYPE    APA102
#define COLOR_ORDER GRB
#define BRIGHTNESS  255

#define NUM_VERT_LEDS 60
#define NUM_HORIZ_LEDS 240

#define TOTAL_LEDS    360

CRGB leds[TOTAL_LEDS];


void setup() {
  delay(3000); // 3 second delay for recovery

  // Add Left Vertical strip LEDs
  FastLED.addLeds<LED_TYPE,PIN_DATA_VL,PIN_CLOCK_VL,COLOR_ORDER>(leds, NUM_VERT_LEDS)
    .setCorrection(TypicalLEDStrip)
    .setDither(BRIGHTNESS < 255);

  // Add Right Vertical strip LEDs
  FastLED.addLeds<LED_TYPE,PIN_DATA_VR,PIN_CLOCK_VR,COLOR_ORDER>(leds, NUM_VERT_LEDS)
    .setCorrection(TypicalLEDStrip)
    .setDither(BRIGHTNESS < 255);

  // Add Horizontal strip LEDs
  FastLED.addLeds<LED_TYPE,PIN_DATA_H,PIN_CLOCK_H,COLOR_ORDER>(leds, NUM_HORIZ_LEDS)
    .setCorrection(TypicalLEDStrip)
    .setDither(BRIGHTNESS < 255);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


void loop()
{
  pride();
  FastLED.show();
}


// This function draws rainbows with an ever-changing,
// widely-varying set of parameters.
void pride()
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;

  for( uint16_t i = 0 ; i < TOTAL_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    CRGB newcolor = CHSV( hue8, sat8, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (TOTAL_LEDS-1) - pixelnumber;

    nblend( leds[pixelnumber], newcolor, 64);
  }
}
