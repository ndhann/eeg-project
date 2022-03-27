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

double deltapower = 0.0;
double thetapower = 0.0;
double alphapower = 0.0;
double betapower = 0.0;

const int inputPin = A1;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);


  // obtain the sampling period in microseconds
  sampling_period_micro = round(1000000 * (1.0 / SAMPLES));
}

void calculatePowers(double input[]) {
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
}
void printPowers(double input[]) {
  calculatePowers(input);

  Serial.print("Delta wave power is: ");
  Serial.println(deltapower);
  Serial.print("Theta wave power is: ");
  Serial.println(thetapower);
  Serial.print("Alpha wave power is: ");
  Serial.println(alphapower);
  Serial.print("Beta wave power is: ");
  Serial.println(betapower);
}

void drawBar(int x, int y, double power, double maxpower) {
  if (((power / maxpower) > (0.8))) {
    display.drawChar(x + (4 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power / maxpower) > (0.6))) {
    display.drawChar(x + (3 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power / maxpower) > (0.4))) {
    display.drawChar(x + (2 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power / maxpower) > (0.2))) {
    display.drawChar(x + (1 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power / maxpower) > (0.1))) {
    display.drawChar(x + (0 * 7), y, 0xDA, WHITE, BLACK, 1);
  }
}

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
}

void drawBarGraph(double deltapower, double thetapower, double alphapower, double betapower) {
  double maxpower = findMaxPower(deltapower, thetapower, alphapower, betapower);
  drawBar(80, 8, deltapower, maxpower);
  drawBar(80, 16, thetapower, maxpower);
  drawBar(80, 24, alphapower, maxpower);
  drawBar(80, 32, betapower, maxpower);
}

void clearDisplayedPowers() {
  display.setCursor(0, 0);
  display.setTextWrap(false);
  // clear the displayed values by simply overwriting them with blank text
  display.println("                        ");
  display.println("                        ");
  display.println("                        ");
  display.println("                        ");
  display.println("                        ");
  display.setTextWrap(true);
}

void displayPowers(double input[]) {
  calculatePowers(input);

  clearDisplayedPowers();
  display.setCursor(0, 0);

  display.println("Power");

  display.print("Delta : ");
  display.println(deltapower);

  display.print("Theta: ");
  display.println(thetapower);

  display.print("Alpha: ");
  display.println(alphapower);

  display.print("Beta: ");
  display.println(betapower);

  drawBarGraph(deltapower, thetapower, alphapower, betapower);

  display.display();
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
  //Serial.print("FFT peak is: ");
  //Serial.println(peak);

  displayPowers(inputReal);
}
