#include <Adafruit_NeoPixel.h>

#define PIN            6
#define NUMPIXELS      (16*16)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

struct RGB {
    float r, g, b;
};

struct HSV {
    float h, s, v;
};

RGB hsvToRgb(HSV hsv) {
    float h = hsv.h, s = hsv.s, v = hsv.v;
    float r, g, b;

    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch(i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    RGB rgb = { r, g, b };
    return rgb;
}

void setup() {
  strip.begin();
  strip.show();
}

float hueOffset = 0;  // This will be used to shift the hues over time for animation.

void loop() {
  for (int i = 0; i < NUMPIXELS; i++) {
    float hue = (i / (float)NUMPIXELS) + hueOffset;  // Convert pixel index to hue value
    if (hue > 1.0) hue -= 1.0;  // Wrap hue value around

    RGB col = hsvToRgb({hue, 1.0, 1.0});
    strip.setPixelColor(i, strip.Color((uint8_t)(col.r * 10), (uint8_t)(col.g * 10), (uint8_t)(col.b * 10)));
  }

  strip.show();

  hueOffset += 0.1;  // Adjust the hue shift for the next animation frame.
  if (hueOffset > 1.0) hueOffset -= 1.0;  // Keep hueOffset between 0 and 1.

  delay(50);
}
