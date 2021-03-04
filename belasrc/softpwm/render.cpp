/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
  Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
  Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/

#include <Bela.h>
#include <stdlib.h>

#define FULL_CW 2.0
#define FULL_CCW 1.0
#define FULL_STOP 1.5

int softPwmPin = 12; // Digital pin 4 - check the pin diagram in the IDE
float period = 20.0; // duration (in ms) of one pulse of the square wave
float
    periodSampleLength; // duration (in samples) of one pulse of the square wave
float highTime = FULL_CW; // duration (in ms) of high part of pulse
float highTimeSampleLength;

bool setup(BelaContext *context, void *userData) {
  periodSampleLength = period * (context->audioSampleRate) / 1000.0;
  // highTimeSampleLength = highTime * (context->audioSampleRate) / 1000.0;
  highTimeSampleLength = 85;
  rt_printf("Sample Rate: %.2f\tPeriod: %.2f\tPulse Width: %.2f\n",
            context->digitalSampleRate, periodSampleLength,
            highTimeSampleLength);
  pinMode(context, 0, softPwmPin, OUTPUT);
  system("config-pin P2.03 gpio");
  return true;
}

void render(BelaContext *context, void *userData) {
  for (unsigned int n = 0; n < context->digitalFrames; ++n) {
    static unsigned int count = 0;
    int value = 0;
    if (count <= highTimeSampleLength) {
      rt_printf("h");
      value = 1;
    } else {
      rt_printf("l");
      value = 0;
    }

    digitalWriteOnce(context, n, softPwmPin, value);

    count++;
    if (count == periodSampleLength)
      count = 0;

    if ((int)context->audioFramesElapsed % (int)context->audioSampleRate == 0) {
      rt_printf("%ds elapsed\n",
                (int)(context->audioFramesElapsed / context->audioSampleRate));
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}