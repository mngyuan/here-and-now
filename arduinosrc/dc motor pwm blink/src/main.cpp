/*
  Fade

  This example shows how to fade an LED on pin 9 using the analogWrite()
  function.

  The analogWrite() function uses PWM, so if you want to change the pin you're
  using, be sure to use another PWM capable pin. On most Arduino, the PWM pins
  are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Fade
*/

#include <Arduino.h>

#define MOTOR_PIN 3

int brightness = 0;  // how bright the LED is
int fadeAmount = 25; // how many points to fade the LED by

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(MOTOR_PIN, OUTPUT);
  Serial.begin(9600);
}

void fading() {
  Serial.println(brightness);
  // set the brightness of pin 9:
  analogWrite(MOTOR_PIN, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 250) {
    fadeAmount = -fadeAmount;
  }
  // wait for 30 milliseconds to see the dimming effect
  delay(400);
}

// the loop routine runs over and over again forever:
void loop() {
  // analogWrite(MOTOR_PIN, 128);
  // delay(50);
  // analogWrite(MOTOR_PIN, 255);
  // delay(100);
  // analogWrite(MOTOR_PIN, 128);
  // delay(50);
  // analogWrite(MOTOR_PIN, 0);
  // delay(1000);
  analogWrite(MOTOR_PIN, 255);
  delay(1000);
  analogWrite(MOTOR_PIN, 0);
  delay(1000);
}
