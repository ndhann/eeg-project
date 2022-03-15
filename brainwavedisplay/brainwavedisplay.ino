#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define VOLTSPERBIT 0.0049
#define SAMPLEINTERVAL 7.8125 // for sample rate of 128Hz
double currentTime;
double sampleTime;

double inputDelta = 0;
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

void readAllInputs(double *delta, double *theta, double *alpha, double *beta) {
  *delta = analogRead(pinDelta) * VOLTSPERBIT; // convert the ADC input to a voltage value
  *theta = analogRead(pinTheta) * VOLTSPERBIT;
  *alpha = analogRead(pinAlpha) * VOLTSPERBIT;
  *beta = analogRead(pinBeta) * VOLTSPERBIT;
}

void drawBar(int x, int y, double power) {
  // this is the maximum power that can be read by the arduino.
  // this value may need to be tweaked so that read power values are scaled properly
  double maxpower = (VOLTSPERBIT * 1023) * (VOLTSPERBIT * 1023);

  // if statements for each of the 5 segments of the bar
  // there are 5 maximum segments in each bar
  if (((power/maxpower) > (0.8))) {
    display.drawChar(x + (4 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power/maxpower) > (0.6))) {
    display.drawChar(x + (3 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power/maxpower) > (0.4))) {
    display.drawChar(x + (2 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power/maxpower) > (0.2))) {
    display.drawChar(x + (1 * 7), y, 0xDA, WHITE, BLACK, 1);
  }

  if (((power/maxpower) > (0.1))) {
    display.drawChar(x + (0 * 7), y, 0xDA, WHITE, BLACK, 1);
  }
}

void clearBarGraph() {
  // set this to make sure text printed elsewhere doesn't mess up with characters in unwanted places
  display.setTextWrap(false);
  // loop over all locations of the bar graphs and clear them
  for (int i = 1; i <= 4; i++) {
    display.setCursor(80, (8 * i));
    display.print("               ");
  }
  display.setTextWrap(true);
}

void drawBarGraph(double deltapower, double thetapower, double alphapower, double betapower) {
  // clear previously displayed values
  clearBarGraph();
  // display new bars for each of the read power values
  drawBar(80, 8, deltapower);
  drawBar(80, 16, thetapower);
  drawBar(80, 24, alphapower);
  drawBar(80, 32, betapower);
}

void clearDisplayInputValues() {
  display.setCursor(0, 0);
  // clear the displayed values by simply overwriting them with blank text
  display.println("               ");
  display.println("               ");
  display.println("               ");
  display.println("               ");
  display.println("               ");
}

void printInputValues(double delta, double theta, double alpha, double beta) {
  // calculate power to display
  double deltapower = delta * delta;
  double thetapower = theta * theta;
  double alphapower = alpha * alpha;
  double betapower = beta * beta;

  // clear previously displayed values
  clearDisplayInputValues();
  display.setCursor(0, 0);

  display.println("Power");

  display.print("Delta: ");
  display.println(deltapower);

  display.print("Theta: ");
  display.println(thetapower);

  display.print("Alpha: ");
  display.println(alphapower);

  display.print("Beta: ");
  display.println(betapower);

  // draw the bar graph; this is explained above
  drawBarGraph(deltapower, thetapower, alphapower, betapower);

  // finally, actually display everything
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
  display.setTextColor(WHITE, BLACK);

  // timer setup
  currentTime = millis();
  sampleTime = currentTime + SAMPLEINTERVAL;
}

void loop() {
  // determine whether it's time to sample or not
  if (currentTime >= sampleTime) {
    // when time to sample, read input values and store them
    readAllInputs(&inputDelta, &inputTheta, &inputAlpha, &inputBeta);
    currentTime = millis();
    // set next sample time
    sampleTime = currentTime + SAMPLEINTERVAL;
  }

  // display calculated power and bar graphs
  printInputValues(inputDelta, inputTheta, inputAlpha, inputBeta);

  // update current time on each loop; timing is based on milliseconds
  currentTime = millis();
}
