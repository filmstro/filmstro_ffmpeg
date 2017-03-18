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
 \class        FFmpegVideoScaler
 \file         filmstro_ffmpeg_FFmpegVideoScaler.cpp
 \brief        Converts and scales FFmpeg frames into juce format

 \author       Daniel Walz @ filmstro.com
 \date         January 25th 2017

 \description  Converts and scales FFmpeg frames into juce format

 ==============================================================================
 */

#include "../JuceLibraryCode/JuceHeader.h"


FFmpegVideoScaler::FFmpegVideoScaler ()
: scalerContext (nullptr)
{

}

FFmpegVideoScaler::~FFmpegVideoScaler ()
{
    if (scalerContext) {
        sws_freeContext (scalerContext);
        scalerContext = nullptr;
    }
}

void FFmpegVideoScaler::setupScaler (const int in_width,  const int in_height,  const AVPixelFormat in_format,
                                     const int out_width, const int out_height, const AVPixelFormat out_format)
{
    if (scalerContext) {
        sws_freeContext (scalerContext);
        scalerContext = nullptr;
    }

    /* create scaling context */
    scalerContext = sws_getContext (in_width,  in_height, in_format,
                                    out_width, out_height, out_format,
                                    SWS_BILINEAR, NULL, NULL, NULL);
    if (!scalerContext) {
        DBG ("Impossible to create scale context for the conversion");
    }
}

void FFmpegVideoScaler::convertFrameToImage (juce::Image& image, const AVFrame* frame)
{
    if (scalerContext) {
        Image::BitmapData data (image, 0, 0,
                                image.getWidth(),
                                image.getHeight(),
                                Image::BitmapData::writeOnly);

        int bitsPerLine = 4 * image.getWidth();
        int linesizes[4] = {bitsPerLine, bitsPerLine, bitsPerLine, bitsPerLine};

        uint8_t* destination[4] = {data.data, data.data, data.data, data.data};

        sws_scale (scalerContext,
                   frame->data,
                   frame->linesize,
                   0,
                   frame->height,
                   destination,
                   linesizes);
    }
}

void FFmpegVideoScaler::convertImageToFrame (AVFrame* frame, const juce::Image& image)
{
    if (scalerContext) {
        Image::BitmapData data (image, 0, 0,
                                image.getWidth(),
                                image.getHeight());

        uint8_t* source[4] = {data.data, data.data, data.data, data.data};
        int bitsPerLine = 4 * image.getWidth();
        int linesizes[4] = {bitsPerLine, bitsPerLine, bitsPerLine, bitsPerLine};

        sws_scale (scalerContext,
                   source,
                   linesizes,
                   0,
                   image.getHeight(),
                   frame->data,
                   frame->linesize);
    }
}
