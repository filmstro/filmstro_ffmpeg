
filmstro_ffmpeg
===============

by Daniel Walz / Filmstro Ltd.
Published under the BSD License (3 clause)

This is a module to read video files and play/display it using the JUCE 
framework (juce.com). The audio will be available as a juce::AudioSource and 
can be processed through a regular processing chain.

Find the [API documentation here](https://filmstro.github.io/filmstro_ffmpeg/docs/)

Dependencies
============

The reading is done using ffMpeg (ffmpeg.org). You will have to build ffmpeg
and link your project to it, if it's not already installed. We use this flags:

OSX:

    ./configure --cc=CC --arch=x86_64 --disable-static --enable-shared --disable-stripping --disable-debug --install-name-dir='@loader_path'

Windows:

    ./configure --toolchain=msvc --disable-static --enable-shared --prefix=../build/windows --arch=x86 

To make

    make -j10
    make install

You can add an audio meter simply by adding the module https://github.com/ffAudio/ff_meters to the projucer project.


Current State
=============

It works for us with mp4 files. However, there are many formats that theoretically should work, but some create
problems, probably to do with the sequence of packets / frames in the stream. To
improve this any insights and findings are very welcome.

There is an example how to write e.g. an Animation to a file, see examples/AnimationWriter. The animation is
simply copied from the JUCE examples and the writer is added.

********************************************************************************

We hope it is of any use, let us know of any problems or improvements you may 
come up with...

Brighton, 9th February 2017

********************************************************************************
