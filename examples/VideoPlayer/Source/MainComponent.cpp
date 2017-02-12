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
                                public ButtonListener,
                                public SliderListener,
                                public FFmpegVideoListener
{
public:
    //==============================================================================
    MainContentComponent()
    {
        videoAspectRatio = 1.77;
        videoFileName = new Label();
        videoFileName->setColour (Label::textColourId, Colours::lightblue);
        addAndMakeVisible (videoFileName);

        openFile = new TextButton ("Open", "Open");
        openFile->addListener (this);
        addAndMakeVisible (openFile);

        play = new TextButton ("Play", "Play");
        play->addListener (this);
        addAndMakeVisible (play);

        pause = new TextButton ("Pause", "Pause");
        pause->addListener (this);
        addAndMakeVisible (pause);

        stop = new TextButton ("Stop", "Stop");
        stop->addListener (this);
        addAndMakeVisible (stop);

        osd = new TextButton ("OSD", "OSD");
        osd->setClickingTogglesState (true);
        osd->setToggleState (true, dontSendNotification);
        osd->addListener (this);
        addAndMakeVisible (osd);

        waveform = new TextButton ("Visualiser", "Visualiser");
        waveform->setClickingTogglesState (true);
        addAndMakeVisible (waveform);

        videoReader = new FFmpegVideoReader (384000, 30);
        videoReader->addVideoListener (this);

        transportSource = new AudioTransportSource ();
        transportSource->setSource (videoReader, 0, nullptr);

        videoComponent = new VideoComponentWithDropper (videoReader);
        addAndMakeVisible (videoComponent);

        seekBar = new Slider (Slider::LinearHorizontal, Slider::NoTextBox);
        addAndMakeVisible (seekBar);
        seekBar->addListener (this);

        visualiser = new AudioVisualiserComponent (2);
        visualiser->setSamplesPerBlock (32);
        addAndMakeVisible (visualiser);

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

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()

        videoReader->prepareToPlay (samplesPerBlockExpected, sampleRate);
        transportSource->prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // the AudioTransportSource takes care of start, stop and resample
        transportSource->getNextAudioBlock (bufferToFill);

        if (waveform->getToggleState()) {
            visualiser->pushBuffer (*bufferToFill.buffer);
        }
    }

    void releaseResources() override
    {
        transportSource->releaseResources ();
        videoReader->releaseResources ();
    }

    //==============================================================================
    void buttonClicked (Button* b) override
    {
        if (b == openFile) {
            transportSource->stop();
            FileChooser chooser ("Open Video File");
            if (chooser.browseForFileToOpen()) {
                File video = chooser.getResult();
                videoFileName->setText (video.getFullPathName(), dontSendNotification);
                videoReader->loadMovieFile (video);

            }
        }
        else if (b == play) {
            transportSource->start();
        }
        else if (b == stop) {
            transportSource->stop();
            videoReader->setNextReadPosition (0);
        }
        else if (b == pause) {
            transportSource->stop();
        }
        else if (b == osd) {
            videoComponent->setShowOSD (osd->getToggleState());
            seekBar->setVisible (osd->getToggleState());
        }
    }

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

        seekBar->setRange (0.0, videoReader->getVideoDuration ());

        transportSource->setSource (videoReader, 0, nullptr, videoReader->getVideoSamplingRate(), videoReader->getVideoChannels());

        videoAspectRatio = videoReader->getVideoAspectRatio ();
        resized ();

        if (AudioIODevice* device = deviceManager.getCurrentAudioDevice()) {
            videoReader->prepareToPlay (device->getCurrentBufferSizeSamples(),
                                        device->getCurrentSampleRate());

        }
    }

    /** React to slider changes with seeking */
    void sliderValueChanged (juce::Slider *slider) override
    {
        if (slider == seekBar) {
            videoReader->setNextReadPosition (slider->getValue() * videoReader->getVideoSamplingRate());
        }
    }

    void presentationTimestampChanged (const double pts) override
    {
        // a Slider::setValue can only occur on message thread, because it triggers a
        // repaint, which fails an assert being the message thread
        //seekBar->setValue (pts, dontSendNotification);
    }

    void paint (Graphics& g) override
    {
        // until the presentationTimestampChanged callback works we must update here
        seekBar->setValue (videoReader->getCurrentTimeStamp(), dontSendNotification);
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        videoFileName->setBounds (0, 0, getWidth()-360, 20);
        play->setBounds (getWidth()-300, 0, 40, 20);
        pause->setBounds (getWidth()-260, 0, 40, 20);
        stop->setBounds (getWidth()-220, 0, 40, 20);
        osd->setBounds (getWidth()-180, 0, 40, 20);
        waveform->setBounds (getWidth()-140, 0, 80, 20);
        openFile->setBounds (getWidth()-60, 0, 60, 20);
        const int videoHeight = getHeight() - 20;
        videoComponent->setBounds (0, 20, getWidth(), videoHeight);
        seekBar->setBounds (40, videoHeight - 30, getWidth() - 80, 20);
        visualiser->setBounds (0, videoHeight + 20, getWidth(), getHeight() - (videoHeight + 20));
    }


private:
    //==============================================================================

    // Your private member variables go here...

    ScopedPointer<Label>                videoFileName;
    ScopedPointer<TextButton>           openFile;
    ScopedPointer<TextButton>           play;
    ScopedPointer<TextButton>           pause;
    ScopedPointer<TextButton>           stop;
    ScopedPointer<TextButton>           osd;
    ScopedPointer<TextButton>           waveform;
    ScopedPointer<VideoComponentWithDropper> videoComponent;
    ScopedPointer<FFmpegVideoReader>     videoReader;
    ScopedPointer<AudioTransportSource> transportSource;
    ScopedPointer<AudioVisualiserComponent> visualiser;
    ScopedPointer<Slider>               seekBar;

    double                              videoAspectRatio;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
