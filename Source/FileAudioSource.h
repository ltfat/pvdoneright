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
    FileAudioSource(AudioFormatManager* afm, bool setOwnership);
    ~FileAudioSource();
    void setFile(File f);

    File file;
private:
    TimeSliceThread filePreloadThread{"filePreThread"};
    OptionalScopedPointer<AudioFormatManager> formatManager;
    std::unique_ptr<AudioFormatReaderSource> formatReaderSource;
    AudioTransportSource transportSource;
    AudioFormatReader* reader;
    int samplesToPreload {(int)1e6};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileAudioSource)
};
