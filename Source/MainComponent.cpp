/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "FileAudioSource.h"
#include "ScalingAudioSource.h"

constexpr static double maxRatio = 10;

class ScalingSlider: public Slider
{
public:
    String getTextFromValue( double scal    )
    {
        if (scal >= 0.0)
            scal = 1.0 + (maxRatio - 1.0) * scal;
        else
            scal = 1.00 + scal * ((maxRatio - 1.0) / maxRatio);

        scal *= 100.0;

        return String(round(scal)) += String("%");
    }

    double getValueFromText( const String& scalstr )
    {
        double scal = Slider::getValueFromText(scalstr)/100.0;

        if (scal >= 1.0)
            scal = (scal - 1.0)/(maxRatio - 1);
        else
            scal = (scal - 1.0)*maxRatio/(maxRatio - 1.0);

        return scal;
    }

};


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   :
    public AudioAppComponent,
    public Slider::Listener,
    public Button::Listener,
    public KeyListener

{
public:
    //==============================================================================
    MainContentComponent()
    {
        addAndMakeVisible (stretchSlider);
        stretchSlider.setRange (-1, 1, 0.0005);
        stretchSlider.addListener(this);

        addAndMakeVisible(fileButton);
        fileButton.addListener(this);

        addAndMakeVisible(deviceManagerButton);
        deviceManagerButton.addListener(this);

        pBar.setPercentageDisplay(true);
        addAndMakeVisible(pBar);

        addKeyListener(this);


        setSize (600, 200);

        //showAudioDeviceManagerDialog();

        // specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
        File folder(File::getSpecialLocation(File::userMusicDirectory));
        fileButton.setButtonText("Load audio file");
//#ifdef JUCE_ANDROID
        fsource.setFile(File("/storage/sdcard0/Music/Walk.wav"));
//#else
//        fsource.setFile(File("~/Desktop/noise.wav"));
//#endif
        fsource.start();
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
        ssource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        // ssource.setScalingRatio(1.0/2.0596);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        ssource.getNextAudioBlock(bufferToFill);

        cpuUsage = AudioAppComponent::deviceManager.getCpuUsage();
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        // You can add your drawing code here!
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        stretchSlider.setBounds(10, 20, getWidth() - 10, 20);
        fileButton.setBounds(10, 45, getWidth() / 4, 20);
        deviceManagerButton.setBounds(20 + getWidth() / 4, 45, getWidth() / 4 , 20);
        pBar.setBounds(10, 75, getWidth() / 4, 20);
    }

    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &stretchSlider)
        {
            double scal = stretchSlider.getValue();
            if (scal >= 0.0)
                scal = 1.0 + (maxRatio - 1.0) * scal;
            else
                scal = 1.00 + scal * ((maxRatio - 1.0) / maxRatio);
            ssource.setScalingRatio(scal);
        }
    }

    void buttonClicked (Button* b) override
    {
        if (b == &fileButton)
        {
            FileChooser fc ("Choose audio file", File::getSpecialLocation (File::userHomeDirectory), "*",false);
#ifndef JUCE_ANDROID
            if (fc.browseForFileToOpen())
            {
                File f(fc.getResult());

                fsource.setFile(f);
                fsource.start();
            }
#endif
        }
        else if (b == &deviceManagerButton)
        {
            showAudioDeviceManagerDialog();
        }
    }

    bool keyPressed (const KeyPress &key, Component *originatingComponent)
    {
        double jumpby =  stretchSlider.getInterval();

        jumpby *=10;

        if (key == KeyPress::upKey || key == KeyPress::rightKey)
        {
            stretchSlider.setValue(stretchSlider.getValue() + jumpby);
        }
        else if (key == KeyPress::downKey || key == KeyPress::leftKey)
        {

            stretchSlider.setValue(stretchSlider.getValue() - jumpby);
        }
        return true;
    }

    bool keyStateChanged (bool isKeyDown, Component *originatingComponent)
    {
        return true;
    }

    void showAudioDeviceManagerDialog()
    {

        DialogWindow::LaunchOptions o;

        o.content.setOwned(new AudioDeviceSelectorComponent(
                               deviceManager, 0, 0, 0, 1, false, false, true, false));

        o.content->setSize(500, 450);

        o.dialogTitle = TRANS("Audio Settings");
        o.dialogBackgroundColour = Colour(0xfff0f0f0);
        o.escapeKeyTriggersCloseButton = true;
        o.useNativeTitleBar = true;
        o.resizable = false;

        o.launchAsync();
    }


private:
    //==============================================================================

    // Your private member variables go here...
    FileAudioSource fsource;
    ScalingAudioSource ssource{&fsource, false, 2, maxRatio};

    ScalingSlider stretchSlider;
    TextButton fileButton{"Load Audio File"};
    TextButton deviceManagerButton{"Open Device Manager"};
    double cpuUsage{0.0};
    ProgressBar pBar{cpuUsage};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()
{
    return new MainContentComponent();
}
