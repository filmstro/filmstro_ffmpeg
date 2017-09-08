/*
  ==============================================================================

  Copyright (c) 2017, Daniel Walz
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

  BEGIN_JUCE_MODULE_DECLARATION
 
  ID:            filmstro_ffmpeg
  vendor:        Filmstro Ltd.
  version:       0.9.0
  name:          FFmpeg wrapping classes for use in JUCE
  description:   Provides classes to read audio streams from video files or to
                 mux audio into an existing video
  dependencies:  juce_audio_basics, juce_audio_formats, juce_core, filmstro_audiohelpers
  website:       http://www.filmstro.com/
  license:       BSD V2 3-clause

  END_JUCE_MODULE_DECLARATION
 
  ==============================================================================
 */


#ifndef FSPRO_FFMPEG_INCLUDED_H
#define FSPRO_FFMPEG_INCLUDED_H


#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "filmstro_audiohelpers/filmstro_audiohelpers_AudioBufferFIFO.h"

#ifndef FILMSTRO_USE_FFMPEG
#define FILMSTRO_USE_FFMPEG 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
    
#ifdef __cplusplus
}
#endif

#include "filmstro_ffmpeg_FFmpegVideoListener.h"
#include "filmstro_ffmpeg_FFmpegVideoScaler.h"
#include "filmstro_ffmpeg_FFmpegAudioResampler.h"
#include "filmstro_ffmpeg_FFmpegVideoReader.h"
#include "filmstro_ffmpeg_FFmpegVideoWriter.h"
#include "filmstro_ffmpeg_FFmpegVideoComponent.h"


#endif /* FSPRO_FFMPEG_INCLUDED_H */
