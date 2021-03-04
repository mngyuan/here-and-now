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
#include <Gpio.h>
#include <cmath>
#include <libraries/AudioFile/AudioFile.h>
#include <numeric>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#define MENG_DEV true
#define MENG_POT_INPUT false
#define MAX_FACTOR 4.0

#define HX711_CLOCK_PIN 30
#define HX711_DATA_PIN 23
#define SAMPLE_MEMORY 64
#define HX711_SPREAD 10
#define USER_PRES_MIN -20000
#define USER_PRES_MAX 800000
#define SCK_ON (clockGpio.set())
#define SCK_OFF (clockGpio.clear())
#define DT_R (dataGpio.read())

void reset_converter(void);
unsigned long read_cnt(long offset);
void set_gain(int r);
void setHighPri(void);
void setup_gpio();

Gpio clockGpio;
Gpio dataGpio;
float spread_percent = HX711_SPREAD / 100.0 / 2.0;
std::vector<long> hx711Samples;
long hx711Reading = 0;
AuxiliaryTask hx711Task;
// Change this to change how often the HX711 is read (in Hz)
int readInterval = 80;
int readCount = 0;           // How long until we read again...
int readIntervalSamples = 0; // How many samples between reads

// Bela globals
// sample
std::string gOriginalFilename = "localnatives.wav";
std::vector<std::vector<float>> gOriginalSampleData;
std::string gRemixFilename = "21-01 here and now.wav";
std::vector<std::vector<float>> gRemixSampleData;
std::string gFullRemixFilename = "21-01 here and now max.wav";
std::vector<std::vector<float>> gFullRemixSampleData;
float gReadPtr = 0.0; // Position of last read sample from file
int gCurSampleReadTimes =
    0; // How many times the current read sample has been played
int gAudioFramesPerAnalogFrame = 0;
int gCount = 0; // frames encountered ever
// Set the analog channels to read from
int gAnalogInputSpeed = 0;
int gAnalogInputAmplitude = 1;
// readings - does this need to be global .. ?
float factor = 1.0;
float factorRaw = 0.0;
float amplitude;

void setHighPri(void) {
  struct sched_param sched;

  memset(&sched, 0, sizeof(sched));

  sched.sched_priority = 10;
  if (sched_setscheduler(0, SCHED_FIFO, &sched))
    printf("Warning: Unable to set high priority\n");
}

void setup_gpio() {
  clockGpio.open(HX711_CLOCK_PIN, Gpio::OUTPUT);
  dataGpio.open(HX711_DATA_PIN, Gpio::INPUT);
  SCK_OFF;
}

void reset_converter(void) {
  SCK_ON;
  usleep(60);
  SCK_OFF;
  usleep(60);
}

void set_gain(int r) {
  int i;

  // r = 0 - 128 gain ch a
  // r = 1 - 32  gain ch b
  // r = 2 - 63  gain ch a

  while (DT_R)
    ;

  for (i = 0; i < 24 + r; i++) {
    SCK_ON;
    SCK_OFF;
  }
}

unsigned long read_cnt(long offset) {
  int argc = 13; // no idea why the hx711 code reports such a long argc but it
                 // relies on it in a few places
  long count;
  int b = 0;

  count = 0;

  while (DT_R)
    ;
  b++;
  b++;
  b++;
  b++;

  for (unsigned int i = 0; i < 24; i++) {
    SCK_ON;
    count = count << 1;
    b++;
    b++;
    b++;
    b++;
    SCK_OFF;
    b++;
    b++;
    if (DT_R > 0) {
      count++;
    }
    //	b++;
    //	b++;
  }

  SCK_ON;
  b++;
  b++;
  b++;
  b++;
  SCK_OFF;
  b++;
  b++;
  b++;
  b++;
  //  count = ~0x1800000 & count;
  //  count = ~0x800000 & count;

  if (count & 0x800000) {
    count |= (long)~0xffffff;
  }

  // if things are broken this will show actual data

  if (argc < 2) {
    for (int i = 31; i >= 0; i--) {
      printf("%d ", ((count - offset) & (1 << i)) != 0);
    }

    printf("n: %10ld     -  ", count - offset);
    printf("\n");
  }

  return (count - offset);
}

void readHX711(void *) {
  long currentReading = read_cnt(0);
  hx711Reading = currentReading;
  // hx711Samples.push_back(currentReading);
  // if (hx711Samples.size() > SAMPLE_MEMORY) {
  //  hx711Samples.erase(hx711Samples.begin());
  //}

  // long average = accumulate(hx711Samples.begin(), hx711Samples.end(), 0.0) /
  //                hx711Samples.size();
  // float filter_low = (float)average * (1.0 - spread_percent);
  // float filter_high = (float)average * (1.0 + spread_percent);
  // int cleanSamples = 0;
  // long cleanSum = 0;
  // for (auto s : hx711Samples) {
  //   if (s > filter_low && s < filter_high) {
  //     cleanSum += s;
  //     cleanSamples++;
  //   }
  // }
  // if (cleanSamples == 0) {
  //   // we jumped a lot?
  //   cleanSamples = 1;
  // }

  // rt_printf("Reading: %ld\tSmooth avg: %ld\tSamples: %d\n", currentReading,
  //           cleanSum / cleanSamples, cleanSamples);
}

bool setup(BelaContext *context, void *userData) {
  gOriginalSampleData = AudioFileUtilities::load(gOriginalFilename);
  gRemixSampleData = AudioFileUtilities::load(gRemixFilename);
  gFullRemixSampleData = AudioFileUtilities::load(gFullRemixFilename);
  // Check if analog channels are enabled

  if (context->analogFrames == 0 ||
      context->analogFrames > context->audioFrames) {
    rt_printf(
        "Error: this example needs analog enabled, with 4 or 8 channels\n");
    return false;
  }
  // Useful calculations
  if (context->analogFrames)
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

  // HX711 related setup
  hx711Task = Bela_createAuxiliaryTask(readHX711, 50, "bela-hx711");
  readIntervalSamples = context->audioSampleRate / readInterval;

  // need to set pins for HX711 to gpio
  // system("config-pin P2.03 gpio");
  // system("config-pin P2.05 gpio");

  setHighPri();
  setup_gpio();
  reset_converter();

  return true;
}

void render(BelaContext *context, void *userData) {
  // times our read for aux task every readInterval hz
  if (++readCount >= readIntervalSamples) {
    readCount = 0;
    Bela_scheduleAuxiliaryTask(hx711Task);
  }

  for (unsigned int n = 0; n < context->audioFrames; n++) {
    gCount++;
    if (gAudioFramesPerAnalogFrame && !(n % gAudioFramesPerAnalogFrame)) {
      // read analog inputs and update frequency and amplitude
      // Depending on the sampling rate of the analog inputs, this will
      // happen every audio frame (if it is 44100)
      // or every two audio frames (if it is 22050)

      if (MENG_POT_INPUT) {
        factorRaw = analogRead(context, n / gAudioFramesPerAnalogFrame,
                               gAnalogInputSpeed);
        factor = map(factorRaw, 0, 1, 1.0, MAX_FACTOR);
        amplitude = analogRead(context, n / gAudioFramesPerAnalogFrame,
                               gAnalogInputAmplitude);
      } else {
        factor =
            map(hx711Reading, USER_PRES_MIN, USER_PRES_MAX, 1.0, MAX_FACTOR);
        amplitude = map(hx711Reading, USER_PRES_MIN, USER_PRES_MAX, 0.0, 1.0);
      }
      // rt_printf("Factor: %.2f\n", factor);
    }

    // 8 1
    // 1 8
    // 0.5 16
    gReadPtr += MAX_FACTOR / factor;
    if (gReadPtr > gOriginalSampleData[0].size() * MAX_FACTOR) {
      // Replay when at end
      gReadPtr = 0.0;
    }

    // Print a message once in a while
    if (gCount % (int)(context->audioSampleRate) == 0) {
      // rt_printf("FactorRaw: %.2f\tFactor: %.4f\tstep: %.4f\n", factorRaw,
      //           factor, MAX_FACTOR / factor);
      rt_printf("HX711: %ld\tAmplitude: %.2f\n", hx711Reading, amplitude);
    }

    for (unsigned int channel = 0; channel < context->audioOutChannels;
         channel++) {
      // Wrap channel index in case there are more audio output channels than
      // the file contains
      float originalOut =
          gOriginalSampleData[channel % gOriginalSampleData.size()]
                             [(int)(gReadPtr / MAX_FACTOR)];
      float remixOut = gRemixSampleData[channel % gRemixSampleData.size()]
                                       [(int)(gReadPtr / MAX_FACTOR)];
      float fullRemixOut =
          gFullRemixSampleData[channel % gFullRemixSampleData.size()]
                              [(int)(gReadPtr / MAX_FACTOR)];
      float out;
      // this system of linear equations is best understood if you draw out the
      // graph
      if (amplitude < 1.0 / 3.0) {
        out = 3.0 / 2.0 * amplitude * originalOut;
      } else if (amplitude < 2.0 / 3.0) {
        out = 3.0 * (amplitude - 1.0 / 3.0) * remixOut +
              (-3.0 / 2.0) * (amplitude - 2.0 / 3.0) * originalOut;
      } else {
        out = -3.0 * (amplitude - 1) * remixOut +
              3.0 * (amplitude - 2.0 / 3.0) * fullRemixOut;
      }

      // deadzone for when it's not being held
      if (amplitude < 0.015) {
        out = 0;
      }
      audioWrite(context, n, channel, out);
    }
  }
}

void cleanup(BelaContext *context, void *userData) {}

/**
\example sample-loader/render.cpp

Simple Sample Loader
--------------------------------

This example loads a specified range of samples from a file into a buffer using
a helper function provided in libraries/AudioFile/AudioFile.h. This should be
used when working with small wav files. See sampleStreamer and
sampleStreamerMulti for more elaborate ways of loading and playing back larger
files.
*/
