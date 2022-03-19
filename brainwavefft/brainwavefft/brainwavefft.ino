#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <arduinoFFT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define VOLTSPERBIT 0.0049
#define SAMPLES 128

arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period_micro;
unsigned long curTime;

double inputReal[SAMPLES];
double inputImag[SAMPLES];

const int inputPin = A1;

void setup() {
  Serial.begin(115200);
  Serial.println("hello?");

  // obtain the sampling period in microseconds
  sampling_period_micro = round(1000000 * (1.0 / SAMPLES));
}

void loop() {
  for (int i = 0; i < SAMPLES; i++) {
    curTime = micros();
    inputReal[i] = analogRead(inputPin);
    inputImag[i] = 0;
    Serial.print("Inside the first for loop, ");
    Serial.println(i);
    while (micros() < (curTime + sampling_period_micro)) {
      // do nothing until it's time to sample again
    }
  }

  Serial.println("outside the loop");
  // calculate the FFT
  //FFT.Windowing(inputReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  Serial.println("FFT Windowing done");
  FFT.Compute(inputReal, inputImag, SAMPLES, FFT_FORWARD);
  Serial.println("FFT Computations done");
  double peak = FFT.MajorPeak(inputReal, SAMPLES, SAMPLES);
  Serial.print("FFT peak is: ");
  Serial.println(peak);

  for ( int i = 0; i < (SAMPLES/2); i++) {
    Serial.print(i, 1);
    Serial.print(" ");
    Serial.println(inputReal[i], 1);
  }

  //delay(1000); // repeat every second
}
