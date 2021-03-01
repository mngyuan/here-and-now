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
#include <libraries/AudioFile/AudioFile.h>
#include <vector>

std::string gFilename = "localnatives.wav";
#define MAX_FACTOR 4.0

std::vector<std::vector<float> > gSampleData;

float gReadPtr = 0.0;	// Position of last read sample from file
int gCurSampleReadTimes = 0; // How many times the current read sample has been played
int gAudioFramesPerAnalogFrame = 0;
int gCount = 0; // frames encountered ever

// Set the analog channels to read from
int gAnalogInputSpeed = 0;
int gAnalogInputAmplitude = 1;

bool setup(BelaContext *context, void *userData)
{
	gSampleData = AudioFileUtilities::load(gFilename);
		// Check if analog channels are enabled
	
	if(context->analogFrames == 0 || context->analogFrames > context->audioFrames) {
		rt_printf("Error: this example needs analog enabled, with 4 or 8 channels\n");
		return false;
	}
	// Useful calculations
	if(context->analogFrames)
		gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

	return true;
}

void render(BelaContext *context, void *userData)
{
	float factor = 1.0;
	float factorRaw = 0.0;
	float amplitude;
    for(unsigned int n = 0; n < context->audioFrames; n++) {
    	gCount++;
		if(gAudioFramesPerAnalogFrame && !(n % gAudioFramesPerAnalogFrame)) {
			// read analog inputs and update frequency and amplitude
			// Depending on the sampling rate of the analog inputs, this will
			// happen every audio frame (if it is 44100)
			// or every two audio frames (if it is 22050)
			factorRaw = analogRead(context, n/gAudioFramesPerAnalogFrame, gAnalogInputSpeed);
			factor = map(factorRaw, 0, 1, 1.0, MAX_FACTOR);
			amplitude = analogRead(context, n/gAudioFramesPerAnalogFrame, gAnalogInputAmplitude);
			// rt_printf("Factor: %.2f\n", factor);
		}
		
		// 8 1
		// 1 8
		// 0.5 16
		gReadPtr += MAX_FACTOR / factor;
        if(gReadPtr > gSampleData[0].size() * MAX_FACTOR) {
        	// Replay when at end
            gReadPtr = 0.0;
        }
        // Print a message once in a while
        if(gCount % (int)(context->audioSampleRate) == 0) {
        	rt_printf("FactorRaw: %.2f\tFactor: %.4f\tstep: %.4f\n", factorRaw, factor, MAX_FACTOR / factor);
        }

    	for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
    	    // Wrap channel index in case there are more audio output channels than the file contains
		float out = amplitude * gSampleData[channel%gSampleData.size()][(int)(gReadPtr / MAX_FACTOR)];
    		audioWrite(context, n, channel, out);
    	}
    }
}


void cleanup(BelaContext *context, void *userData)
{
}


/**
\example sample-loader/render.cpp

Simple Sample Loader
--------------------------------

This example loads a specified range of samples from a file into a buffer using a
helper function provided in libraries/AudioFile/AudioFile.h. This should be used when working
with small wav files. See sampleStreamer and sampleStreamerMulti for more elaborate ways
of loading and playing back larger files.
*/
