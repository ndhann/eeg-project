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

  // obtain the sampling period in microseconds
  sampling_period_micro = round(1000000 * (1.0 / SAMPLES));
}

void printPowers(double input[]) {
  double deltapower = 0.0;
  double thetapower = 0.0;
  double alphapower = 0.0;
  double betapower = 0.0;
  for (int i = 0; i <= 3; i++) {
    deltapower += input[i] * input[i];
  }
  for (int i = 4; i <= 8; i++) {
    thetapower += input[i] * input[i];
  }
  for (int i = 9; i <= 12; i++) {
    alphapower += input[i] * input[i];
  }
  for (int i = 13; i <= 30; i++) {
    betapower += input[i] * input[i];
  }

  Serial.print("Delta wave power is: ");
  Serial.println(deltapower);
  Serial.print("Theta wave power is: ");
  Serial.println(thetapower);
  Serial.print("Alpha wave power is: ");
  Serial.println(alphapower);
  Serial.print("Beta wave power is: ");
  Serial.println(betapower);
}

void loop() {
  for (int i = 0; i < SAMPLES; i++) {
    curTime = micros();
    inputReal[i] = analogRead(inputPin);
    inputImag[i] = 0;
    while (micros() < (curTime + sampling_period_micro)) {
      // do nothing until it's time to sample again
    }
  }
  
  // calculate the FFT
  FFT.Compute(inputReal, inputImag, SAMPLES, FFT_FORWARD);
  double peak = FFT.MajorPeak(inputReal, SAMPLES, SAMPLES);
  Serial.print("FFT peak is: ");
  Serial.println(peak);

  printPowers(inputReal);
}
