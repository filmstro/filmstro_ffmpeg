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
  \class        FFmpegVideoComponent
  \file         filmstro_ffmpeg_FFmpegVideoComponent.h
  \brief        A component to view a video decoded by FFmpeg
 
  \author       Daniel Walz @ filmstro.com
  \date         August 31st 2016

  \description  This component will display the movie in the gui. 
                It has a FFmpegVideoSource, which will send changeMessages when
                a new Video frame arrived in the stream to be displayed.

  ==============================================================================
 */


#ifndef FILMSTRO_FFMPEG_FFMPEGVIDEOCOMPONENT_H_INCLUDED
#define FILMSTRO_FFMPEG_FFMPEGVIDEOCOMPONENT_H_INCLUDED

class FFmpegVideoSource;

/**
 \class         FFmpegVideoComponent
 \description   Component to display a video read by FFmpegVideoReader
 */
class FFmpegVideoComponent :    public juce::Component,
                                public juce::Timer,
                                public FFmpegVideoListener
{
public:
    FFmpegVideoComponent ();
    virtual ~FFmpegVideoComponent ();

    /** Callback when the component is resized */
    void resized () override;

    void paint (juce::Graphics& g) override;

    /** triggers repaint on message thread */
    void timerCallback () override;

    /** callback from FFmpegVideoReader to display a new frame */
    void displayNewFrame (const AVFrame*) override;

    /** This is called whenever the size changed, so a framebuffer can be resized */
    void videoSizeChanged (const int width, const int height, const AVPixelFormat) override;

    /** Set a video source for the component */
    void setVideoReader (FFmpegVideoReader* source);

    /** Get the video source for the component */
    FFmpegVideoReader* getVideoReader () const;

private:
    /** Format the timecode in seconds */
    juce::String    formatTimeCode (const double tc);

    /** Reference to the FFmpegVideoReader to provide video frames */
    juce::WeakReference<FFmpegVideoReader>  videoSource;

    const AVFrame*                          currentFrame;

    juce::Image                             frameBuffer;

    FFmpegVideoScaler                       videoScaler;

    bool                                    dirty;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFmpegVideoComponent)
};

#endif /* FILMSTRO_FFMPEG_FFMPEGVIDEOCOMPONENT_H_INCLUDED */

