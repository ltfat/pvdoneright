/*
  ==============================================================================

    ScalingAudioSource.h
    Created: 12 Jul 2017 4:45:09pm
    Author:  susnak

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"


class ScalingAudioSource: public AudioSource
{
public:
    ScalingAudioSource(AudioSource* inputSource,
                       bool deleteInputWhenDeleted,
                       int numChannels = 2, double maxScalingRatio = 4.0);

    ~ScalingAudioSource();

    void setScalingRatio(double samplesInPerOutputSample);

    double getScalingRatio() const noexcept {return ratio;}

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;
private:
    OptionalScopedPointer<AudioSource> input;
    AudioSampleBuffer buffer;
    
    double ratio;
    double maxRatio;
    int numChannels;
    SpinLock ratioLock;
    ltfat_pv_state_s* pv{nullptr};
    float* outPtr[10];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScalingAudioSource)
};
