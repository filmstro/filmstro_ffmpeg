/*
 ==============================================================================
 Copyright (c) 2017, Filmstro Ltd. - Daniel Walz
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 3. Neither the name of the copyright holder nor the names of its contributors
 may be used to endorse or promote products derived from this software without
 specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 OF THE POSSIBILITY OF SUCH DAMAGE.
 ==============================================================================
 \class        AudioProcessorPlayerSource
 \file         filmstro_audiobasics_AudioProcessorPlayerSource.h
 \brief        An AudioSource to playback audioProcessors

 \author       Daniel Walz / Filmstro Ltd.
 \date         December 15th 2016

 \description  An AudioSource playing back an AudioProcessor

 ==============================================================================
 */


#ifndef filmstro_audiobasics_AudioProcessorPlayerSource_h
#define filmstro_audiobasics_AudioProcessorPlayerSource_h

class AudioProcessorPlayerSource : public juce::AudioSource
{
public:
    AudioProcessorPlayerSource (juce::AudioProcessor* proc, const int channels = 2,
                                const bool deleteProcessor = true)
      : processor (proc, deleteProcessor),
        numChannels (channels),
        fifo (channels, 4096)
    {
        
    }

    virtual ~AudioProcessorPlayerSource ()
    {
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        buffer.setSize (numChannels, samplesPerBlockExpected);
        if (processor) {
            buffer.setSize (numChannels, samplesPerBlockExpected);
            processor->prepareToPlay (sampleRate, samplesPerBlockExpected);
        }
    }

    void releaseResources () override
    {
        if (processor) {
            processor->releaseResources ();
        }
    }

    void setProcessor (juce::AudioProcessor* proc, const bool deleteProcessor)
    {
        fifo.clear ();
        processor.set (proc, deleteProcessor);
    }

    juce::AudioProcessor* getCurrentProcessor () const
    {
        return processor;
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        if (processor) {
            juce::MidiBuffer midiBuffer;
            while (fifo.getNumReady() < bufferToFill.numSamples) {
                processor->processBlock (buffer, midiBuffer);
                fifo.addToFifo (buffer);
            }
            const int ready = fifo.getNumReady ();
            if (ready >= bufferToFill.numSamples)
            {
                fifo.readFromFifo (bufferToFill);
            }
            else if (ready > 0) {
                fifo.readFromFifo (bufferToFill, ready);
                bufferToFill.buffer->clear (bufferToFill.startSample + ready, bufferToFill.numSamples - ready);
            }
            else {
                bufferToFill.clearActiveBufferRegion ();
            }
        }
        else {
            bufferToFill.clearActiveBufferRegion();
        }
    }

private:

    juce::OptionalScopedPointer<juce::AudioProcessor> processor;

    int                       numChannels;

    // buffer some audio data
    AudioBufferFIFO<float>    fifo;

    // processor buffer
    juce::AudioBuffer<float>  buffer;
};



#endif /* filmstro_audiobasics_AudioProcessorPlayerSource_h */
