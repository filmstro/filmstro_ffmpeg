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

    Demo to play back a movie using ffmpeg and JUCE

 ==============================================================================
 */

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "OSDComponent.h"

//==============================================================================
/*
 This component lives inside our window, and this is where you should put all
 your controls and content.
 */
class VideoComponentWithDropper :   public FFmpegVideoComponent,
                                    public FileDragAndDropTarget
{
public:
    VideoComponentWithDropper (FFmpegVideoReader* readerToOpenFiles)
    {
        setVideoReader (readerToOpenFiles);
    }

    virtual ~VideoComponentWithDropper () {}

    bool isInterestedInFileDrag (const StringArray &files) override
    {
        return true;
    }

    void filesDropped (const StringArray &files, int x, int y) override
    {
        if (FFmpegVideoReader* reader = getVideoReader()) {
            File fileToOpen (files [0]);
            reader->loadMovieFile (fileToOpen);
        }
    }
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   :  public AudioAppComponent,
                                public FFmpegVideoListener
{
public:
    //==============================================================================
    MainContentComponent()
    {
        videoAspectRatio = 1.77;

        videoReader = new FFmpegVideoReader (384000, 30);
        videoReader->addVideoListener (this);

        transportSource = new AudioTransportSource ();
        transportSource->setSource (videoReader, 0, nullptr);

        videoComponent = new VideoComponentWithDropper (videoReader);
        addAndMakeVisible (videoComponent);

        osdComponent = new OSDComponent (videoReader, transportSource);
        addAndMakeVisible (osdComponent);

        setSize (800, 600);

        // specify the number of input and output channels that we want to open
        setAudioChannels (0, 6);

#ifdef DEBUG
        if (AudioIODevice* device = deviceManager.getCurrentAudioDevice()) {
            DBG ("Current Samplerate: " + String (device->getCurrentSampleRate()));
            DBG ("Current Buffersize: " + String (device->getCurrentBufferSizeSamples()));
            DBG ("Current Bitdepth:   " + String (device->getCurrentBitDepth()));
        }
#endif /* DEBUG */
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.
        videoReader->prepareToPlay (samplesPerBlockExpected, sampleRate);
        transportSource->prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // the AudioTransportSource takes care of start, stop and resample
        transportSource->getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource->releaseResources ();
        videoReader->releaseResources ();
    }

    //==============================================================================
    /** Reset gui when a new file is loaded */
    void videoFileChanged (const juce::File& video) override
    {
        String abbrev (video.getFullPathName());
        if (abbrev.length() > 30)
            abbrev = "(...)" + abbrev.substring (abbrev.length() - 30);
        DBG ("====================================================");
        DBG ("Loaded file :   " + abbrev);
        DBG ("Channels:          " + String (videoReader->getVideoChannels()));
        DBG ("Duration (sec):    " + String (videoReader->getVideoDuration()));
        DBG ("Framerate (1/sec): " + String (videoReader->getFramesPerSecond()));
        DBG ("SampleRate:        " + String (videoReader->getVideoSamplingRate()));
        DBG ("SampleFormat:      " + String (av_get_sample_fmt_name (videoReader->getSampleFormat())));
        DBG ("Width:             " + String (videoReader->getVideoWidth()));
        DBG ("Height:            " + String (videoReader->getVideoHeight()));
        DBG ("Pixel format:      " + String (av_get_pix_fmt_name (videoReader->getPixelFormat())));
        DBG ("Pixel aspect ratio:" + String (videoReader->getVideoPixelAspect()));
        DBG ("====================================================");

        osdComponent->setVideoLength (videoReader->getVideoDuration ());

        transportSource->setSource (videoReader, 0, nullptr, videoReader->getVideoSamplingRate(), videoReader->getVideoChannels());

        videoAspectRatio = videoReader->getVideoAspectRatio ();
        resized ();

        if (AudioIODevice* device = deviceManager.getCurrentAudioDevice()) {
            videoReader->prepareToPlay (device->getCurrentBufferSizeSamples(),
                                        device->getCurrentSampleRate());

        }
    }

    void presentationTimestampChanged (const double pts) override
    {
        // a Slider::setValue can only occur on message thread, because it triggers a
        // repaint, which fails an assert being the message thread
        //osdComponent->setCurrentTime (videoReader->getCurrentTimeStamp());
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        videoComponent->setBounds (getBounds());
        osdComponent->setBounds (getBounds());
    }


private:
    //==============================================================================

    // Your private member variables go here...
    ScopedPointer<VideoComponentWithDropper> videoComponent;
    ScopedPointer<FFmpegVideoReader>    videoReader;
    ScopedPointer<OSDComponent>         osdComponent;
    ScopedPointer<AudioTransportSource> transportSource;

    double                              videoAspectRatio;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
