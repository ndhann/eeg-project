#define SAMPLEINTERVAL 7.8125
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

void setup() {
  // put your setup code here, to run once:
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

  currentTime = millis();
}
