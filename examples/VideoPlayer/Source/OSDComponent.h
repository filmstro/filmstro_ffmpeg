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

 Overlay component 

 ==============================================================================
 */


#ifndef OSDCOMPONENT_H_INCLUDED
#define OSDCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class OSDComponent    : public Component,
                        public Slider::Listener,
                        public Button::Listener
{
public:
    OSDComponent (FFmpegVideoReader* readerToControl, AudioTransportSource* transportToControl)
    : videoReader (readerToControl), transport (transportToControl)
    {
        ffwdSpeed = 2;

        setInterceptsMouseClicks (false, true);

        //flexBox.items.add (FlexItem().withFlex (8.0, 1.0).withAlignSelf (FlexItem::AlignSelf::flexEnd));

        openFile = new TextButton ("Open", "Open");
        openFile->addListener (this);
        addAndMakeVisible (openFile);
        flexBox.items.add (FlexItem (*openFile).withFlex (1.0, 1.0, 0.5).withHeight (20.0));

        saveFile = new TextButton ("Save", "Save");
        saveFile->addListener (this);
        addAndMakeVisible (saveFile);
        flexBox.items.add (FlexItem (*saveFile).withFlex (1.0, 1.0, 0.5).withHeight (20.0));

        seekBar = new Slider (Slider::LinearHorizontal, Slider::NoTextBox);
        addAndMakeVisible (seekBar);
        seekBar->addListener (this);
        flexBox.items.add (FlexItem (*seekBar).withFlex (6.0, 1.0, 0.5).withHeight (20.0));

        stop = new TextButton ("Stop", "Stop");
        stop->addListener (this);
        addAndMakeVisible (stop);
        flexBox.items.add (FlexItem (*stop).withFlex (1.0, 1.0, 0.5).withHeight (20.0));

        pause = new TextButton ("Pause", "Pause");
        pause->addListener (this);
        addAndMakeVisible (pause);
        flexBox.items.add (FlexItem (*pause).withFlex (1.0, 1.0, 0.5).withHeight (20.0));

        play = new TextButton ("Play", "Play");
        play->addListener (this);
        addAndMakeVisible (play);
        flexBox.items.add (FlexItem (*play).withFlex (1.0, 1.0, 0.5).withHeight (20.0));

        ffwd = new TextButton ("FFWD", "FFWD");
        ffwd->addListener (this);
        addAndMakeVisible (ffwd);
        flexBox.items.add (FlexItem (*ffwd).withFlex (1.0, 1.0, 0.5).withHeight (20.0));

    }

    ~OSDComponent()
    {
    }

    void paint (Graphics& g) override
    {
        // is transparent
        seekBar->setValue (videoReader->getCurrentTimeStamp(), dontSendNotification);
    }

    void resized() override
    {
        Rectangle<int> bounds = getBounds().withTop (getHeight() - 40);
        flexBox.performLayout (bounds);
    }

    void setCurrentTime (const double time)
    {
        seekBar->setValue (time, dontSendNotification);
    }

    void setVideoLength (const double length)
    {
        seekBar->setRange (0.0, length);
    }

    /** React to slider changes with seeking */
    void sliderValueChanged (juce::Slider* slider) override
    {
        if (slider == seekBar) {
            videoReader->setNextReadPosition (slider->getValue() * videoReader->getVideoSamplingRate());
        }
    }

    void buttonClicked (Button* b) override
    {
        if (b == openFile) {
            transport->stop();
            FileChooser chooser ("Open Video File");
            if (chooser.browseForFileToOpen()) {
                File video = chooser.getResult();
                videoReader->loadMovieFile (video);

            }
        }
        else if (b == play) {
            if (ffwdSpeed != 2) {
                int64 lastPos = videoReader->getNextReadPosition();
                ffwdSpeed = 2;
                double factor = 0.5 + (ffwdSpeed / 4.0);
                transport->setSource (videoReader, 0, nullptr, videoReader->getVideoSamplingRate() * factor, videoReader->getVideoChannels());
                videoReader->setNextReadPosition (lastPos);
            }
            transport->start();
        }
        else if (b == stop) {
            transport->stop();
            videoReader->setNextReadPosition (0);
        }
        else if (b == pause) {
            transport->stop();
        }
        else if (b == ffwd) {
            int64 lastPos = videoReader->getNextReadPosition();
            ffwdSpeed = ++ffwdSpeed % 7;
            double factor = 0.5 + (ffwdSpeed / 4.0);
            transport->setSource (videoReader, 0, nullptr, videoReader->getVideoSamplingRate() * factor, videoReader->getVideoChannels());
            videoReader->setNextReadPosition (lastPos);
            transport->start ();

        }
        else if (b == saveFile) {
            transport->stop();
            FileChooser chooser ("Save Video File");
            if (chooser.browseForFileToSave (true)) {
                File saveFileName = chooser.getResult();

                FFmpegVideoWriter writer;
                writer.copySettingsFromContext (videoReader->getVideoContext());
                writer.copySettingsFromContext (videoReader->getAudioContext());
                writer.copySettingsFromContext (videoReader->getSubtitleContext());

                writer.openMovieFile (saveFileName);

                videoReader->setNextReadPosition (0);
                videoReader->addVideoListener (&writer);

                AudioBuffer<float> buffer;
                buffer.setSize (2, 1024);

                while (videoReader->getCurrentTimeStamp() < videoReader->getVideoDuration()) {
                    AudioSourceChannelInfo info (&buffer, 0, 1024);
                    videoReader->getNextAudioBlock (info);

                    writer.writeNextAudioBlock (info);
                }

                videoReader->removeVideoListener (&writer);
            }
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSDComponent)

    FlexBox                             flexBox;

    ScopedPointer<Slider>               seekBar;
    ScopedPointer<TextButton>           openFile;
    ScopedPointer<TextButton>           saveFile;
    ScopedPointer<TextButton>           play;
    ScopedPointer<TextButton>           pause;
    ScopedPointer<TextButton>           stop;
    ScopedPointer<TextButton>           ffwd;
    int                                 ffwdSpeed;
    FFmpegVideoReader*                  videoReader;

    AudioTransportSource*               transport;
};


#endif  // OSDCOMPONENT_H_INCLUDED
