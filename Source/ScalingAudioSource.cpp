/*
  ==============================================================================

    ScalingAudioSource.cpp
    Created: 12 Jul 2017 4:45:09pm
    Author:  susnak

  ==============================================================================
*/

#include "ScalingAudioSource.h"


ScalingAudioSource::ScalingAudioSource(AudioSource* const inputSource,
                                       const bool deleteInputWhenDeleted,
                                       const int numChannels, double maxScalingRatio )
    : input(inputSource, deleteInputWhenDeleted),
      maxRatio(maxScalingRatio),
      ratio(1.0),
      numChannels (2)
{
}

ScalingAudioSource::~ScalingAudioSource() {}


void ScalingAudioSource::setScalingRatio(const double samplesInPerOutputSample)
{
    jassert (samplesInPerOutputSample > 0);

    const SpinLock::ScopedLockType sl (ratioLock);
    ratio = jmax (0.0, samplesInPerOutputSample);
}

void ScalingAudioSource::releaseResources()
{
    if (pv) ltfat_pv_done_s(&pv);
    pv = nullptr;
    input->releaseResources();
    buffer.setSize (numChannels, 0);
}

void ScalingAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    const int scaledBlockSize = roundToInt (samplesPerBlockExpected * maxRatio);

    ltfat_pv_init_s(maxRatio, numChannels, samplesPerBlockExpected, &pv);
    input->prepareToPlay(scaledBlockSize, sampleRate);

    buffer.setSize (numChannels, scaledBlockSize);
}

void ScalingAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    double localRatio;
    {
        const SpinLock::ScopedLockType sl (ratioLock);
        localRatio = ratio;
    }

    int outLen = info.numSamples;
    int inLen = ltfat_pv_nextinlen_s(pv,outLen);
    int channels = info.buffer->getNumChannels();

    AudioSourceChannelInfo readInfo (&buffer, 0, inLen);
    input->getNextAudioBlock (readInfo);

    const float** inPtr = buffer.getArrayOfReadPointers();
    float** outPtr = info.buffer->getArrayOfWritePointers();

    ltfat_pv_execute_s(pv,inPtr,inLen, channels, localRatio, outLen, outPtr);

}
