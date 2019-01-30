#include "FastLED.h"

FASTLED_USING_NAMESPACE


//
// CRGB leds[NUM_STRIPS][NUM_VERT_LEDS_PER_STRIP];
//
// // For mirroring strips, all the "special" stuff happens just in setup.  We
// // just addLeds multiple times, once for each strip
// void setup() {
//   // tell FastLED there's 60 NEOPIXEL leds on pin 10
//   FastLED.addLeds<NEOPIXEL, 10>(leds[0], NUM_VERT_LEDS_PER_STRIP);
//
//   // tell FastLED there's 60 NEOPIXEL leds on pin 11
//   FastLED.addLeds<NEOPIXEL, 11>(leds[1], NUM_VERT_LEDS_PER_STRIP);
//
//   // tell FastLED there's 60 NEOPIXEL leds on pin 12
//   FastLED.addLeds<NEOPIXEL, 12>(leds[2], NUM_VERT_LEDS_PER_STRIP);
//
// }
//
// void loop() {
//   // This outer loop will go over each strip, one at a time
//   for(int x = 0; x < NUM_STRIPS; x++) {
//     // This inner loop will go over each led in the current strip, one at a time
//     for(int i = 0; i < NUM_VERT_LEDS_PER_STRIP; i++) {
//       leds[x][i] = CRGB::Red;
//       FastLED.show();
//       leds[x][i] = CRGB::Black;
//       delay(100);
//     }
//   }
// }

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define PIN_DATA_VR  19
#define PIN_CLOCK_VR 18

#define PIN_DATA_VL  23
#define PIN_CLOCK_VL 22

#define PIN_DATA_H  12
#define PIN_CLOCK_H 14

#define LED_TYPE    APA102
#define COLOR_ORDER GRB

#define NUM_VERT_STRIPS  2
#define NUM_HORIZ_STRIPS 1

// #define NUM_VERT_LEDS  60
#define NUM_VERT_LEDS  360
#define NUM_HORIZ_LEDS 120

CRGB leds[NUM_VERT_LEDS];

#define BRIGHTNESS         40
#define FRAMES_PER_SECOND  10

// -- The core to run FastLED.show()
#define FASTLED_SHOW_CORE 0

// -- Task handles for use in the notifications
static TaskHandle_t FastLEDshowTaskHandle = 0;
static TaskHandle_t userTaskHandle = 0;

/** show() for ESP32
 *  Call this function instead of FastLED.show(). It signals core 0 to issue a show,
 *  then waits for a notification that it is done.
 */
void FastLEDshowESP32()
{
    if (userTaskHandle == 0) {
        // -- Store the handle of the current task, so that the show task can
        //    notify it when it's done
        userTaskHandle = xTaskGetCurrentTaskHandle();

        // -- Trigger the show task
        xTaskNotifyGive(FastLEDshowTaskHandle);

        // -- Wait to be notified that it's done
        const TickType_t xMaxBlockTime = pdMS_TO_TICKS( 200 );
        ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
        userTaskHandle = 0;
    }
}

/** show Task
 *  This function runs on core 0 and just waits for requests to call FastLED.show()
 */
void FastLEDshowTask(void *pvParameters)
{
    // -- Run forever...
    for(;;) {
        // -- Wait for the trigger
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // -- Do the show (synchronously)
        FastLED.show();

        // -- Notify the calling task
        xTaskNotifyGive(userTaskHandle);
    }
}

void setup() {
  delay(3000); // 3 second delay for recovery
  Serial.begin(115200);

  // FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_VERT_LEDS).setCorrection(TypicalLEDStrip);

  // tell FastLED about the LED strip configuration
  // Add Left Vertical strip LEDs
  FastLED.addLeds<LED_TYPE,PIN_DATA_VL,PIN_CLOCK_VL,COLOR_ORDER>(leds, 60).setCorrection(TypicalLEDStrip);

  // Add Right Vertical strip LEDs
  FastLED.addLeds<LED_TYPE,PIN_DATA_VR,PIN_CLOCK_VR,COLOR_ORDER>(leds, 60).setCorrection(TypicalLEDStrip);

  // Add Horizontal strip LEDs
  FastLED.addLeds<LED_TYPE,PIN_DATA_H,PIN_CLOCK_H,COLOR_ORDER>(leds, 240).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

    int core = xPortGetCoreID();
    Serial.print("Main code running on core ");
    Serial.println(core);

    // -- Create the FastLED show task
    xTaskCreatePinnedToCore(FastLEDshowTask, "FastLEDshowTask", 2048, NULL, 2, &FastLEDshowTaskHandle, FASTLED_SHOW_CORE);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLEDshowESP32();
  // FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_VERT_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_VERT_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_VERT_LEDS, 10);
  int pos = random16(NUM_VERT_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_VERT_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_VERT_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_VERT_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_VERT_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_VERT_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
