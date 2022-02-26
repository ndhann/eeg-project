#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SAMPLEINTERVAL 7.8125 // for sample rate of 128Hz
double currentTime;
double sampleTime;

double inputDelta = 0.0;
const int pinDelta = A0;
const int resistDelta = 1;

double inputTheta = 0.0;
const int pinTheta = A1;
const int resistTheta = 1;

double inputAlpha = 0.0;
const int pinAlpha = A2;
const int resistAlpha = 1;

double inputBeta = 0.0;
const int pinBeta = A3;
const int resistBeta = 1;

void readAllInputs(double delta, double theta, double alpha, double beta) {
  delta = analogRead(pinDelta);
  theta = analogRead(pinTheta);
  alpha = analogRead(pinAlpha);
  beta = analogRead(pinBeta);
}

void clearDisplayInputValues() {
  display.setCursor(0, 0);
  display.println("               ");
  display.println("               ");
  display.println("               ");
  display.println("               ");
}

void printInputValues(double delta, double theta, double alpha, double beta) {
  clearDisplayInputValues();
  display.setCursor(0, 0);
  
  display.print("Delta: ");
  display.println(delta);

  display.print("Theta: ");
  display.println(theta);

  display.print("Alpha: ");
  display.println(alpha);

  display.print("Beta: ");
  display.println(beta);

  display.display();
}

void setup() {
  // serial console
  Serial.begin(9600);

  // set up display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);

  // timer setup
  currentTime = millis();
  sampleTime = currentTime + SAMPLEINTERVAL;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (currentTime >= sampleTime) {
    readAllInputs(inputDelta, inputTheta, inputAlpha, inputBeta);
    currentTime = millis();
    sampleTime = currentTime + SAMPLEINTERVAL;
  }

  printInputValues(inputDelta, inputTheta, inputAlpha, inputBeta);

  currentTime = millis();
}
