#include <Adafruit_NeoPixel.h>
#include <arduinoFFT.h>

#define PIN                 5
#define MIC                 A0
#define NUMPIXELS           (8*8)
#define SAMPLES             512
#define INTERVALS           8

double frequency = 0;

int intervals[INTERVALS][2] = {{50, 100}, {100, 200}, {200, 400}, {400, 800}, {800, 1600}, {1600, 3200}, {3200, 6400}, {6400, 10000}};
// int intervals[INTERVALS][2] = {{32, 64}, {64, 128}, {128, 256}, {256, 512}, {512, 1024}, {1024, 2048}, {2048, 4096}, {4096, 8192}};
arduinoFFT FFT = arduinoFFT();
double scaledValues[1024];
double vReal[SAMPLES];
double vImag[SAMPLES];
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int freqColors[8][3] = {
    {0, 0, 255},
    {36, 0, 219},
    {73, 0, 182},
    {109, 0, 146},
    {146, 0, 109},
    {182, 0, 73},
    {219, 0, 36},
    {255, 0, 0},
};

double scale[8][8] = {
  {0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8},
  {0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8},
  {0.125, 0.25, 0.5, 1, 2, 4, 8, 16},
  {0.125, 0.25, 0.5, 1, 2, 4, 8, 16},
  {0.25, 0.5, 1, 2, 4, 8, 16, 32},
  {0.25, 0.5, 1, 2, 4, 8, 16, 32},
  {0.5, 1, 2, 4, 8, 16, 32, 64},
  {0.5, 1, 2, 4, 8, 16, 32, 64}
};

double scaleValue(double value, double minVal, double maxVal) {
    if (minVal >= maxVal) {
        return 0.0;
    }
    double scaledValue = (value - minVal) / (maxVal - minVal);
    return constrain(scaledValue, 0.0, 1.0);
}

void setup() {
  for (int i = 0; i < 1024; i++) {
    scaledValues[i] = scaleValue(i, 0, 1023);
  }
  Serial.begin(9600);
  pinMode(MIC, INPUT);
  strip.begin();
  strip.show();
  unsigned long startTime = millis();
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = scaledValues[analogRead(MIC)];
  }
  unsigned long endTime = millis();
  unsigned long elapsedTime = endTime - startTime;
  double period = static_cast<double>(elapsedTime) / SAMPLES / 1000.0;
  frequency = 1.0 / period;
  Serial.print("Elapsed Time: ");
  Serial.print(elapsedTime);
  Serial.println(" ms");
}

void loop() {
  for (int i = 0; i < SAMPLES; i++) {
    vImag[i] = 0;
  }
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = scaledValues[analogRead(MIC)];
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  // Calculate intensity within each interval
  double intervalIntensities[INTERVALS] = {0};
  for (int i = 0; i < INTERVALS; i++) {
    for (int j = int(intervals[i][0] * SAMPLES / frequency); j < int(intervals[i][1] * SAMPLES / frequency); j++) {
      intervalIntensities[i] += vReal[j];
    }
  }
  // Serial.println("Frequency Intervals:");
  // for (int i = 0; i < INTERVALS; i++) {
  //   Serial.print(intervals[i][0]);
  //   Serial.print("-");
  //   Serial.print(intervals[i][1]);
  //   Serial.print(" Hz: ");
  //   Serial.println(intervalIntensities[i]);
  // }
  for (int i = 0; i < 8; i++) {
    int j = 0;
    while (j < 8 && intervalIntensities[i] > scale[i][j]) {
      strip.setPixelColor(i * 8 + j++, strip.Color(freqColors[i][0], 0, freqColors[i][2]));
    }
    if (j < 7) {
      double tmp = intervalIntensities[i] / scale[i][j + 1];
      strip.setPixelColor(i * 8 + j++, strip.Color(freqColors[i][0] * tmp, 0, freqColors[i][2] * tmp));
    }
    while (j < 8)
      strip.setPixelColor(i * 8 + j++, strip.Color(0, 0, 0));
  }
  strip.show();
}
