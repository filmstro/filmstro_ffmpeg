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
  \file         filmstro_ffmpeg_FFmpegVideoWriter.cpp
  \brief        A component to write a video stream with ffmpeg

  \author       Daniel Walz @ filmstro.com
  \date         September 19th 2016

  \description  Use this writer to write audio frames and video frames in a 
                video file using ffmpeg
  ==============================================================================
 */


#include "../JuceLibraryCode/JuceHeader.h"


FFmpegVideoWriter::FFmpegVideoWriter()
 :  audioWritePosition (0),
    formatContext   (nullptr),
    videoContext    (nullptr),
    audioContext    (nullptr),
    subtitleContext (nullptr),
    videoCodec      (AV_CODEC_ID_NONE),
    audioCodec      (AV_CODEC_ID_NONE),
    subtitleCodec   (AV_CODEC_ID_NONE),
    videoStreamIdx  (-1),
    audioStreamIdx  (-1),
    subtitleStreamIdx (-1),
    sampleRate      (48000),
    channelLayout   (AV_CH_LAYOUT_STEREO),
    videoWidth      (0),
    videoHeight     (0),
    pixelFormat     (AV_PIX_FMT_NONE),
    pixelAspect     (av_make_q (1, 1)),
    audioFifo       (2, 8192)
{
    formatContext = nullptr;
    videoTimeBase = av_make_q (1, 24);
    audioTimeBase = av_make_q (1, sampleRate);
    subtitleTimeBase = AV_TIME_BASE_Q;
    av_register_all();
}

FFmpegVideoWriter::~FFmpegVideoWriter()
{
}

void FFmpegVideoWriter::setVideoCodec (const AVCodecID codec)
{
    videoCodec = codec;
}

void FFmpegVideoWriter::setAudioCodec (const AVCodecID codec)
{
    audioCodec = codec;
}

void FFmpegVideoWriter::setSubtitleCodec (const AVCodecID codec)
{
    subtitleCodec = codec;
}

void FFmpegVideoWriter::setSampleRate (const int newSampleRate)
{
    sampleRate = newSampleRate;
    audioTimeBase = av_make_q (1, newSampleRate);
}

void FFmpegVideoWriter::setVideoSize (const int width, const int height)
{
    videoWidth = width;
    videoHeight = height;
}

void FFmpegVideoWriter::setPixelFormat (const AVPixelFormat format)
{
    pixelFormat = format;
}

void FFmpegVideoWriter::setPixelAspect (const int num, const int den)
{
    pixelAspect = av_make_q (num, den);
}

void FFmpegVideoWriter::setTimeBase (AVMediaType type, AVRational timebase)
{
    switch (type) {
        case AVMEDIA_TYPE_VIDEO:
            videoTimeBase = timebase;
            break;
        case AVMEDIA_TYPE_AUDIO:
            audioTimeBase = timebase;
            break;
        case AVMEDIA_TYPE_SUBTITLE:
            subtitleTimeBase = timebase;
            break;
        default:
            break;
    }
}

void FFmpegVideoWriter::copySettingsFromContext (const AVCodecContext* context)
{
    if (context) {
        if (context->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoCodec  = context->codec_id;
            videoWidth  = context->width;
            videoHeight = context->height;
            pixelFormat = context->pix_fmt;
            pixelAspect = context->sample_aspect_ratio;
            if (context->framerate.num != 0) {
                // preferred way
                videoTimeBase = av_inv_q (context->framerate);
            }
            else if (context->time_base.num > 0) {
                // some decoders don't set framerate but might provide a time_base
                videoTimeBase = context->time_base;
            }
        }
        else if (context->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioCodec  = context->codec_id;
            channelLayout = context->channel_layout;
            // we don't copy audio, it's coming from JUCE so it's gonna be AV_SAMPLE_FMT_FLTP
            //sampleRate  = context->sample_rate;
        }
        else if (context->codec_type == AVMEDIA_TYPE_SUBTITLE) {
            // FIXME: TODO
        }
    }
}

bool FFmpegVideoWriter::openMovieFile (const juce::File& outputFile)
{
    if (formatContext) {
        avformat_free_context (formatContext);
    }

    videoStreamIdx  = -1;
    audioStreamIdx  = -1;
    subtitleStreamIdx = -1;

    audioWritePosition   = 0;

    /* allocate the output media context */
    avformat_alloc_output_context2 (&formatContext, NULL, NULL, outputFile.getFullPathName().toRawUTF8());
    if (!formatContext) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2 (&formatContext, NULL, "mpeg", outputFile.getFullPathName().toRawUTF8());
    }
    if (!formatContext)
        return false;

    if (videoCodec > AV_CODEC_ID_NONE) {
        AVStream* stream = avformat_new_stream (formatContext, NULL);
        if (!stream) {
            DBG ("Failed allocating video output stream\n");
        }
        else {
            videoStreamIdx = formatContext->nb_streams - 1;
            AVCodec* encoder = avcodec_find_encoder (videoCodec);
            if (encoder) {
                stream->time_base = videoTimeBase;
                videoContext = avcodec_alloc_context3 (encoder);
                videoContext->time_base = videoTimeBase;
                videoContext->pix_fmt   = pixelFormat;
                videoContext->sample_aspect_ratio = pixelAspect;
                videoContext->codec_type = encoder->type;
                videoContext->color_range = AVCOL_RANGE_JPEG;
                videoContext->codec_id  = videoCodec;
                videoContext->width     = videoWidth;
                videoContext->height    = videoHeight;
                videoContext->bit_rate  = 400000;
                videoContext->gop_size  = 10;
                videoContext->max_b_frames = 1;
                avcodec_parameters_from_context (stream->codecpar, videoContext);

                AVDictionary* options = nullptr;

                if (encoder->id == AV_CODEC_ID_H264) {
                    av_dict_set (&options, "preset", "slow", 0);
                    av_dict_set (&options, "tune", "film", 0);
                }

                int ret = avcodec_open2 (videoContext, encoder, &options);
                if (ret < 0) {
                    DBG ("Cannot open video encoder");
                    videoStreamIdx = -1;
                    videoCodec = AV_CODEC_ID_NONE;
                }
                av_dict_free (&options);
            }
            else {
                DBG ("Video encoder not found for " + String (avcodec_get_name (videoCodec)));
            }
        }
    }

    if (audioCodec > AV_CODEC_ID_NONE) {
        AVStream* stream = avformat_new_stream (formatContext, NULL);
        if (!stream) {
            DBG ("Failed allocating audio output stream\n");
        }
        else {
            audioStreamIdx = formatContext->nb_streams - 1;
            AVCodec* encoder = avcodec_find_encoder (audioCodec);
            if (encoder) {
                audioContext = avcodec_alloc_context3 (encoder);
                audioContext->time_base = audioTimeBase;
                audioContext->sample_rate = audioTimeBase.den;
                audioContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
                audioContext->channel_layout = channelLayout;
                audioContext->channels = av_get_channel_layout_nb_channels (channelLayout);
                audioContext->bit_rate = 64000;
                audioContext->frame_size = 1024;
                audioContext->bits_per_raw_sample = 32;
                avcodec_parameters_from_context (stream->codecpar, audioContext);

                int ret = avcodec_open2 (audioContext, encoder, NULL);
                if (ret < 0) {
                    char codecName [256];
                    av_get_codec_tag_string (codecName, 256, audioCodec);
                    DBG ("Cannot open audio encoder for format: " + String::fromUTF8 (codecName));
                    audioStreamIdx = -1;
                    audioCodec = AV_CODEC_ID_NONE;
                }
            }
            else {
                DBG ("Audio encoder not found for " + String (avcodec_get_name (audioCodec)));
            }
        }
    }

    //if (subtitleCodec > AV_CODEC_ID_NONE) {
    //    // FIXME: TODO
    //}

    av_dump_format (formatContext, 0, outputFile.getFullPathName().toRawUTF8(), 1);

    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open (&formatContext->pb, outputFile.getFullPathName().toRawUTF8(), AVIO_FLAG_WRITE) < 0) {
            DBG ("Could not open output file '" + outputFile.getFullPathName() + "'");
            closeContexts();
            return false;
        }
    }

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        if (videoContext) videoContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        if (audioContext) audioContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        if (subtitleContext) subtitleContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (!videoContext && !audioContext && !subtitleContext) {
        DBG ("No stream provided to encode");
        closeContexts();
        return false;
    }

    if (!(formatContext->oformat->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&formatContext->pb, outputFile.getFullPathName().toRawUTF8(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", outputFile.getFullPathName().toRawUTF8());
            closeContexts();
            return false;
        }
    }

    int ret = avformat_write_header (formatContext, NULL);
    if (ret < 0) {
        DBG ("Error occurred when opening output file");
        closeContexts();
        return false;
    }

    return true;
}

void FFmpegVideoWriter::closeMovieFile ()
{
    finishWriting ();
}

void FFmpegVideoWriter::finishWriting ()
{
    if (formatContext) {
        AVMediaType mediaType;
        for (int idx=0; idx < formatContext->nb_streams; ++idx) {
            if (idx == videoStreamIdx) {
                if (!(videoContext->codec->capabilities &
                      AV_CODEC_CAP_DELAY))
                    break;
                mediaType = AVMEDIA_TYPE_VIDEO;
            }
            else if (idx == audioStreamIdx) {
                if (!(audioContext->codec->capabilities &
                      AV_CODEC_CAP_DELAY))
                    break;
                mediaType = AVMEDIA_TYPE_AUDIO;
            }
            else {
                break;
            }

            av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", idx);
            while (encodeWriteFrame (NULL, mediaType));
        }

        av_write_trailer (formatContext);
    }
    closeContexts();
}

void FFmpegVideoWriter::closeContexts ()
{
    avformat_free_context (formatContext);
    videoStreamIdx  = -1;
    audioStreamIdx  = -1;
    subtitleStreamIdx = -1;
    videoContext = nullptr;
    audioContext = nullptr;
    subtitleContext = nullptr;
    avcodec_free_context (&videoContext);
    avcodec_free_context (&audioContext);
    avcodec_free_context (&subtitleContext);
    formatContext   = nullptr;
    videoScaler     = nullptr;
    audioWritePosition   = 0;
}

void FFmpegVideoWriter::writeNextAudioBlock (juce::AudioSourceChannelInfo& info)
{
    audioFifo.addToFifo (*info.buffer, info.numSamples);
    writeAudioFrame (false);
}

void FFmpegVideoWriter::writeNextVideoFrame (const juce::Image& image, const juce::int64 timestamp)
{
    // use scaler and add interface for image processing / e.g. branding
    if (videoContext) {
        if (! videoScaler) {
            videoScaler = new FFmpegVideoScaler ();
            videoScaler->setupScaler (image.getWidth(),
                                      image.getHeight(),
                                      AV_PIX_FMT_BGR0,
                                      videoContext->width,
                                      videoContext->height,
                                      videoContext->pix_fmt);
        }
        AVFrame* frame = av_frame_alloc ();
        frame->width = videoContext->width;
        frame->height = videoContext->height;
        frame->format = videoContext->pix_fmt;
        av_frame_set_color_range (frame, videoContext->color_range);
        int ret = av_image_alloc(frame->data, frame->linesize,
                                 videoContext->width,
                                 videoContext->height,
                                 videoContext->pix_fmt, 32);
        if (ret < 0) {
            DBG ("Could not allocate raw picture buffer");
            av_free (&frame);
            return;
        }
        videoScaler->convertImageToFrame (frame, image);
        frame->pts = timestamp;
        encodeWriteFrame (frame, AVMEDIA_TYPE_VIDEO);
    }
}

bool FFmpegVideoWriter::writeAudioFrame (const bool flush)
{
    if (formatContext && audioContext &&
        isPositiveAndBelow (audioStreamIdx, static_cast<int> (formatContext->nb_streams)))
    {

        int numFrameSamples  = audioContext->frame_size;

        if (audioFifo.getNumReady() >= numFrameSamples || flush) {
            const uint64_t channelLayout = AV_CH_LAYOUT_STEREO;
            const int numChannels = av_get_channel_layout_nb_channels (channelLayout);
            AVFrame* frame = av_frame_alloc ();
            frame->nb_samples   = numFrameSamples;
            frame->format       = AV_SAMPLE_FMT_FLTP;
            frame->channel_layout = channelLayout;
            frame->channels     = numChannels;
            frame->pts          = audioWritePosition;
            DBG ("Start writing audio frame, pts: " + String (audioWritePosition));
            int bufferSize = av_samples_get_buffer_size (nullptr, frame->channels, frame->nb_samples, AV_SAMPLE_FMT_FLTP, 0);
            float* samples = static_cast<float*> (av_malloc (bufferSize));
            avcodec_fill_audio_frame (frame,
                                      frame->channels,
                                      static_cast<enum AVSampleFormat> (frame->format),
                                      (const uint8_t*) (samples),
                                      bufferSize,
                                      0);

            float** sampleData = new float*[numChannels];
            for (int i=0; i < frame->channels; ++i) {
                sampleData[i] = samples + i * frame->nb_samples;
            }
            audioFifo.readFromFifo (sampleData, numFrameSamples);

            encodeWriteFrame (frame, AVMEDIA_TYPE_AUDIO);

            delete[] sampleData;
            //av_freep (buffer);

            audioWritePosition += numFrameSamples;
        }
        
        return true;
    }
    return false;
}

int FFmpegVideoWriter::encodeWriteFrame (AVFrame *frame, AVMediaType type) {
    int got_frame = 0;
    if (formatContext) {
        int ret;
        AVPacket packet;
        packet.data = NULL;
        packet.size = 0;
        av_init_packet (&packet);
        if (type == AVMEDIA_TYPE_VIDEO) {
            if (frame)
                av_frame_set_color_range (frame, AVCOL_RANGE_JPEG);
            ret = avcodec_encode_video2 (videoContext, &packet, frame, &got_frame);
            packet.stream_index = videoStreamIdx;
            av_packet_rescale_ts (&packet,
                                  videoContext->time_base,
                                  formatContext->streams [videoStreamIdx]->time_base);
            av_frame_free (&frame);
            if (ret < 0) {
                char error[255];
                av_strerror (ret, error, 255);
                DBG (String ("Error when encoding video data: ") += error);
                return false;
            }
        }
        else if (type == AVMEDIA_TYPE_AUDIO) {
            ret = avcodec_encode_audio2 (audioContext, &packet, frame, &got_frame);
            packet.stream_index = audioStreamIdx;
            av_packet_rescale_ts (&packet,
                                  audioContext->time_base,
                                  formatContext->streams [audioStreamIdx]->time_base);
            av_frame_free (&frame);
            if (ret < 0) {
                char error[255];
                av_strerror (ret, error, 255);
                DBG (String ("Error when encoding audio data: ") += error);
                return false;
            }
        }
        else {
            // the AVFrame only holds audio or video data, so you shouldn't call that
            // function with a type other than AVMEDIA_TYPE_VIDEO or AVMEDIA_TYPE_AUDIO
            jassertfalse;
        }

        if (got_frame == 1) {
            if (av_interleaved_write_frame (formatContext, &packet) < 0) {
                DBG ("Error when writing data");
                return false;
            }
        }
    }
    else {
        DBG ("No writer open, did not write frame");
        av_frame_free (&frame);
    }
    return got_frame == 1;
}

void FFmpegVideoWriter::displayNewFrame (const AVFrame* frame)
{
    // forward a copy to ffmpeg, the writer will dispose the frame...
    AVFrame* frameCopy = av_frame_alloc();
    av_frame_copy_props (frameCopy, frame);
    av_frame_copy (frameCopy, frame);
    frameCopy->width = frame->width;
    frameCopy->height = frame->height;
    frameCopy->format = frame->format;
    frameCopy->colorspace = frame->colorspace;
    frameCopy->sample_aspect_ratio = frame->sample_aspect_ratio;
    frameCopy->color_range = frame->color_range;
    frameCopy->pts = frame->pts;
    encodeWriteFrame (frameCopy, AVMEDIA_TYPE_VIDEO);
}


