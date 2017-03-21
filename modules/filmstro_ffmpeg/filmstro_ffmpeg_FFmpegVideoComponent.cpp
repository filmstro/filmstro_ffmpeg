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
  \file         filmstro_ffmpeg_FFmpegVideoComponent.cpp
  \brief        A component to view a video decoded by FFmpeg

  \author       Daniel Walz @ filmstro.com
  \date         August 31st 2016

  \description  This component will display the movie in the gui.
                It has a FFmpegVideoSource, which will send changeMessages when
                a new Video frame arrived in the stream to be displayed.

  ==============================================================================

  Ripped from example:
  http://ffmpeg.org/doxygen/trunk/demuxing_decoding_8c-example.html

  ==============================================================================
 */


#include "../JuceLibraryCode/JuceHeader.h"

// ==============================================================================
// FFmpegVideoComponent
// ==============================================================================

FFmpegVideoComponent::FFmpegVideoComponent ()
  : currentFrame (nullptr),
    dirty        (true)
{
    setOpaque (true);
    startTimerHz (80);
}

FFmpegVideoComponent::~FFmpegVideoComponent ()
{
}

void FFmpegVideoComponent::resized ()
{
    if (videoSource) {
        double aspectRatio = videoSource->getVideoAspectRatio() * videoSource->getVideoPixelAspect();
        if (aspectRatio > 0) {
            double w = getWidth();
            double h = getHeight();
            if (w / h > aspectRatio) {
                w = h * aspectRatio;
            }
            else {
                h = w / aspectRatio;
            }
            frameBuffer = Image (Image::PixelFormat::ARGB, static_cast<int> (w), static_cast<int> (h), true);
            videoScaler.setupScaler (videoSource->getVideoWidth(),
                                     videoSource->getVideoHeight(),
                                     videoSource->getPixelFormat(),
                                     frameBuffer.getWidth(), 
                                     frameBuffer.getHeight (), 
                                     AV_PIX_FMT_BGR0);
        }
    }
}

void FFmpegVideoComponent::timerCallback ()
{
    if (dirty) {
        dirty = false;
        repaint ();
    }
}

void FFmpegVideoComponent::paint (juce::Graphics& g)
{
    g.fillAll (Colours::black);
    if (videoSource && currentFrame && frameBuffer.isValid()) {
        videoScaler.convertFrameToImage (frameBuffer, currentFrame);
        g.drawImageAt (frameBuffer,
                       (getWidth() - frameBuffer.getWidth()) * 0.5,
                       (getHeight() - frameBuffer.getHeight()) * 0.5);
    }
    else {
        g.drawFittedText ("No VideoSource connected", getLocalBounds(), juce::Justification::centred, 1);
    }
}

/** callback from FFmpegVideoReader to display a new frame */
void FFmpegVideoComponent::displayNewFrame (const AVFrame* frame)
{
    if (dirty) {
        DBG ("Frame not painted: " + String (av_frame_get_best_effort_timestamp (currentFrame)));
    }
    currentFrame = frame;
    dirty = true;
}

/** This is called whenever the size changed, so a framebuffer can be resized */
void FFmpegVideoComponent::videoSizeChanged (const int width, const int height, const AVPixelFormat format)
{
    resized();
    dirty = true;
}

void FFmpegVideoComponent::setVideoReader (FFmpegVideoReader* source)
{
    if (videoSource)
        videoSource->removeVideoListener (this);

    videoSource = source;

    if (videoSource)
        videoSource->addVideoListener(this);

    dirty = true;
}

FFmpegVideoReader* FFmpegVideoComponent::getVideoReader () const
{
    return videoSource;
}
