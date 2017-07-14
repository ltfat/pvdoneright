/*
  ==============================================================================

    FileAudioSource.h
    Created: 12 Jul 2017 11:42:16pm
    Author:  susnak

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class FileAudioSource: public AudioTransportSource
{
public:
    FileAudioSource();
    ~FileAudioSource();
    void setFile(File f);
private:
    ScopedPointer<AudioFormatReaderSource> formatReaderSource;
    TimeSliceThread filePreloadThread{"filePreThread"};
    AudioTransportSource transportSource;
    AudioFormatManager formatManager;
    AudioFormatReader* reader;
    int samplesToPreload {(int)1e6};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileAudioSource)
};
