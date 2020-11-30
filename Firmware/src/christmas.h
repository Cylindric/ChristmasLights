#ifndef CHRISTMAS_H
#define CHRISTMAS_H

#include <Arduino.h>
#include "FastLED.h"

#define NUM_LEDS      300
#define LED_TYPE   WS2812B
#define COLOR_ORDER   GRB
#define DATA_PIN        5
#define VOLTS           5
#define MAX_MA       4000

class Christmas
{
public:
  void setupChristmas();
  void setupCylon();
  void loopChristmas();
  void loopCylon();
  void start();
  void stop();

  bool state;

private:
  void chooseNextColorPalette(CRGBPalette16&);
  void drawTwinkles(CRGBSet&);
  CRGB computeOneTwinkle(uint32_t, uint8_t);
  void coolLikeIncandescent(CRGB&, uint8_t);
  uint8_t attackDecayWave8(uint8_t);
  void fadeall();

  CRGBArray<NUM_LEDS> leds;

  // Background color for 'unlit' pixels
  // Can be set to CRGB::Black if desired.
  CRGB gBackgroundColor = CRGB::Black; 
  // Example of dim incandescent fairy light background color
  // CRGB gBackgroundColor = CRGB(CRGB::FairyLight).nscale8_video(16);

  CRGBPalette16 gCurrentPalette;
  CRGBPalette16 gTargetPalette;
};


// Overall twinkle speed.
// 0 (VERY slow) to 8 (VERY fast).  
// 4, 5, and 6 are recommended, default is 4.
#define TWINKLE_SPEED 4

// Overall twinkle density.
// 0 (NONE lit) to 8 (ALL lit at once).  
// Default is 5.
#define TWINKLE_DENSITY 5

// How often to change color palettes.
#define SECONDS_PER_PALETTE  30
// Also: toward the bottom of the file is an array 
// called "ActivePaletteList" which controls which color
// palettes are used; you can add or remove color palettes
// from there freely.

// If AUTO_SELECT_BACKGROUND_COLOR is set to 1,
// then for any palette where the first two entries 
// are the same, a dimmed version of that color will
// automatically be used as the background color.
#define AUTO_SELECT_BACKGROUND_COLOR 0

// If COOL_LIKE_INCANDESCENT is set to 1, colors will 
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 1


#endif // CHRISTMAS_H
