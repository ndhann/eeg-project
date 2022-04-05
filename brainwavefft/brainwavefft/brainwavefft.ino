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

int inputReal[SAMPLES];
int output[SAMPLES];
int outputFreq[SAMPLES];

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
    while (micros() < (curTime + sampling_period_micro)) {
      // do nothing until it's time to sample again
    }
  }

  // calculate the FFT
  Q_FFT(inputReal, SAMPLES, SAMPLES);

  displayPowers();
}




//-----------------------------FFT Function----------------------------------------------//
/*
Code to perform High speed (5-7 times faster) and low accuracy FFT on arduino,
This code compromises accuracy for speed,
setup:

1. in[]     : Data array, 
2. N        : Number of sample (recommended sample size 2,4,8,16,32,64,128...)
3. Frequency: sampling frequency required as input (Hz)

It will by default return frequency with max aplitude,

If sample size is not in power of 2 it will be clipped to lower side of number. 
i.e, for 150 number of samples, code will consider first 128 sample, remaining sample  will be omitted.
For Arduino nano, FFT of more than 256 sample not possible due to mamory limitation 
Code by ABHILASH
Contact: abhilashpatel121@gmail.com 
Documentation & deatails: https://www.instructables.com/member/abhilash_patel/instructables/
*/


float Q_FFT(int in[],int N,float Frequency)
{ 

unsigned int Pow2[13]={1,2,4,8,16,32,64,128,256,512,1024,2048}; // declaring this as global array will save 1-2 ms of time


int a,c1,f,o,x;         
byte check=0;
a=N;  
                                 
      for(int i=0;i<12;i++)                 //calculating the levels
         { if(Pow2[i]<=a){o=i;} }
      
int out_r[Pow2[o]]={};   //real part of transform
int out_im[Pow2[o]]={};  //imaginory part of transform
           
x=0;  
      for(int b=0;b<o;b++)                     // bit reversal
         {
          c1=Pow2[b];
          f=Pow2[o]/(c1+c1);
                for(int j=0;j<c1;j++)
                    { 
                     x=x+1;
                     out_im[x]=out_im[j]+f;
                    }
         }

 
      for(int i=0;i<Pow2[o];i++)            // update input array as per bit reverse order
         {
          out_r[i]=in[out_im[i]]; 
          out_im[i]=0;
         }


int i10,i11,n1,tr,ti;
float e;
int c,s;
    for(int i=0;i<o;i++)                                    //fft
    {
     i10=Pow2[i];              // overall values of sine/cosine  
     i11=Pow2[o]/Pow2[i+1];    // loop with similar sine cosine
     e=360/Pow2[i+1];
     e=0-e;
     n1=0;

          for(int j=0;j<i10;j++)
          {
            c=e*j;
  while(c<0){c=c+360;}
  while(c>360){c=c-360;}

          n1=j;
          
          for(int k=0;k<i11;k++)
                 {

       if(c==0) { tr=out_r[i10+n1];
                  ti=out_im[i10+n1];}
  else if(c==90){ tr= -out_im[i10+n1];
                  ti=out_r[i10+n1];}
  else if(c==180){tr=-out_r[i10+n1];
                  ti=-out_im[i10+n1];}
  else if(c==270){tr=out_im[i10+n1];
                  ti=-out_r[i10+n1];}
  else if(c==360){tr=out_r[i10+n1];
                  ti=out_im[i10+n1];}
  else if(c>0  && c<90)   {tr=out_r[i10+n1]-out_im[i10+n1];
                           ti=out_im[i10+n1]+out_r[i10+n1];}
  else if(c>90  && c<180) {tr=-out_r[i10+n1]-out_im[i10+n1];
                           ti=-out_im[i10+n1]+out_r[i10+n1];}
  else if(c>180 && c<270) {tr=-out_r[i10+n1]+out_im[i10+n1];
                           ti=-out_im[i10+n1]-out_r[i10+n1];}
  else if(c>270 && c<360) {tr=out_r[i10+n1]+out_im[i10+n1];
                           ti=out_im[i10+n1]-out_r[i10+n1];}
          
                 out_r[n1+i10]=out_r[n1]-tr;
                 out_r[n1]=out_r[n1]+tr;
                 if(out_r[n1]>15000 || out_r[n1]<-15000){check=1;}
          
                 out_im[n1+i10]=out_im[n1]-ti;
                 out_im[n1]=out_im[n1]+ti;
                 if(out_im[n1]>15000 || out_im[n1]<-15000){check=1;}          
          
                 n1=n1+i10+i10;
                  }       
             }

    if(check==1){                                             // scale the matrics if value higher than 15000 to prevent varible from overloading
                for(int i=0;i<Pow2[o];i++)
                    {
                     out_r[i]=out_r[i]/100;
                     out_im[i]=out_im[i]/100;    
                    }
                     check=0;  
                }           

     }

/*
for(int i=0;i<Pow2[o];i++)
{
Serial.print(out_r[i]);
Serial.print("\t");                                     // un comment to print RAW o/p    
Serial.print(out_im[i]); Serial.println("i");      
}
*/

//---> here onward out_r contains amplitude and our_in conntains frequency (Hz)
int fout,fm,fstp;
float fstep;
fstep=Frequency/N;
fstp=fstep;
fout=0;fm=0;

    for(int i=1;i<Pow2[o-1];i++)               // getting amplitude from compex number
        {
        if((out_r[i]>=0) && (out_im[i]>=0)){out_r[i]=out_r[i]+out_im[i];}
   else if((out_r[i]<=0) && (out_im[i]<=0)){out_r[i]=-out_r[i]-out_im[i];}
   else if((out_r[i]>=0) && (out_im[i]<=0)){out_r[i]=out_r[i]-out_im[i];}
   else if((out_r[i]<=0) && (out_im[i]>=0)){out_r[i]=-out_r[i]+out_im[i];}
   // to find peak sum of mod of real and imaginery part are considered to increase speed
        
out_im[i]=out_im[i-1]+fstp;
if (fout<out_r[i]){fm=i; fout=out_r[i];}
         
         Serial.print(out_im[i]);Serial.print("Hz");
         Serial.print("\t");                            // un comment to print freuency bin    
         Serial.println(out_r[i]); 
          
        }


float fa,fb,fc;
fa=out_r[fm-1];
fb=out_r[fm]; 
fc=out_r[fm+1];
fstep=(fa*(fm-1)+fb*fm+fc*(fm+1))/(fa+fb+fc);

for (int i = 1; i < Pow2[o-1]; i++) {
  output[i] = out_r[i];
  outputFreq[i] = out_im[i];
}

return(fstep*Frequency/N);
}
    

//------------------------------------------------------------------------------------//
