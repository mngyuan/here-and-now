/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>

#define MOTOR_PIN 3

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(MOTOR_PIN, OUTPUT);
}

void loop()
{
  // // set the brightness of pin 9:
  // analogWrite(MOTOR_PIN, brightness);

  // // change the brightness for next time through the loop:
  // brightness = brightness + fadeAmount;

  // // reverse the direction of the fading at the ends of the fade:
  // if (brightness <= 0 || brightness > 255) {
  //   fadeAmount = -fadeAmount;
  // }
  // // wait for 30 milliseconds to see the dimming effect
  // delay(30);
  analogWrite(MOTOR_PIN, 0);
  delay(1000);
  analogWrite(MOTOR_PIN, 255);
  delay(1000);
}

