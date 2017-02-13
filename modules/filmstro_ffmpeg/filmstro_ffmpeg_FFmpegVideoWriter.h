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
  \class        FFmpegVideoWriter
  \file         filmstro_ffmpeg_FFmpegVideoWriter.h
  \brief        A class to write a video stream using FFmpeg
 
  \author       Daniel Walz @ filmstro.com
  \date         September 19th 2016

  \description  Use this writer to write audio frames and video frames in a
                video file using ffmpeg
  ==============================================================================
 */


#ifndef FILMSTRO_FFMPEG_FFMPEGVIDEOWRITER_H_INCLUDED
#define FILMSTRO_FFMPEG_FFMPEGVIDEOWRITER_H_INCLUDED

class FFmpegVideoSource;


class FFmpegVideoWriter
{
public:

    FFmpegVideoWriter();
    ~FFmpegVideoWriter();

    bool openMovieFile (const juce::File& outputFile, double sampleRate);

    void setVideoReader (FFmpegVideoReader* source);

    void finishWriting ();

    bool writeAudioFrame (const juce::AudioSampleBuffer& bufferToWrite, int startSample, int numSamples);

    bool writeAudioFrame (const juce::AudioSampleBuffer& bufferToWrite);

    bool writeVideoFrame (AVFrame* frame);

    // ==============================================================================
private:

    // source to provide video frames for writing
    juce::WeakReference<FFmpegVideoReader> videoSource;

    AVFormatContext*        formatContext;

    int                     videoStreamIdx;
    int                     audioStreamIdx;
    int                     subtitleStreamIdx;

    // buffer audio to match the video's audio frame size
    AudioBufferFIFO<float>  audioFifo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFmpegVideoWriter)
};

#endif /* FILMSTRO_FFMPEG_FFMPEGVIDEOWRITER_H_INCLUDED */

