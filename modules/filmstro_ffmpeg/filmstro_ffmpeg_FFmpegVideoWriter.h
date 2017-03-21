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


class FFmpegVideoWriter : public FFmpegVideoListener
{
public:

    FFmpegVideoWriter();
    ~FFmpegVideoWriter();

    /** Set the requested video codec before opening a file */
    void setVideoCodec (AVCodecID codec = AV_CODEC_ID_PROBE);
    /** Set the requested audio codec before opening a file */
    void setAudioCodec (AVCodecID codec = AV_CODEC_ID_PROBE);
    /** Set the requested subtitle codec before opening a file */
    void setSubtitleCodec (AVCodecID codec = AV_CODEC_ID_PROBE);

    /** Set the audio sample rate before opening a file */
    void setSampleRate (const int newSampleRate);

    /** Set the video size before opening a file */
    void setVideoSize (const int width, const int height);

    /** Set the pixel format before opening a file */
    void setPixelFormat (const AVPixelFormat format);

    /** Set the pixel aspect ratio as fraction before opening a file */
    void setPixelAspect (const int num, const int den);

    /** Set the timebase for the stream, provide either 
     AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_AUDIO, AVMEDIA_TYPE_SUBTITLE */
    void setTimeBase (AVMediaType type, AVRational timebase);

    /** copies settings from a context (e.g. FFmpegVideoReader) to the writer */
    void copySettingsFromContext (const AVCodecContext* context);

    /** Opens a file for writing audio, video and subtitles. The settings like
     encoders, samplerate etc. has to be set first. */
    bool openMovieFile (const juce::File& outputFile);

    /** Closes the movie file. Also flushes all left over samples and frames */
    void closeMovieFile ();

    /** Append a chunk of audio data. It will call writeAudioFrame to get rid of the data */
    void writeNextAudioBlock (juce::AudioSourceChannelInfo& info);

    /** Write the next video frame from juce image */
    void writeNextVideoFrame (const juce::Image& image, const juce::int64 timestamp);

    /** This callback receives frames from e.g. the FFmpegVideoReader to be written to the video file. 
     The timestamp has to be set in the frame. */
    void displayNewFrame (const AVFrame*) override;


    // ==============================================================================
private:

    void closeContexts ();

    void finishWriting ();

    /** Write audio data to frame, if there is enough. If flush is set to true, it will append silence to fill the last frame. */
    bool writeAudioFrame (const bool flush=false);

    int encodeWriteFrame (AVFrame *frame, AVMediaType type);

    // ==============================================================================

    /** This is the samplecode of the next sample to be written */
    juce::int64             audioWritePosition;

    AVFormatContext*        formatContext;

    AVCodecContext*         videoContext;
    AVCodecContext*         audioContext;
    AVCodecContext*         subtitleContext;

    AVCodecID               videoCodec;
    AVCodecID               audioCodec;
    AVCodecID               subtitleCodec;

    int                     videoStreamIdx;
    int                     audioStreamIdx;
    int                     subtitleStreamIdx;

    AVRational              videoTimeBase;
    AVRational              audioTimeBase;
    AVRational              subtitleTimeBase;

    int                     sampleRate;
    int64_t                 channelLayout;

    int                     videoWidth;
    int                     videoHeight;
    AVPixelFormat           pixelFormat;
    AVRational              pixelAspect;

    // buffer audio to match the video's audio frame size
    AudioBufferFIFO<float>  audioFifo;

    juce::ScopedPointer<FFmpegVideoScaler> videoScaler;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFmpegVideoWriter)
};

#endif /* FILMSTRO_FFMPEG_FFMPEGVIDEOWRITER_H_INCLUDED */

