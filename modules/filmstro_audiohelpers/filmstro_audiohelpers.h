/*
 ==============================================================================
 Copyright (c) 2017 Filmstro Ltd. - Daniel Walz
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

 ID:            filmstro_audiohelpers
 vendor:        Filmstro Ltd.
 version:       0.9.0
 name:          Basic audio helper classes
 description:   Audio helper classes of general purpose
 dependencies:  juce_audio_basics, juce_audio_formats, juce_audio_devices, juce_audio_processors
 website:       http://www.filmstro.com/
 license:       Proprietary

 END_JUCE_MODULE_DECLARATION

 ==============================================================================
 */


#ifndef FILMSTRO_AUDIOHELPERS_INCLUDED_H
#define FILMSTRO_AUDIOHELPERS_INCLUDED_H


#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "filmstro_audiohelpers/filmstro_audiohelpers_AudioBufferFIFO.h"
#include "filmstro_audiohelpers/filmstro_audiohelpers_AudioProcessorPlayerSource.h"
#include "filmstro_audiohelpers/filmstro_audiohelpers_OutputSourcePlayer.h"
#include "filmstro_audiohelpers/filmstro_audiohelpers_SharedFormatManager.h"

#endif /* FILMSTRO_AUDIOHELPERS_INCLUDED_H */

