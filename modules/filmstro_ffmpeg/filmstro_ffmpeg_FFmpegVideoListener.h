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
 \class        FFmpegVideoListener
 \file         filmstro_ffmpeg_FFmpegVideoListener.h
 \brief        Interface for video related callbacks from FFmpeg

 \author       Daniel Walz @ filmstro.com
 \date         January 25th 2017

 \description  This is an interface for objects, that want to receive video frames

 ==============================================================================
 */


#ifndef FILMSTRO_FFMPEG_FFMPEGVIDEOLISTENER_H_INCLUDED
#define FILMSTRO_FFMPEG_FFMPEGVIDEOLISTENER_H_INCLUDED

class FFmpegVideoListener {

public:

    FFmpegVideoListener() {}

    virtual ~FFmpegVideoListener() {}

    /** This will notify about advancing the presentation timestamp */
    virtual void presentationTimestampChanged (const double) {}

    /** This is called whenever the size changed, so a framebuffer can be resized */
    virtual void videoSizeChanged (const int width, const int height, const AVPixelFormat) {}

    /** This is called when a frame is read from the video stream. It will be
     called in the correct sequence, but not synchronised to the audio stream.
     You can use that for transcoding a video stream */
    virtual void readRawFrame (const AVFrame*) {}

    /** This is called when a frame is due to be displayed according to audio's
     presentation timestamp PTS as raw frame */
    virtual void displayNewFrame (const AVFrame*) {}

    /** This is called when the video source file has changed */
    virtual void videoFileChanged (const juce::File& newSource) {}

};

#endif /* FILMSTRO_FFMPEG_FFMPEGVIDEOLISTENER_H_INCLUDED */
