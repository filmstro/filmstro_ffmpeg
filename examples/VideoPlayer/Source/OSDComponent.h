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

        idle = new MouseIdle (*this);
    }

    ~OSDComponent()
    {
    }

    void paint (Graphics& g) override
    {
        if (videoReader && videoReader->getVideoDuration() > 0) {
            g.setColour (Colours::white);
            g.setFont (24);
            String dim = String (videoReader->getVideoWidth()) + " x " + String (videoReader->getVideoHeight());
            g.drawFittedText (dim, getLocalBounds(), Justification::topLeft, 1);
            g.drawFittedText (FFmpegVideoReader::formatTimeCode (videoReader->getCurrentTimeStamp ()),
                              getLocalBounds(), Justification::topRight, 1);
        }
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

                FFmpegVideoReader copyReader;
                copyReader.loadMovieFile (videoReader->getVideoFileName());
                copyReader.prepareToPlay (1024, videoReader->getVideoSamplingRate());

                FFmpegVideoWriter writer;
                AVRational videoTimeBase = copyReader.getVideoTimeBase();
                if (videoTimeBase.num > 0) {
                    writer.setTimeBase (AVMEDIA_TYPE_VIDEO, videoTimeBase);
                }
                writer.copySettingsFromContext (copyReader.getVideoContext());
                writer.copySettingsFromContext (copyReader.getAudioContext());
                writer.copySettingsFromContext (copyReader.getSubtitleContext());

                writer.openMovieFile (saveFileName);

                copyReader.setNextReadPosition (0);
                copyReader.addVideoListener (&writer);

                AudioBuffer<float> buffer;
                buffer.setSize (2, 1024);

                while (copyReader.getCurrentTimeStamp() < copyReader.getVideoDuration()) {
                    AudioSourceChannelInfo info (&buffer, 0, 1024);
                    copyReader.waitForNextAudioBlockReady (info, 500);
                    copyReader.getNextAudioBlock (info);

                    writer.writeNextAudioBlock (info);
                }
                writer.closeMovieFile ();

                copyReader.removeVideoListener (&writer);
            }
        }
    }

    class MouseIdle : public MouseListener, public Timer
    {
    public:
        MouseIdle (Component& c) :
        component (c),
        lastMovement (Time::getMillisecondCounter())
        {
            Desktop::getInstance().addGlobalMouseListener (this);
            startTimerHz (20);
        }

        void timerCallback () override
        {
            const int64 relTime = Time::getMillisecondCounter() - lastMovement;
            if (relTime < 2000) {
                component.setVisible (true);
                component.setAlpha (1.0);
                component.setMouseCursor (MouseCursor::StandardCursorType::NormalCursor);
            }
            else if (relTime < 2300) {
                component.setAlpha (1.0 - jmax (0.0, (relTime - 2000.0) / 300.0));
                component.setMouseCursor (MouseCursor::StandardCursorType::NoCursor);
            }
            else {
                component.setVisible (false);
                component.setMouseCursor (MouseCursor::StandardCursorType::NoCursor);
            }
        }

        void mouseMove (const MouseEvent &event) override
        {
            if (event.position.getDistanceFrom (lastPosition) > 3.0) {
                lastMovement = Time::getMillisecondCounter();
                lastPosition = event.position;
            }
        }
    private:
        Component&   component;
        int64        lastMovement;
        Point<float> lastPosition;
    };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSDComponent)

    FlexBox                             flexBox;

    ScopedPointer<MouseIdle>            idle;
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
