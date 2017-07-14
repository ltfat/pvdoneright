/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "FileAudioSource.h"
#include "ScalingAudioSource.h"

class ScalingSlider: public Slider
{
    public:
    String getTextFromValue( double	scal	)
    {
        if (scal>=0.0)
            scal = 1.0 + 3.0*scal;
        else
            scal = 1.00 + scal*(3.0/4.0);
            
        return String(scal);
    }

};


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public AudioAppComponent, public Slider::Listener 
                                
{
public:
    //==============================================================================
    MainContentComponent()
    {
        addAndMakeVisible (stretchSlider);
        stretchSlider.setRange (-1, 1); 
        stretchSlider.addListener(this);
        
        setSize (800, 600);
                
        showAudioDeviceManagerDialog();
        
        // specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
        fsource.setFile(File("~/Desktop/Domination.wav"));
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
        ssource.prepareToPlay(samplesPerBlockExpected,sampleRate);
        //ssource.setScalingRatio(2.0);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        ssource.getNextAudioBlock(bufferToFill);
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
        stretchSlider.setBounds(10,20, getWidth(),20);
    }
    
    void sliderValueChanged (Slider* slider) override
{
    if (slider == &stretchSlider)
    {
        double scal = stretchSlider.getValue();
        if (scal>=0.0)
            scal = 1.0 + 3.0*scal;
        else
            scal = 1.00 + scal*(3.0/4.0);
        ssource.setScalingRatio(scal);
    }
}
    
    void showAudioDeviceManagerDialog()
{

    DialogWindow::LaunchOptions o;

    o.content.setOwned(new AudioDeviceSelectorComponent(
        deviceManager, 0, 0, 0, 2, false, false, true, false));

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
    ScalingAudioSource ssource{&fsource,false};
    
    ScalingSlider stretchSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }
