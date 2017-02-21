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
  \class        FFmpegVideoReader
  \file         filmstro_ffmpeg_FFmpegVideoReader.h
  \brief        A component to view a video decoded by FFmpeg
 
  \author       Daniel Walz @ filmstro.com
  \date         August 31st 2016

  \description  This class will read a video file using ffmpeg

  ==============================================================================
 */


#ifndef FILMSTRO_FFMPEG_FFMPEGVIDEOREADER_H_INCLUDED
#define FILMSTRO_FFMPEG_FFMPEGVIDEOREADER_H_INCLUDED

#include <atomic>

/**
 \class         FFmpegVideoReader
 \description   Reads a ffmpeg video file
 
 The FFmpegVideoReader opens a ffmpeg stream for reading. It acts as regular 
 juce::AudioSource and can be played like that. Additionally it provides a 
 FFmpegVideoSource which can be used to display the video frames or to write
 the file back using a FFmpegVideoWriter.
 */
class FFmpegVideoReader :   public juce::PositionableAudioSource
{
public:

    /** Constructs a FFmpegVideoReader. Because usually audio and video frames may
     be in arbitrary order, the reader provides a FIFO for audio samples and a FIFO 
     for video frames. */
    FFmpegVideoReader (const int audioFifoSize=192000, const int videoFifoSize=20);
    virtual ~FFmpegVideoReader();

    // ==============================================================================
    // video decoder thread
    // ==============================================================================
    /**
     \class         FFmpegVideoReader::DecoderThread
     \description   class for FFmpegReader to decode audio and images asynchronously
                    This is to keep the audio thread as fast as possible
     */
    class DecoderThread : public juce::Thread
    {
    public:
        DecoderThread (AudioBufferFIFO<float>& fifo, const int videoFifoSize);

        virtual ~DecoderThread ();

        bool loadMovieFile (const juce::File& inputFile);

        void closeMovieFile ();

        void addVideoListener (FFmpegVideoListener* listener);

        void removeVideoListener (FFmpegVideoListener* listener);

        /** working loop */
        void run() override;

        /** set the currently played PTS according to the audio stream */
        void setCurrentPTS (const double pts, bool seek = false);

        /** returns the presentation timestamp the video is synchronised to */
        double getCurrentPTS () const;

        /** get the width of the video images according to decoder */
        int getVideoWidth () const;

        /** get the height of the video images according to decoder */
        int getVideoHeight () const;

        /** get the pixel format of the video images according to decoder,
         will be converted to BGR0 to be displayed as juce::Image */
        enum AVPixelFormat getPixelFormat () const;

        /** This will return the aspect ratio of each pixel */
        double getPixelAspect () const;

        enum AVSampleFormat getSampleFormat () const;

        /** Return the framerate. If framerate in the decoder context is not set,
         this will return the timebase of the video stream. */
        double  getFramesPerSecond () const;


        /** Give access to the context to set up writers */
        AVFormatContext* getVideoReaderContext();

        double getSampleRate () const;

        double getDuration () const;

        int getNumChannels () const;

        AVCodecContext* getVideoContext () const;
        AVCodecContext* getAudioContext () const;
        AVCodecContext* getSubtitleContext () const;

    private:

        int openCodecContext (AVCodecContext** decoderContext,
                              enum AVMediaType type,
                              bool refCounted);

        /** Returns the number of added samples to the audio FIFO */
        int decodeAudioPacket (AVPacket packet);

        /** Returns the presentation timecode PTS of the decoded frame */
        double decodeVideoPacket (AVPacket packet);


        // ==============================================================================


        /** has access to the audio sources fifo to fill it */
        AudioBufferFIFO<float>& audioFifo;
        
        juce::WaitableEvent waitForPacket;

        /** vector of PTS -> AVFrame tuples */
        std::vector<std::pair<double, AVFrame*> > videoFrames;
        std::atomic<int>    videoFifoRead;
        std::atomic<int>    videoFifoWrite;

        AVFormatContext*    formatContext;
        AVCodecContext*     videoContext;
        AVCodecContext*     audioContext;
        AVCodecContext*     subtitleContext;

        int                 videoStreamIdx;
        int                 audioStreamIdx;
        int                 subtitleStreamIdx;

        AVFrame*            audioFrame;

        std::atomic<double> currentPTS;

        juce::ListenerList<FFmpegVideoListener> videoListeners;

        /** Buffer for reading */
        juce::AudioBuffer<float> buffer;

    };

    // ==============================================================================
    // video methods
    // ==============================================================================

    bool    loadMovieFile (const juce::File& inputFile);

    void    closeMovieFile ();

    /** Return the framerate. If framerate in the decoder context is not set,
     this will return the timebase of the video stream. */
    double  getFramesPerSecond () const;

    /** Get current presentation timecode according to audio stream */
    double  getCurrentTimeStamp() const;

    /** returns the duration of the video in seconds according to the container context */
    double  getVideoDuration () const;

    /** returns the sampling rate as specified in the video file. This can be different 
     from the samplingrate the prepareToPlay was called with. 
     The FFmpegVideoReader will not resample. */
    int     getVideoSamplingRate () const;

    /** returns the number of audio channels in the video file. Make sure you call 
     getNextAudioBuffer with the same number of channels */
    int     getVideoChannels () const;

    /** returns the audio sample format in the video file. 
     The FFmpegVideoReader will convert into discrete channels of float values */
    enum AVSampleFormat getSampleFormat () const;

    /** add a listener to receive video frames for displaying and to get timestamp
     notifications. The video frames will happen synchronously to getNextAudioBlock */
    void addVideoListener (FFmpegVideoListener* listener);

    /** remove a video listener */
    void removeVideoListener (FFmpegVideoListener* listener);

    double getLastVideoPTS () const;

    /** get the width of the video images according to decoder */
    int getVideoWidth () const;

    /** get the height of the video images according to decoder */
    int getVideoHeight () const;

    /** returns the aspect ratio video frame */
    double getVideoAspectRatio () const;

    /** This will return the aspect ratio of each pixel */
    double getVideoPixelAspect () const;

    /** get the pixel format of the video images according to decoder,
        will be converted to BGR0 to be displayed as juce::Image */
    enum AVPixelFormat getPixelFormat () const;



    // ==============================================================================
    // from PositionableAudioSource
    // ==============================================================================

    void 	prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    void 	releaseResources () override;

    /** decodes packets to fill the audioFifo. If a video packet is found it will be forwarded to VideoDecoderThread */
    void 	getNextAudioBlock (const juce::AudioSourceChannelInfo &bufferToFill) override;

    /** Wait until the decoder thread has finished enough data. This is needed for non-realtime processing. */
    bool    waitForNextAudioBlockReady (const juce::AudioSourceChannelInfo &bufferToFill, const int msecs) const;

    /** Seeks in the stream */
    void 	setNextReadPosition (juce::int64 newPosition) override;

    /** Returns the sample count of the next sample to be returned by getNextAudioBlock */
    juce::int64 	getNextReadPosition () const override;

    /** Returns the total length in samples. May not be accurate, because it uses ffmpegs 
        duration property multiplied with the samplerate */
    juce::int64 	getTotalLength () const override;

    /** Returns true if this source is actually playing in a loop. */
    bool isLooping() const override;

    /** Tells the source whether you'd like it to play in a loop. */
    void setLooping (bool shouldLoop) override;

    // ==============================================================================
    // FFmpeg low level
    // ==============================================================================

    AVCodecContext* getVideoContext () const;
    AVCodecContext* getAudioContext () const;
    AVCodecContext* getSubtitleContext () const;


    // ==============================================================================
private:

    juce::File  videoFileName;

    bool        fileReadyToRead;

    bool        looping;

    int         sampleRate;

    double      framesPerSec;

    double      resampleFactor;

    double      currentTimeStamp;

    juce::int64                         nextReadPos;

    AudioBufferFIFO<float>              audioFifo;

    DecoderThread                       decoder;

    // use WeakReference so that removing a source doesn't lead to disaster
    juce::WeakReference<FFmpegVideoReader>::Master masterReference;
    friend class juce::WeakReference<FFmpegVideoReader>;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FFmpegVideoReader)
};

#endif /* FILMSTRO_FFMPEG_FFMPEGVIDEOREADER_H_INCLUDED */

