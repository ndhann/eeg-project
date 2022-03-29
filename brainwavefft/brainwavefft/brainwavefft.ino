#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <arduinoFFT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 16
#define RST_PIN 4
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

// calculated from the ADC ranges (0-5V and 0-1023)
#define VOLTSPERBIT 0.0049
#define SAMPLES 128

arduinoFFT FFT = arduinoFFT();

unsigned int sampling_period_micro;
unsigned long curTime;

double inputReal[SAMPLES];
double inputImag[SAMPLES];

double deltapower = 0.0;
double thetapower = 0.0;
double alphapower = 0.0;
double betapower = 0.0;

// The pin to receive the signal to process
const int inputPin = A1;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000L);

  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  oled.clear();
  // Set font to something small so everything can be displayed on screen.
  oled.setFont(System5x7);

  // obtain the sampling period in microseconds
  sampling_period_micro = round(1000000 * (1.0 / SAMPLES));
}

void calculatePowers() {
  deltapower = 0.0;
  thetapower = 0.0;
  alphapower = 0.0;
  betapower = 0.0;

  // calculate the RMS for each frequency range
  // Delta: 1-3Hz
  for (int i = 1; i <= 3; i++) {
    deltapower += sq(inputReal[i]) + sq(inputImag[i]);
  }
  deltapower = sqrt(deltapower / 3.0);

  // Theta: 4-8Hz
  for (int i = 4; i <= 8; i++) {
    thetapower += sq(inputReal[i]) + sq(inputImag[i]);
  }
  thetapower = sqrt(thetapower / 5.0);

  // Alpha: 9-12Hz
  for (int i = 9; i <= 12; i++) {
    alphapower += sq(inputReal[i]) + sq(inputImag[i]);
  }
  alphapower = sqrt(alphapower / 4.0);

  // Beta: 13-30Hz
  for (int i = 13; i <= 30; i++) {
    betapower += sq(inputReal[i]) + sq(inputImag[i]);
  }
  betapower = sqrt(betapower / 18.0);
}

// This function prints to the serial terminal. Used for debugging.
void printPowers() {
  calculatePowers();

  Serial.print("Delta wave power is: ");
  Serial.println(deltapower);
  Serial.print("Theta wave power is: ");
  Serial.println(thetapower);
  Serial.print("Alpha wave power is: ");
  Serial.println(alphapower);
  Serial.print("Beta wave power is: ");
  Serial.println(betapower);
}

// Draws a bar using ASCII characters based on the two quantities given
void drawBar(double power, double maxpower) {
  if (((power / maxpower) > (0.8))) {
    oled.print("|");
  }

  if (((power / maxpower) > (0.6))) {
    oled.print("|");
  }

  if (((power / maxpower) > (0.4))) {
    oled.print("|");
  }

  if (((power / maxpower) > (0.2))) {
    oled.print("|");
  }

  if (((power / maxpower) > (0.1))) {
    oled.print("|");
  }
}

// Simple algorithm to find the largest power
double findMaxPower(double deltapower, double thetapower, double alphapower, double betapower) {
  double maxpower = 0.0;
  if (maxpower < deltapower) {
    maxpower = deltapower;
  }
  if (maxpower < thetapower) {
    maxpower = thetapower;
  }
  if (maxpower < alphapower) {
    maxpower = alphapower;
  }
  if (maxpower < betapower) {
    maxpower = betapower;
  }
  return maxpower;
}

// displays values and the bar graph on the oled display
void displayPowers() {
  calculatePowers();
  double maxpower = findMaxPower(deltapower, thetapower, alphapower, betapower);
  Serial.println(maxpower);

  // clear the screen every time calculations are refreshed
  oled.clear();

  oled.println("Power");

  // write out values and draw bars for each of the frequency ranges
  oled.print("Delta: ");
  oled.print(deltapower);
  oled.print(" ");
  drawBar(deltapower, maxpower);
  oled.println();

  oled.print("Theta: ");
  oled.print(thetapower);
  oled.print(" ");
  drawBar(thetapower, maxpower);
  oled.println();

  oled.print("Alpha: ");
  oled.print(alphapower);
  oled.print(" ");
  drawBar(alphapower, maxpower);
  oled.println();

  oled.print("Beta:  ");
  oled.print(betapower);
  oled.print(" ");
  drawBar(betapower, maxpower);
  oled.println();
}

void loop() {
  // loop and obtain 128 samples over one second
  for (int i = 0; i < SAMPLES; i++) {
    curTime = micros();
    inputReal[i] = analogRead(inputPin) * VOLTSPERBIT; // read the ADC value and convert to volts
    inputImag[i] = 0; // no imaginary input
    while (micros() < (curTime + sampling_period_micro)) {
      // do nothing until it's time to sample again
    }
  }

  // calculate the FFT
  FFT.Compute(inputReal, inputImag, SAMPLES, FFT_FORWARD);
  double peak = FFT.MajorPeak(inputReal, SAMPLES, SAMPLES);
  //Serial.print("FFT peak is: ");
  //Serial.println(peak);

  displayPowers();
}
