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
 \class        FFmpegAudioResampler
 \file         filmstro_ffmpeg_FFmpegAudioResampler.h
 \brief        Converts and scales FFmpeg frames into juce format

 \author       Daniel Walz @ filmstro.com
 \date         January 25th 2017

 \description  Converts and scales FFmpeg frames into juce format

 ==============================================================================
 */

#pragma once

class FFmpegAudioResampler {
public:
    /** Creates a scaler object. It does nothing before you call setupScaler */
    FFmpegAudioResampler ()
      : resamplerContext (nullptr),
        inputSampleRate  (0),
        outputSampleRate (0)
    {}

    ~FFmpegAudioResampler ()
    {
        if (resamplerContext)
            swr_free (&resamplerContext);
    }

    /** Setup a resampler to convert sample formats and sample rates */
    void setupFrameToBufferResampler (const int64_t         in_layout,
                                      const int             in_sampleRate,
                                      const AVSampleFormat  in_format,
                                      const juce::AudioChannelSet out_channelSet,
                                      const double          out_sampleRate)
    {
        // TODO: add also mapping for surround formats, when channel order is different
        int64_t layout = AV_CH_LAYOUT_NATIVE;

        if (out_channelSet == juce::AudioChannelSet::mono())
            layout = AV_CH_LAYOUT_MONO;
        else if (out_channelSet == juce::AudioChannelSet::stereo())
            layout = AV_CH_LAYOUT_STEREO;
        else if (out_channelSet == juce::AudioChannelSet::createLCRS())
            layout = AV_CH_LAYOUT_4POINT0;
        else if (out_channelSet == juce::AudioChannelSet::createLRS())
            layout = AV_CH_LAYOUT_SURROUND;
        else if (out_channelSet == juce::AudioChannelSet::create5point0())
            layout = AV_CH_LAYOUT_5POINT0;
        else if (out_channelSet == juce::AudioChannelSet::create5point1())
            layout = AV_CH_LAYOUT_5POINT1;

        setupResampler (in_layout, in_sampleRate, in_format,
                        layout, out_sampleRate, AV_SAMPLE_FMT_FLTP);
    }

    /** Setup a resampler to convert sample formats and sample rates */
    void setupBufferToFrameResampler (const juce::AudioChannelSet in_channelSet,
                                      const double          in_sampleRate,
                                      const int64_t         out_layout,
                                      const int             out_sampleRate,
                                      const AVSampleFormat  out_format)
    {
        // TODO: add also mapping for surround formats, when channel order is different
        int64_t layout = AV_CH_LAYOUT_NATIVE;

        if (in_channelSet == juce::AudioChannelSet::mono())
            layout = AV_CH_LAYOUT_MONO;
        else if (in_channelSet == juce::AudioChannelSet::stereo())
            layout = AV_CH_LAYOUT_STEREO;
        else if (in_channelSet == juce::AudioChannelSet::createLCRS())
            layout = AV_CH_LAYOUT_4POINT0;
        else if (in_channelSet == juce::AudioChannelSet::createLRS())
            layout = AV_CH_LAYOUT_SURROUND;
        else if (in_channelSet == juce::AudioChannelSet::create5point0())
            layout = AV_CH_LAYOUT_5POINT0;
        else if (in_channelSet == juce::AudioChannelSet::create5point1())
            layout = AV_CH_LAYOUT_5POINT1;

        setupResampler (layout,     in_sampleRate,  AV_SAMPLE_FMT_FLTP,
                        out_layout, out_sampleRate, out_format);
    }

    void setupResampler (const int64_t          in_layout,
                         const int              in_sampleRate,
                         const AVSampleFormat   in_format,
                         const int64_t          out_layout,
                         const int              out_sampleRate,
                         const AVSampleFormat   out_format)
    {
        /* create scaling context */
        resamplerContext = swr_alloc_set_opts (resamplerContext,
                                               out_layout,
                                               out_format,
                                               out_sampleRate,
                                               in_layout,
                                               in_format,
                                               in_sampleRate,
                                               0,
                                               nullptr);
        if (!resamplerContext) {
            DBG ("Impossible to create scale context for the conversion");
            inputSampleRate = 0;
            outputSampleRate = 0;
            return;
        }

        if (swr_init (resamplerContext) < 0) {
            DBG ("Impossible to initialise scale context for the conversion");
            swr_free (&resamplerContext);
            inputSampleRate = 0;
            outputSampleRate = 0;
            return;
        }

        inputSampleRate = in_sampleRate;
        outputSampleRate = out_sampleRate;
    }

    /** Resamples the data from the frame to the buffer. It will return the number of actually created samples. */
    int convertFrameToBuffer (juce::AudioSampleBuffer& buffer, const AVFrame* frame)
    {
        if (! isSetup()) {
            DBG ("Resampler not properly setup");
            return 0;
        }

        const int out_samples = swr_get_out_samples (resamplerContext, frame->nb_samples);
        jassert (buffer.getNumSamples() >= out_samples);

        swr_convert (resamplerContext,
                     reinterpret_cast<uint8_t**>(buffer.getArrayOfWritePointers()),
                     out_samples,
                     (const uint8_t**)(frame->extended_data),
                     frame->nb_samples);
        return out_samples;
    }

    void reset()
    {
        swr_free (&resamplerContext);
        inputSampleRate = 0;
        outputSampleRate = 0;
    }

    bool isSetup () const
    {
        return resamplerContext &&
                inputSampleRate != 0 &&
                outputSampleRate != 0;
    }

private:
    SwrContext* resamplerContext;

    int inputSampleRate;
    int outputSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFmpegAudioResampler)
};
