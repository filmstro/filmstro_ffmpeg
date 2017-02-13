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
 :  videoStreamIdx (-1),
    audioStreamIdx (-1),
    subtitleStreamIdx (-1),
    audioFifo (2, 8192)
{
    formatContext = nullptr;

    av_register_all();
}

FFmpegVideoWriter::~FFmpegVideoWriter()
{
}

bool FFmpegVideoWriter::openMovieFile (const juce::File& outputFile,
                                       double sampleRate)
{
    if (formatContext) {
        avformat_free_context (formatContext);
    }

    videoStreamIdx = -1;
    audioStreamIdx = -1;
    subtitleStreamIdx = -1;

    /* allocate the output media context */
    avformat_alloc_output_context2 (&formatContext, NULL, NULL, outputFile.getFullPathName().toRawUTF8());
    if (!formatContext) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2 (&formatContext, NULL, "mpeg", outputFile.getFullPathName().toRawUTF8());
    }
    if (!formatContext)
        return false;
/*
    // copy streams
    for (i = 0; i < inputFormatContext->nb_streams; i++) {
        out_stream = avformat_new_stream (formatContext, NULL);
        if (!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }
        in_stream = inputFormatContext->streams[i];
        dec_ctx = in_stream->codec;
        enc_ctx = out_stream->codec;
        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
            || dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            // in this example, we choose transcoding to same codec
            //encoder = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
            encoder = avcodec_find_encoder(dec_ctx->codec_id);
            if (!encoder) {
                av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
                return AVERROR_INVALIDDATA;
            }
            // In this example, we transcode to same properties (picture size,
            // sample rate etc.). These properties can be changed for output
            // streams easily using filters
            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                *videoStreamIdx = i;
                enc_ctx->height = dec_ctx->height;
                enc_ctx->width = dec_ctx->width;
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;

                enc_ctx->pix_fmt = encoder->pix_fmts[0];
                int f = 0;
                enum AVPixelFormat nextFormat = encoder->pix_fmts[f];
                while (nextFormat >= 0) {
                    if (nextFormat == dec_ctx->pix_fmt) {
                        enc_ctx->pix_fmt = nextFormat;
                        break;
                    }
                    nextFormat = encoder->pix_fmts[++f];
                }

                // video time_base can be set to whatever is handy and supported by encoder
                enc_ctx->time_base = dec_ctx->time_base;

                if (encoder->id == AV_CODEC_ID_H264) {
                    av_opt_set (enc_ctx->priv_data, "preset", "medium", 0);
                    av_opt_set (enc_ctx->priv_data, "tune",   "film", 0);
                }

                if (avcodec_open2(enc_ctx, encoder, NULL) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
                    *videoStreamIdx = -1;
                }
            } else {
                *audioStreamIdx = i;
                //enc_ctx->sample_rate = dec_ctx->sample_rate;
                //enc_ctx->channel_layout = dec_ctx->channel_layout;
                enc_ctx->sample_rate = sampleRate;
                enc_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
                enc_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;

                enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
                // take first format from list of supported formats
                //enc_ctx->sample_fmt = encoder->sample_fmts[0];
                enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};

                if (avcodec_open2(enc_ctx, encoder, NULL) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Cannot open audio encoder for stream #%u\n", i);
                    *audioStreamIdx = -1;
                }
            }

        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
            return AVERROR_INVALIDDATA;
        } else {
            // if this stream must be remuxed
            ret = avcodec_copy_context(outputFormatContext->streams[i]->codec,
                                       inputFormatContext->streams[i]->codec);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Copying stream context failed\n");
                return ret;
            }
        }
        if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
            enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    av_dump_format (outputFormatContext, 0, filename, 1);
    if (!(outputFormatContext->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open (&outputFormatContext->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log (NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
            return ret;
        }
    }
    // init muxer, write output file header
    ret = avformat_write_header (outputFormatContext, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return ret;
    }
    */
    return true;
}

void FFmpegVideoWriter::setVideoReader (FFmpegVideoReader* source)
{
    videoSource = source;
}

void FFmpegVideoWriter::finishWriting ()
{
}

bool FFmpegVideoWriter::writeAudioFrame (const juce::AudioSampleBuffer& bufferToWrite, int startSample, int numSamples)
{
    if (!formatContext || audioStreamIdx < 0) return false;

    audioFifo.addToFifo (bufferToWrite, numSamples);
    int numFrameSamples  = 1024;
    int numFrameChannels = 2;

//    ffMpegGetAudioParameters (outputFormatContext, audioStreamIdx, &numFrameSamples, &numFrameChannels);

    if (audioFifo.getNumReady() >= numFrameSamples) {
        float** channelData;

        // allocate audio frame
//        AVFrame* frame = ffMpegPrepareAudioFrame (outputFormatContext->streams [audioStreamIdx],
//                                                  &channelData);

        audioFifo.readFromFifo (channelData, numFrameSamples);

        // write audio frame
//        ffMpegWriteAudioFrame (outputFormatContext, audioStreamIdx, &frame, &channelData);
    }

    if (videoSource) {
        // write video frame

    }
    return true;
}

bool FFmpegVideoWriter::writeAudioFrame (const juce::AudioSampleBuffer& bufferToWrite)
{
    return writeAudioFrame (bufferToWrite, 0, bufferToWrite.getNumSamples());
}

bool FFmpegVideoWriter::writeVideoFrame (AVFrame* frame)
{
    // TODO
    return true;
}


