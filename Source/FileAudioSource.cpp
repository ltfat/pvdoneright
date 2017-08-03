/*
  ==============================================================================

    FileAudioSource.cpp
    Created: 12 Jul 2017 11:42:16pm
    Author:  susnak

  ==============================================================================
*/

#include "FileAudioSource.h"

FileAudioSource::FileAudioSource(AudioFormatManager* afm, bool setOwnership)
    : formatManager(afm,setOwnership)
{
    filePreloadThread.startThread();

}

FileAudioSource::~FileAudioSource()
{
    filePreloadThread.stopThread(1000);
    stop();
    // formatReaderSource = nullptr;
    setSource(nullptr);
}

void FileAudioSource::setFile(File f)
{
    stop();
    setSource(nullptr);
    formatReaderSource = nullptr;

    reader = formatManager->createReaderFor(f);

    if (reader != nullptr)
    {
        file = f;
        formatReaderSource = new AudioFormatReaderSource(reader, true);
        //formatReaderSource->setLooping(true);

        setSource(formatReaderSource, samplesToPreload, &filePreloadThread, reader->sampleRate);
    }

}
