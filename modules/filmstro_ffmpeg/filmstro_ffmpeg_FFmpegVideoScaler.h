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
 \file         filmstro_ffmpeg_FFmpegVideoScaler.h
 \brief        Converts and scales FFmpeg frames into juce format

 \author       Daniel Walz @ filmstro.com
 \date         January 25th 2017

 \description  Converts and scales FFmpeg frames into juce format

 ==============================================================================
 */


#ifndef FILMSTRO_FFMPEG_FFMPEGVIDEOSCALER_H_INCLUDED
#define FILMSTRO_FFMPEG_FFMPEGVIDEOSCALER_H_INCLUDED

class FFmpegVideoScaler {
public:
    FFmpegVideoScaler ();
    ~FFmpegVideoScaler ();

    void setupScaler (const int in_width,  const int in_height,  const AVPixelFormat in_format,
                      const int out_width, const int out_height, const AVPixelFormat out_format);

    void convertFrameToImage (juce::Image& image, const AVFrame* frame);

    void convertImageToFrame (AVFrame* frame, const juce::Image& image);

private:
    SwsContext* scalerContext;

    int         inLinesizes[4];
    int         outLinesizes[4];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFmpegVideoScaler)
};

#endif /* FILMSTRO_FFMPEG_FFMPEGVIDEOSCALER_H_INCLUDED */
