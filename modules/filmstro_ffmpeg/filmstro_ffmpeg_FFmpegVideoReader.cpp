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
  \file         filmstro_ffmpeg_FFmpegVideoReader.cpp
  \brief        A class to read a video using FFmpeg

  \author       Daniel Walz @ filmstro.com
  \date         August 31st 2016

  \description  This class is used to read a video stream using the ffmpeg library.
                It acts as an AudioSource, so it's audio can be streamed into 
                JUCE processing chains. It also implements an FFmpegVideoSource,
                so that the frames can be displayed in a FFmpegVideoComponent.
  ==============================================================================

  Ripped from example:
  http://ffmpeg.org/doxygen/trunk/demuxing_decoding_8c-example.html

  ==============================================================================
 */


#include "../JuceLibraryCode/JuceHeader.h"

// enable this to print a DBG statement for each packet containing stream ID and timestamp
//#define DEBUG_LOG_PACKETS


FFmpegVideoReader::FFmpegVideoReader (const int audioFifoSize, const int videoFifoSize)
 :  looping                 (false),
    sampleRate              (0),
    resampleFactor          (1.0),
    currentTimeStamp        (0.0),
    nextReadPos             (0),
    audioFifo               (2, audioFifoSize),
    decoder                 (audioFifo, videoFifoSize)
{
}

FFmpegVideoReader::~FFmpegVideoReader()
{
    decoder.stopThread (500);
    closeMovieFile ();
    masterReference.clear();
}

// ==============================================================================
// video methods
// ==============================================================================

bool FFmpegVideoReader::loadMovieFile (const File& inputFile)
{
    if (inputFile.existsAsFile() == false) {
        videoFileName = File();
        decoder.closeMovieFile();
        return false;
    }

    if (decoder.loadMovieFile (inputFile)) {
        videoFileName = inputFile;
        return true;
    }
    return false;
}

void FFmpegVideoReader::closeMovieFile ()
{
    decoder.closeMovieFile();
    videoFileName = File();
}

juce::File FFmpegVideoReader::getVideoFileName () const
{
    return videoFileName;
}

double FFmpegVideoReader::getFramesPerSecond () const
{
    return decoder.getFramesPerSecond();
}

double FFmpegVideoReader::getCurrentTimeStamp() const
{
    if (sampleRate > 0)
        return static_cast<double> (nextReadPos) / sampleRate;
    return 0;
}

double FFmpegVideoReader::getVideoDuration () const
{
    return decoder.getDuration();
}

int FFmpegVideoReader::getVideoSamplingRate () const
{
    return decoder.getSampleRate();
}

int FFmpegVideoReader::getVideoChannels () const
{
    return decoder.getNumChannels();
}

// ==============================================================================
// from FFmpegVideoSource
// ==============================================================================

double FFmpegVideoReader::getLastVideoPTS () const
{
    return decoder.getCurrentPTS();
}

void FFmpegVideoReader::addVideoListener (FFmpegVideoListener* listener)
{
    decoder.addVideoListener (listener);
}

void FFmpegVideoReader::removeVideoListener (FFmpegVideoListener* listener)
{
    decoder.removeVideoListener (listener);
}

int FFmpegVideoReader::getVideoWidth () const
{
    return decoder.getVideoWidth();
}

int FFmpegVideoReader::getVideoHeight () const
{
    return decoder.getVideoHeight();
}

double FFmpegVideoReader::getVideoAspectRatio () const
{
    return static_cast<double> (decoder.getVideoWidth()) /
        static_cast<double> (decoder.getVideoHeight());
}

/** This will return the aspect ratio of each pixel */
double FFmpegVideoReader::getVideoPixelAspect () const
{
    return decoder.getPixelAspect();
}

enum AVSampleFormat FFmpegVideoReader::getSampleFormat () const
{
    return decoder.getSampleFormat();
}

enum AVPixelFormat FFmpegVideoReader::getPixelFormat () const
{
    return decoder.getPixelFormat();
}

AVRational FFmpegVideoReader::getVideoTimeBase () const
{
    return decoder.getVideoTimeBase();
}

juce::String FFmpegVideoReader::formatTimeCode (const double tc)
{
    MemoryBlock formatted;
    {
        MemoryOutputStream str (formatted, false);
        int tc_int = static_cast<int> (fabs(tc));
        if (tc < 0.0)
            str << "-";
        if (tc >= 3600)
            str << String (static_cast<int> (tc_int / 3600)) << ":";
        str << String (static_cast<int> ((tc_int % 3600) / 60)).paddedLeft ('0', 2) << ":";
        str << String (static_cast<int> (tc_int % 60)).paddedLeft ('0', 2) << ".";
        str << String (static_cast<int> ((static_cast<int> (fabs(tc * 100))) % 100)).paddedLeft ('0', 2);
    }
    return formatted.toString();
}


// ==============================================================================
// from AudioSource
// ==============================================================================

void FFmpegVideoReader::prepareToPlay (int samplesPerBlockExpected, double newSampleRate)
{
    const int numChannels = getVideoChannels();
    sampleRate = getVideoSamplingRate();

    audioFifo.setSize (numChannels, 192000);

    nextReadPos = 0;
}

void FFmpegVideoReader::releaseResources ()
{
    audioFifo.setSize (2, 8192);
}

void FFmpegVideoReader::getNextAudioBlock (const juce::AudioSourceChannelInfo &bufferToFill)
{
    double videoSampleRate = getVideoSamplingRate();
    currentTimeStamp += (bufferToFill.numSamples / videoSampleRate);

    // this triggers also reading of new video frame
    decoder.setCurrentPTS (static_cast<double>(nextReadPos) / sampleRate);
#ifdef DEBUG_LOG_PACKETS
    DBG ("Play audio block: " + String (nextReadPos) + " PTS: " + String (static_cast<double>(nextReadPos) / sampleRate));
#endif // DEBUG_LOG_PACKETS

    if (audioFifo.getNumReady() >= bufferToFill.numSamples) {
        audioFifo.readFromFifo (bufferToFill);
    }
    else {
        int numSamples = audioFifo.getNumReady();
        if (numSamples > 0) {
            audioFifo.readFromFifo (bufferToFill, numSamples);
            bufferToFill.buffer->clear (numSamples, bufferToFill.numSamples - numSamples);
        }
        else {
            bufferToFill.clearActiveBufferRegion();
        }
    }

    nextReadPos += bufferToFill.numSamples;
}

bool FFmpegVideoReader::waitForNextAudioBlockReady (const juce::AudioSourceChannelInfo &bufferToFill, const int msecs) const
{
    const juce::int64 timeout (Time::getCurrentTime().toMilliseconds() + msecs);
    while (audioFifo.getNumReady () < bufferToFill.numSamples && Time::getCurrentTime().toMilliseconds() < timeout) {
        Thread::sleep (5);
    }
    return false;
}

void FFmpegVideoReader::setNextReadPosition (juce::int64 newPosition)
{
    nextReadPos = newPosition;
    if (sampleRate > 0) {
        decoder.setCurrentPTS (nextReadPos / sampleRate, true);
    }
}

int64 FFmpegVideoReader::getNextReadPosition () const
{
//    juce::ScopedReadLock myLock (audioLock);
    return nextReadPos;
}

int64 FFmpegVideoReader::getTotalLength () const
{
    return static_cast<int64> (getVideoDuration() * sampleRate);
}

/** Returns true if this source is actually playing in a loop. */
bool FFmpegVideoReader::isLooping() const
{
    return looping;
}

/** Tells the source whether you'd like it to play in a loop. */
void FFmpegVideoReader::setLooping (bool shouldLoop)
{
    looping = shouldLoop;
}

// ==============================================================================
// FFmpeg low level
// ==============================================================================

AVCodecContext* FFmpegVideoReader::getVideoContext () const
{
    return decoder.getVideoContext();
}
AVCodecContext* FFmpegVideoReader::getAudioContext () const
{
    return decoder.getAudioContext();
}
AVCodecContext* FFmpegVideoReader::getSubtitleContext () const
{
    return decoder.getSubtitleContext();
}

// ==============================================================================
// video decoder thread
// ==============================================================================

FFmpegVideoReader::DecoderThread::DecoderThread (AudioBufferFIFO<float>& fifo, const int videoFifoSize)
  : juce::Thread        ("FFmpeg decoder"),
    audioFifo           (fifo),
    videoFifoRead       (0),
    videoFifoWrite      (0),
    formatContext       (nullptr),
    videoContext        (nullptr),
    audioContext        (nullptr),
    subtitleContext     (nullptr),
    videoStreamIdx      (-1),
    audioStreamIdx      (-1),
    subtitleStreamIdx   (-1),
    currentPTS          (0)
{
    av_register_all();

    // initialize frame FIFO, set data to NULL
    videoFrames.resize (videoFifoSize, std::make_pair (0.0, nullptr));
    for (int i=0; i<videoFrames.size(); ++i) {
        videoFrames [i].second = av_frame_alloc();
    }

    audioFrame = av_frame_alloc();

}

FFmpegVideoReader::DecoderThread::~DecoderThread ()
{
    stopThread (100); // just in case

    for (int i=0; i<videoFrames.size(); ++i) {
        av_frame_free (&(videoFrames [i].second));
    }

    av_frame_free (&audioFrame);

}


bool FFmpegVideoReader::DecoderThread::loadMovieFile (const juce::File& inputFile)
{
    if (formatContext) {
        closeMovieFile ();
    }

    // open input file, and allocate format context
    int ret = avformat_open_input (&formatContext, inputFile.getFullPathName().toRawUTF8(), NULL, NULL);
    if (ret < 0) {
        DBG ("Opening file failed");
        return false;
    }

    // retrieve stream information
    if (avformat_find_stream_info (formatContext, NULL) < 0) {
        return false;
    }

    // open the streams
    audioStreamIdx = openCodecContext (&audioContext, AVMEDIA_TYPE_AUDIO, true);
    if (isPositiveAndBelow (audioStreamIdx, static_cast<int> (formatContext->nb_streams))) {
        audioContext->request_sample_fmt = AV_SAMPLE_FMT_FLTP;
        //audioContext->request_channel_layout = AV_CH_LAYOUT_STEREO_DOWNMIX;
    }

    videoStreamIdx = openCodecContext (&videoContext, AVMEDIA_TYPE_VIDEO, true);
    if (isPositiveAndBelow (videoStreamIdx, static_cast<int> (formatContext->nb_streams))) {
        videoListeners.call (&FFmpegVideoListener::videoSizeChanged, videoContext->width,
                                                                     videoContext->height,
                                                                     videoContext->pix_fmt);
    }

    av_dump_format (formatContext, 0, inputFile.getFullPathName().toRawUTF8(), 0);

    videoListeners.call (&FFmpegVideoListener::videoFileChanged, inputFile);

    startThread ();

    return true;
}

void FFmpegVideoReader::DecoderThread::closeMovieFile ()
{
    stopThread(1000);

    if (videoStreamIdx >= 0) {
        avcodec_free_context (&videoContext);
        videoStreamIdx = -1;
    }
    if (audioStreamIdx >= 0) {
        avcodec_free_context (&audioContext);
        audioStreamIdx = -1;
    }
    if (subtitleStreamIdx >= 0) {
        avcodec_free_context (&subtitleContext);
        subtitleStreamIdx = -1;
    }
    avformat_close_input (&formatContext);
}

void FFmpegVideoReader::DecoderThread::addVideoListener (FFmpegVideoListener* listener)
{
    videoListeners.add (listener);
}

void FFmpegVideoReader::DecoderThread::removeVideoListener (FFmpegVideoListener* listener)
{
    videoListeners.remove (listener);
}

int FFmpegVideoReader::DecoderThread::openCodecContext (AVCodecContext** decoderContext,
                                                        enum AVMediaType type,
                                                        bool refCounted)
{
    AVCodec *decoder = NULL;
    AVDictionary *opts = NULL;

    int id = av_find_best_stream (formatContext, type, -1, -1, NULL, 0);

    if (isPositiveAndBelow(id, static_cast<int> (formatContext->nb_streams))) {
        AVStream* stream = formatContext->streams [id];
        // find decoder for the stream
        decoder = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!decoder) {
            DBG ("Failed to find " + String (av_get_media_type_string(type)) + " codec");
            return -1;
        }
        // Allocate a codec context for the decoder
        *decoderContext = avcodec_alloc_context3 (decoder);
        if (!*decoderContext) {
            DBG ("Failed to allocate the " + String (av_get_media_type_string(type)) +
                 " codec context");
            return -1;
        }
        // Copy codec parameters from input stream to output codec context
        if (avcodec_parameters_to_context (*decoderContext, stream->codecpar) < 0) {
            DBG ("Failed to copy " + String (av_get_media_type_string(type)) +
                 " codec parameters to decoder context");
            return -1;
        }
        // Init the decoders, with or without reference counting
        av_dict_set (&opts, "refcounted_frames", refCounted ? "1" : "0", 0);
        if (avcodec_open2 (*decoderContext, decoder, &opts) < 0) {
            DBG ("Failed to open " + String (av_get_media_type_string(type)) + " codec");
            avcodec_free_context (decoderContext);
            return -1;
        }

        return id;
    }
    else {
        DBG ("Could not find " + String (av_get_media_type_string(type)) +
             " stream in input file");
        return -1;
    }
}

int FFmpegVideoReader::DecoderThread::decodeAudioPacket (AVPacket packet)
{
    int got_frame = 0;
    int decoded   = packet.size;
    int outputNumSamples    = 0;

    // decode audio frame
    do {
        // call decode until packet is empty
        int ret = avcodec_decode_audio4 (audioContext, audioFrame, &got_frame, &packet);
        if (ret < 0) {
            DBG ("Error decoding audio frame: (Code " + String (ret) + ")");
            break;
        }

        int64_t framePTS = av_frame_get_best_effort_timestamp (audioFrame);
        double  framePTSsecs = static_cast<double> (framePTS) / audioContext->sample_rate;

#ifdef DEBUG_LOG_PACKETS
        DBG ("Stream " + String (packet.stream_index) +
             " (Audio) " +
             " DTS: " + String (packet.dts) +
             " PTS: " + String (packet.pts) +
             " Frame PTS: " + String (framePTS));
#endif /* DEBUG_LOG_PACKETS */

        /* Some audio decoders decode only part of the packet, and have to be
         * called again with the remainder of the packet data.
         * Sample: fate-suite/lossless-audio/luckynight-partial.shn
         * Also, some decoders might over-read the packet. */
        decoded = jmin (ret, packet.size);

        packet.data += decoded;
        packet.size -= decoded;

        if (got_frame && decoded > 0 && audioFrame->extended_data != nullptr) {
            const int channels   = av_get_channel_layout_nb_channels (audioFrame->channel_layout);
            const int numSamples = audioFrame->nb_samples;

            int offset = (currentPTS - framePTSsecs) * audioContext->sample_rate;
            if (offset > 100) {
                if (offset < numSamples) {
                    outputNumSamples = numSamples-offset;
                    AudioBuffer<float> subset ((float* const*)audioFrame->extended_data, channels, offset, outputNumSamples);
                    audioFifo.addToFifo (subset);
                }
            }
            else {
                outputNumSamples = numSamples;
                audioFifo.addToFifo ((const float **)audioFrame->extended_data, numSamples);
            }
        }

    } while (got_frame && packet.size > 0);
    
    return outputNumSamples;
}

double FFmpegVideoReader::DecoderThread::decodeVideoPacket (AVPacket packet)
{
    double pts_sec = 0.0;
    if (videoContext && packet.size > 0) {
        int got_picture = 0;
        AVFrame* frame = videoFrames [videoFifoWrite].second;

        if (avcodec_decode_video2 (videoContext, frame, &got_picture, &packet) > 0) {
            int64_t pts = av_frame_get_best_effort_timestamp (frame);

            AVRational timeBase = av_make_q (1, AV_TIME_BASE);
            if (isPositiveAndBelow(videoStreamIdx, static_cast<int> (formatContext->nb_streams))) {
                timeBase = formatContext->streams [videoStreamIdx]->time_base;
            }
            pts_sec = av_q2d (timeBase) * pts;
            if (pts_sec >= 0.0) {
                videoFrames [videoFifoWrite].first = pts_sec;
                videoFifoWrite = ++videoFifoWrite % videoFrames.size();
            }

#ifdef DEBUG_LOG_PACKETS
            DBG ("Stream " + String (packet.stream_index) +
                 " (Video) " +
                 " DTS: " + String (packet.dts) +
                 " PTS: " + String (packet.pts) +
                 " best effort PTS: " + String (pts) +
                 " in ms: " + String (pts_sec) +
                 " timebase: " + String (av_q2d(timeBase)));
#endif /* DEBUG_LOG_PACKETS */

        }
    }
    return pts_sec;
}

void FFmpegVideoReader::DecoderThread::run()
{
    int error = 0;
    while (!threadShouldExit()) {
        int freeVideoFrames = (videoFrames.size() + videoFifoWrite - videoFifoRead) % videoFrames.size();
        if (audioFifo.getFreeSpace() > 2048 && (audioFifo.getNumReady() < 4096 || freeVideoFrames < videoFrames.size() - 2)) {

#ifdef DEBUG_LOG_PACKETS
            DBG ("Audio: " + String (audioFifo.getNumReady()) + " ready, "
                 + String (audioFifo.getFreeSpace()) + " free");
#endif /* DEBUG_LOG_PACKETS */

            AVPacket packet;
            // initialize packet, set data to NULL, let the demuxer fill it
            packet.data = NULL;
            packet.size = 0;
            av_init_packet (&packet);

            error = av_read_frame (formatContext, &packet);

            if (error >= 0) {
                if (packet.stream_index == audioStreamIdx) {
                    decodeAudioPacket (packet);
                }
                else if(packet.stream_index == videoStreamIdx) {
                    decodeVideoPacket (packet);
                }
                else {
                    //DBG ("Packet is neither audio nor video... stream: " + String (packet.stream_index));
                }
            }
            av_packet_unref (&packet);
        }
        else {
            waitForPacket.wait (20);
        }
        if (error < 0) {
            // probably EOF, but keep checking from time to time
            waitForPacket.wait (500);
        }
    }
}

void FFmpegVideoReader::DecoderThread::setCurrentPTS (const double pts, bool seek)
{
    if (audioContext && seek) {
        int64_t readPos = pts * audioContext->sample_rate;
        av_seek_frame (formatContext, audioStreamIdx, readPos, 0);
        // invalidate all FIFOs
        audioFifo.reset();
        videoFifoWrite = 0;
        videoFifoRead = 0;
        for (int i=0; i < videoFrames.size(); ++i) {
            videoFrames [i].first = 0.0;
        }
        waitForPacket.signal();
        Thread::wait (20);
    }

    videoListeners.call (&FFmpegVideoListener::presentationTimestampChanged, pts);

    // find highest PTS < currentPTS
    int availableFrames = (videoFrames.size() + videoFifoWrite - videoFifoRead) % videoFrames.size();
    if (availableFrames < 1) {
        // No frame read!
        DBG ("No frame available!");
        currentPTS = pts;
        return;
    }

    int read = videoFifoRead;
    int i=0;

    while ((videoFrames [(read + i) % videoFrames.size()].first < pts) && i < (availableFrames - 1)) {
        ++i;
    }

    currentPTS = pts;
    if (i > 0 || seek) {
        if (i > 1) {
            DBG ("Dropped " + String (i-1) + " frame(s)");
        }
        videoFifoRead = (read + i) % videoFrames.size();
        AVFrame* nextFrame = videoFrames [videoFifoRead].second;

        videoListeners.call (&FFmpegVideoListener::displayNewFrame, nextFrame);
    }
}

double FFmpegVideoReader::DecoderThread::getCurrentPTS () const
{
    return currentPTS;
}

int FFmpegVideoReader::DecoderThread::getVideoWidth () const
{
    if (videoContext) {
        return videoContext->width;
    }
    return 0;
}

int FFmpegVideoReader::DecoderThread::getVideoHeight () const
{
    if (videoContext) {
        return videoContext->height;
    }
    return 0;
}

enum AVSampleFormat FFmpegVideoReader::DecoderThread::getSampleFormat () const
{
    if (audioContext)
        return audioContext->sample_fmt;
    return AV_SAMPLE_FMT_NONE;
}

enum AVPixelFormat FFmpegVideoReader::DecoderThread::getPixelFormat () const
{
    if (videoContext)
        return videoContext->pix_fmt;
    return AV_PIX_FMT_NONE;
}

double FFmpegVideoReader::DecoderThread::getPixelAspect () const
{
    if (videoContext && videoContext->sample_aspect_ratio.num > 0)
        return av_q2d (videoContext->sample_aspect_ratio);
    return 1.0;
}

AVRational FFmpegVideoReader::DecoderThread::getVideoTimeBase () const
{
    if (formatContext) {
        if (isPositiveAndBelow (videoStreamIdx, static_cast<int> (formatContext->nb_streams))) {
            return formatContext->streams [videoStreamIdx]->time_base;
        }
    }
    return av_make_q (0, 1);
}

double FFmpegVideoReader::DecoderThread::getFramesPerSecond () const
{
    if (videoContext && videoContext->framerate.num > 0) {
        return av_q2d (videoContext->framerate);
    }
    if (formatContext && isPositiveAndBelow (videoStreamIdx, static_cast<int> (formatContext->nb_streams))) {
        AVRational tb = formatContext->streams [videoStreamIdx]->time_base;
        if (tb.num > 0) {
            return av_q2d (av_inv_q (tb));
        }
    }
    return 0;
}

double FFmpegVideoReader::DecoderThread::getSampleRate () const
{
    if (audioContext) {
        return audioContext->sample_rate;
    }
    return 0;
}

double FFmpegVideoReader::DecoderThread::getDuration () const
{
    if (formatContext) {
        return formatContext->duration / AV_TIME_BASE;
    }
    return 0;
}

int FFmpegVideoReader::DecoderThread::getNumChannels () const
{
    if (audioContext) {
        return audioContext->channels;
    }
    return 0;
}

AVCodecContext* FFmpegVideoReader::DecoderThread::getVideoContext () const
{
    return videoContext;
}
AVCodecContext* FFmpegVideoReader::DecoderThread::getAudioContext () const
{
    return audioContext;
}
AVCodecContext* FFmpegVideoReader::DecoderThread::getSubtitleContext () const
{
    return subtitleContext;
}

