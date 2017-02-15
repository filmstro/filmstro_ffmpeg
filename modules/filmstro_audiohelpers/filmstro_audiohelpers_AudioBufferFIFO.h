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
 \class        AudioBufferFIFO
 \file         filmstro_audiobasics_AudioBufferFIFO.h
 \brief        A FIFO for audio samples

 \author       Daniel Walz / Filmstro Ltd.
 \date         September 7th 2016

 \description  A FIFO for AudioBuffer / samples

 ==============================================================================
 */

#ifndef FSPRO_AUDIOBASICS_AUDIOBUFFERFIFO_H_INCLUDED
#define FSPRO_AUDIOBASICS_AUDIOBUFFERFIFO_H_INCLUDED

/**
 The AudioBufferFIFO implements an actual sample buffer using JUCEs AbstractFIFO
 class. You can add samples from the various kind of formats, like float pointers
 or AudioBuffers. Then you can read into float arrays, AudioBuffers or even
 AudioSourceChannelInfo to be used directly in AudioSources.
 */
template<typename FloatType>
class AudioBufferFIFO : public juce::AbstractFifo
{
public:
    /*< Creates a FIFO with a buffer of given number of channels and given number of samples */
    AudioBufferFIFO (int channels, int buffersize) :
        AbstractFifo (buffersize)
    {
        buffer.setSize (channels, buffersize);
    }

    /*< Resize the buffer with new number of channels and new number of samples */
    void setSize (const int channels, const int newBufferSize)
    {
        buffer.setSize (channels, newBufferSize);
        setTotalSize (newBufferSize);
        reset ();
    }

    /*< Push samples into the FIFO from raw float arrays */
    void addToFifo (const FloatType** samples, int numSamples)
    {
        jassert (getFreeSpace() >= numSamples);
        int start1, size1, start2, size2;
        prepareToWrite (numSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom (channel, start1, samples[channel], size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom (channel, start2, samples[channel] + size1, size2);
        finishedWrite (size1 + size2);
    }

    /*< Push samples from an AudioBuffer into the FIFO */
    void addToFifo (const juce::AudioBuffer<FloatType>& samples, int numSamples=-1)
    {
        const int addSamples = numSamples < 0 ? samples.getNumSamples() : numSamples;
        jassert (getFreeSpace() >= addSamples);

        int start1, size1, start2, size2;
        prepareToWrite (addSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom (channel, start1, samples.getReadPointer (channel), size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                buffer.copyFrom (channel, start2, samples.getReadPointer (channel, size1), size2);
        finishedWrite (size1 + size2);

    }

    /*< Read samples from the FIFO into raw float arrays */
    void readFromFifo (FloatType** samples, int numSamples)
    {
        jassert (getNumReady() >= numSamples);
        int start1, size1, start2, size2;
        prepareToRead (numSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                juce::FloatVectorOperations::copy (samples [channel],
                                                   buffer.getReadPointer (channel, start1),
                                                   size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                juce::FloatVectorOperations::copy (samples [channel] + size1,
                                                   buffer.getReadPointer (channel, start2),
                                                   size2);
        finishedRead (size1 + size2);
    }

    /*< Read samples from the FIFO into AudioBuffers */
    void readFromFifo (juce::AudioBuffer<FloatType>& samples, int numSamples=-1)
    {
        const int readSamples = numSamples > 0 ? numSamples : samples.getNumSamples();
        jassert (getNumReady() >= readSamples);

        int start1, size1, start2, size2;
        prepareToRead (readSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                samples.copyFrom (channel, 0, buffer.getReadPointer (channel, start1), size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                samples.copyFrom (channel, size1, buffer.getReadPointer (channel, start2), size2);
        finishedRead (size1 + size2);
    }

    /*< Read samples from the FIFO into AudioSourceChannelInfo buffers to be used in AudioSources getNextAudioBlock */
    void readFromFifo (const juce::AudioSourceChannelInfo& info, int numSamples=-1)
    {
        const int readSamples = numSamples > 0 ? numSamples : info.numSamples;
        jassert (getNumReady() >= readSamples);

        int start1, size1, start2, size2;
        prepareToRead (readSamples, start1, size1, start2, size2);
        if (size1 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                    info.buffer->copyFrom (channel, info.startSample, buffer.getReadPointer (channel, start1), size1);
        if (size2 > 0)
            for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                info.buffer->copyFrom (channel, info.startSample + size1, buffer.getReadPointer (channel, start2), size2);
        finishedRead (size1 + size2);
    }

    /*< Returns the number of channels of the underlying buffer */
    int getNumChannels () const {
        return buffer.getNumChannels();
    }

    /*< Clears all samples and sets the FIFO state to empty */
    void clear () {
        buffer.clear ();
        reset();
    }

private:
    /*< The actual audio buffer */
    juce::AudioBuffer<FloatType>    buffer;
};




#endif /* FSPRO_AUDIOBASICS_AUDIOBUFFERFIFO_H_INCLUDED */

