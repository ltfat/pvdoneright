/*
  ==============================================================================

    FileAudioSource.cpp
    Created: 12 Jul 2017 11:42:16pm
    Author:  susnak

  ==============================================================================
*/

#include "FileAudioSource.h"

FileAudioSource::FileAudioSource()
{
    formatManager.registerBasicFormats();


    filePreloadThread.startThread();


}

FileAudioSource::~FileAudioSource()
{
    filePreloadThread.stopThread(1000);
    stop();
    //reader = nullptr;
    setSource(nullptr);

}

void FileAudioSource::setFile(File f)
{
    stop();
    setSource(nullptr);
    formatReaderSource = nullptr;

    reader = formatManager.createReaderFor(f);

    if (reader != nullptr)
    {
        formatReaderSource = new AudioFormatReaderSource(reader, true);
        formatReaderSource->setLooping(true);

        setSource(formatReaderSource, samplesToPreload, &filePreloadThread, reader->sampleRate);
    }

}
