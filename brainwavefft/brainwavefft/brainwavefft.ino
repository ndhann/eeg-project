#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 16
#define RST_PIN 4
#define I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, RST_PIN);

// calculated from the ADC ranges (0-5V and 0-1023)
#define VOLTSPERBIT 0.0049
#define SAMPLES 128

unsigned int sampling_period_micro;
unsigned long curTime;

int inputReal[SAMPLES];
int output[SAMPLES];
int outputFreq[SAMPLES];

double power[SAMPLES];

double deltapower = 0.0;
double thetapower = 0.0;
double alphapower = 0.0;
double betapower = 0.0;

// The pin to receive the signal to process
const int inputPin = A1;

void setup() {
  Serial.begin(115200);

  // Set up display
  if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);

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
    deltapower += sq(inputReal[i]);
  }
  deltapower = sqrt(deltapower / 3.0);

  // Theta: 4-8Hz
  for (int i = 4; i <= 8; i++) {
    thetapower += sq(inputReal[i]);
  }
  thetapower = sqrt(thetapower / 5.0);

  // Alpha: 9-12Hz
  for (int i = 9; i <= 12; i++) {
    alphapower += sq(inputReal[i]);
  }
  alphapower = sqrt(alphapower / 4.0);

  // Beta: 13-30Hz
  for (int i = 13; i <= 30; i++) {
    betapower += sq(inputReal[i]);
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
    display.print("|");
  }

  if (((power / maxpower) > (0.6))) {
    display.print("|");
  }

  if (((power / maxpower) > (0.4))) {
    display.print("|");
  }

  if (((power / maxpower) > (0.2))) {
    display.print("|");
  }

  if (((power / maxpower) > (0.1))) {
    display.print("|");
  }
}

void drawBarGraph() {
  for (int i = 1; i <= 30; i++) {
    display.drawLine(4 * i, 63, 4 * i, output[i], WHITE);
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

// displays values and the bar graph on the display display
void displayPowers() {
  calculatePowers();
  double maxpower = findMaxPower(deltapower, thetapower, alphapower, betapower);
  Serial.println(maxpower);

  // clear the screen every time calculations are refreshed
  display.clearDisplay();

  // write out values and draw bars for each of the frequency ranges
  display.print("Delta: ");
  display.print(deltapower);
  display.print(" ");
  display.println();

  display.print("Theta: ");
  display.print(thetapower);
  display.print(" ");
  display.println();

  display.print("Alpha: ");
  display.print(alphapower);
  display.print(" ");
  display.println();

  display.print("Beta:  ");
  display.print(betapower);
  display.print(" ");
  display.println();

  drawBarGraph();
  display.display();
}

void loop() {
  // loop and obtain 128 samples over one second
  for (int i = 0; i < SAMPLES; i++) {
    curTime = micros();
    inputReal[i] = analogRead(inputPin) * VOLTSPERBIT; // read the ADC value and convert to volts
    while (micros() < (curTime + sampling_period_micro)) {
      // do nothing until it's time to sample again
    }
  }

  // calculate the FFT
  FFT(inputReal, SAMPLES, 128.0);

  displayPowers();
}

//---------------------------------------------------------------------------//
byte sine_data [91] =
{
  0,
  4,    9,    13,   18,   22,   27,   31,   35,   40,   44,
  49,   53,   57,   62,   66,   70,   75,   79,   83,   87,
  91,   96,   100,  104,  108,  112,  116,  120,  124,  127,
  131,  135,  139,  143,  146,  150,  153,  157,  160,  164,
  167,  171,  174,  177,  180,  183,  186,  189,  192,  195,       //Paste this at top of program
  198,  201,  204,  206,  209,  211,  214,  216,  219,  221,
  223,  225,  227,  229,  231,  233,  235,  236,  238,  240,
  241,  243,  244,  245,  246,  247,  248,  249,  250,  251,
  252,  253,  253,  254,  254,  254,  255,  255,  255,  255
};
float f_peaks[5]; // top 5 frequencies peaks in descending order
//---------------------------------------------------------------------------//

//-----------------------------FFT Function----------------------------------------------//

float FFT(int in[], int N, float Frequency)
{
  /*
    Code to perform FFT on arduino,
    setup:
    paste sine_data [91] at top of program [global variable], paste FFT function at end of program
    Term:
    1. in[]     : Data array,
    2. N        : Number of sample (recommended sample size 2,4,8,16,32,64,128...)
    3. Frequency: sampling frequency required as input (Hz)

    If sample size is not in power of 2 it will be clipped to lower side of number.
    i.e, for 150 number of samples, code will consider first 128 sample, remaining sample  will be omitted.
    For Arduino nano, FFT of more than 128 sample not possible due to mamory limitation (64 recomended)
    For higher Number of sample may arise Mamory related issue,
    Code by ABHILASH
    Contact: abhilashpatel121@gmail.com
    Documentation:https://www.instructables.com/member/abhilash_patel/instructables/
    2/3/2021: change data type of N from float to int for >=256 samples
  */

  unsigned int data[13] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};
  int a, c1, f, o, x;
  a = N;

  for (int i = 0; i < 12; i++)          //calculating the levels
  {
    if (data[i] <= a) {
      o = i;
    }
  }

  int in_ps[data[o]] = {};   //input for sequencing
  float out_r[data[o]] = {}; //real part of transform
  float out_im[data[o]] = {}; //imaginory part of transform

  x = 0;
  for (int b = 0; b < o; b++)              // bit reversal
  {
    c1 = data[b];
    f = data[o] / (c1 + c1);
    for (int j = 0; j < c1; j++)
    {
      x = x + 1;
      in_ps[x] = in_ps[j] + f;
    }
  }


  for (int i = 0; i < data[o]; i++)     // update input array as per bit reverse order
  {
    if (in_ps[i] < a)
    {
      out_r[i] = in[in_ps[i]];
    }
    if (in_ps[i] > a)
    {
      out_r[i] = in[in_ps[i] - a];
    }
  }


  int i10, i11, n1;
  float e, c, s, tr, ti;

  for (int i = 0; i < o; i++)                             //fft
  {
    i10 = data[i];            // overall values of sine/cosine  :
    i11 = data[o] / data[i + 1]; // loop with similar sine cosine:
    e = 360 / data[i + 1];
    e = 0 - e;
    n1 = 0;

    for (int j = 0; j < i10; j++)
    {
      c = cosine(e * j);
      s = sine(e * j);
      n1 = j;

      for (int k = 0; k < i11; k++)
      {
        tr = c * out_r[i10 + n1] - s * out_im[i10 + n1];
        ti = s * out_r[i10 + n1] + c * out_im[i10 + n1];

        out_r[n1 + i10] = out_r[n1] - tr;
        out_r[n1] = out_r[n1] + tr;

        out_im[n1 + i10] = out_im[n1] - ti;
        out_im[n1] = out_im[n1] + ti;

        n1 = n1 + i10 + i10;
      }
    }
  }

  /*
    for(int i=0;i<data[o];i++)
    {
    Serial.print(out_r[i]);
    Serial.print("\t");                                     // un comment to print RAW o/p
    Serial.print(out_im[i]); Serial.println("i");
    }
  */


  //---> here onward out_r contains amplitude and our_in conntains frequency (Hz)
  for (int i = 0; i < data[o - 1]; i++)      // getting amplitude from compex number
  {
    out_r[i] = sqrt(out_r[i] * out_r[i] + out_im[i] * out_im[i]); // to  increase the speed delete sqrt
    out_im[i] = i * Frequency / N;

    Serial.print(out_im[i]); Serial.print("Hz");
    Serial.print("\t");                            // un comment to print freuency bin
    Serial.println(out_r[i]);

  }

  // copy to external
  for (int i = 1; i < data[o - 1]; i++) {
    output[i] = out_r[i];
    outputFreq[i] = out_im[i];
  }




  x = 0;     // peak detection
  for (int i = 1; i < data[o - 1] - 1; i++)
  {
    if (out_r[i] > out_r[i - 1] && out_r[i] > out_r[i + 1])
    { in_ps[x] = i;  //in_ps array used for storage of peak number
      x = x + 1;
    }
  }


  s = 0;
  c = 0;
  for (int i = 0; i < x; i++)      // re arraange as per magnitude
  {
    for (int j = c; j < x; j++)
    {
      if (out_r[in_ps[i]] < out_r[in_ps[j]])
      { s = in_ps[i];
        in_ps[i] = in_ps[j];
        in_ps[j] = s;
      }
    }
    c = c + 1;
  }



  for (int i = 0; i < 5; i++) // updating f_peak array (global variable)with descending order
  {
    f_peaks[i] = out_im[in_ps[i]];
  }




}


float sine(int i)
{
  int j = i;
  float out;
  while (j < 0) {
    j = j + 360;
  }
  while (j > 360) {
    j = j - 360;
  }
  if (j > -1   && j < 91) {
    out = sine_data[j];
  }
  else if (j > 90  && j < 181) {
    out = sine_data[180 - j];
  }
  else if (j > 180 && j < 271) {
    out = -sine_data[j - 180];
  }
  else if (j > 270 && j < 361) {
    out = -sine_data[360 - j];
  }
  return (out / 255);
}

float cosine(int i)
{
  int j = i;
  float out;
  while (j < 0) {
    j = j + 360;
  }
  while (j > 360) {
    j = j - 360;
  }
  if (j > -1   && j < 91) {
    out = sine_data[90 - j];
  }
  else if (j > 90  && j < 181) {
    out = -sine_data[j - 90];
  }
  else if (j > 180 && j < 271) {
    out = -sine_data[270 - j];
  }
  else if (j > 270 && j < 361) {
    out = sine_data[j - 270];
  }
  return (out / 255);
}

//------------------------------------------------------------------------------------//
