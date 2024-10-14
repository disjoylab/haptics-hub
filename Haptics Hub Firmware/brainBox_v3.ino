#include <Audio.h>
#include "BrainBox.h"
#include "output_noiseshaped_pwm.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

BrainBox                            theBrain;

AudioInputUSB                       in;
AudioOutputMQS                      mqs1; 
AudioOutputNoiseShapedPWM           PWM1(0);
AudioOutputNoiseShapedPWM           PWM2(1);
AudioOutputNoiseShapedPWM           PWM3(5);
AudioOutputNoiseShapedPWM           PWM4(6);
AudioOutputPWM                      oldpwm1;

AudioFilterBiquad                   filter1;
AudioFilterBiquad                   filter2;
AudioFilterBiquad                   filter3;
AudioFilterBiquad                   filter4;
AudioFilterBiquad                   filter5;
AudioFilterBiquad                   filter6;

AudioConnection                     patchCord0(in, 0, theBrain, 0);
AudioConnection                     patchCord1(in, 1, theBrain, 1);

AudioConnection                     patchCord2(theBrain, 0, filter1, 0);
AudioConnection                     patchCord3(theBrain, 1, filter2, 0);
AudioConnection                     patchCord4(theBrain, 2, filter3, 0);
AudioConnection                     patchCord5(theBrain, 3, filter4, 0);
AudioConnection                     patchCord6(theBrain, 4, filter5, 0);
AudioConnection                     patchCord7(theBrain, 5, filter6, 0);

AudioConnection                     patchCord8(filter1, 0, PWM1, 0);
AudioConnection                     patchCord9(filter2, 0, PWM2, 0);
AudioConnection                     patchCord10(filter3, 0, PWM3, 0);
AudioConnection                     patchCord11(filter4, 0, PWM4, 0);
AudioConnection                     patchCord12(filter5, 0, mqs1, 1);
AudioConnection                     patchCord13(filter6, 0, mqs1, 0);
AudioConnection                     patchCord14(theBrain, 6, oldpwm1, 0);

// AudioConnection                     patchCord2(theBrain, 0, PWM1, 0);
// AudioConnection                     patchCord3(theBrain, 1, PWM2, 0);
// AudioConnection                     patchCord4(theBrain, 2, PWM3, 0);
// AudioConnection                     patchCord5(theBrain, 3, PWM4, 0);
// AudioConnection                     patchCord6(theBrain, 4, mqs1, 1);
// AudioConnection                     patchCord7(theBrain, 5, mqs1, 0);
// AudioConnection                     patchCord8(theBrain, 6, oldpwm1, 0);

uint8_t r = 19;
uint8_t g = 22;
uint8_t b = 23;
uint8_t p = 18;

uint8_t c = 0;

int colors[6][3] = {
  {64, 0, 64},
  {127, 0, 0},
  {64, 32, 0},
  {0, 64, 0},
  {0, 32, 64},
  {0, 0, 127}
};

void setup() {
  Serial.begin(9600);
  AudioMemory(24);
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(p, INPUT_PULLUP);
  filter1.setLowpass(0, 1000, 0.707);
  filter2.setLowpass(0, 1000, 0.707);
  filter3.setLowpass(0, 1000, 0.707);
  filter4.setLowpass(0, 1000, 0.707);
  filter5.setLowpass(0, 1000, 0.707);
  filter6.setLowpass(0, 1000, 0.707);
}

void loop() {
  // if (analogRead(p)<127) {
  //   updateColor(c);
  // }
  updateColor(c);
  c = (c+1)%6;
  Serial.println(AudioMemoryUsageMax());
  delay(30000);
}

void updateColor(uint8_t col) {
  analogWrite(r, colors[col][0]);
  analogWrite(g, colors[col][1]);
  analogWrite(b, colors[col][2]);
}

