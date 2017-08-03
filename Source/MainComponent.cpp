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
            scal = 1.0/(1.00 - scal * ((maxRatio - 1.0) / maxRatio));
        else
            scal = 1.00 + scal * ((maxRatio - 1.0) / maxRatio);

        scal *= 100.0;

        return String(round(scal)) += String("%");
    }

    double getValueFromText( const String& scalstr )
    {
        double scal = Slider::getValueFromText(scalstr) / 100.0;

        if (scal >= 1.0)
            scal = (scal - 1.0) / (scal*(maxRatio - 1)/maxRatio);
        else
            scal = (scal - 1.0) * maxRatio / (maxRatio - 1.0);

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
    public KeyListener,
    public FileBrowserListener,
    public FilenameComponentListener,
    public ChangeListener

{
public:
    //==============================================================================
    MainContentComponent()
    {
        MainContentComponent::formatManager.registerBasicFormats();
        ffilter = new WildcardFileFilter(MainContentComponent::formatManager.getWildcardForAllFormats(),
                                         String("*"), String("Audio Files"));

        fsource = new FileAudioSource(&MainContentComponent::formatManager, false);
        fsource->addChangeListener(this);
        ssource = new ScalingAudioSource(fsource, false, 2, maxRatio);
        dirCont = new DirectoryContentsList(ffilter, directoryMonitoringThread);
        fileList = new FileListComponent(*dirCont);
        fileList->addListener(this);

        fnamecomp = new FilenameComponent(String("Directory"), File(), false, true, false, String(), String(), String());
        addAndMakeVisible(fnamecomp);
        fnamecomp->addListener(this);

        addAndMakeVisible(fileList);

        addAndMakeVisible (stretchSlider);
        stretchSlider.setRange (-1, 1, 0.0005);
        stretchSlider.addListener(this);

        addAndMakeVisible(fileButton);
        fileButton.addListener(this);

        addAndMakeVisible(upButton);
        upButton.addListener(this);

        addAndMakeVisible(deviceManagerButton);
        deviceManagerButton.addListener(this);

        pBar.setPercentageDisplay(true);
        addAndMakeVisible(pBar);

        addKeyListener(this);


        setSize (400, 600);

        //showAudioDeviceManagerDialog();

        // specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
        File folder(String("~"));
        fileButton.setButtonText("Load audio file");
//#ifdef JUCE_ANDROID
        fsource->setFile(File("/storage/sdcard0/Music/Walk.wav"));
//#else
//        fsource.setFile(File("~/Desktop/noise.wav"));
//#endif
//
        // dirCont->setDirectory(folder, true, true);
        fnamecomp->setCurrentFile(folder, true, sendNotification);
        fsource->start();
        directoryMonitoringThread.startThread();
    }

    ~MainContentComponent()
    {
        MainContentComponent::formatManager.clearFormats();
        shutdownAudio();
        directoryMonitoringThread.stopThread(1000);
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()
        ssource->prepareToPlay(samplesPerBlockExpected, sampleRate);
        // ssource.setScalingRatio(1.0/2.0596);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        ssource->getNextAudioBlock(bufferToFill);

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
        auto r = getLocalBounds().reduced(10,10);
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        //
        //

        //stretchSlider.setBounds(10, 20, getWidth() - 10, 20);
        stretchSlider.setBounds(r.removeFromTop(40).reduced(0,10));

        // fileButton.setBounds(10, 45, getWidth() / 4, 20);
        // deviceManagerButton.setBounds(20 + getWidth() / 4, 45, getWidth() / 4 , 20);
        // fnamecomp->setBounds(10, 100, getWidth() - 80, 20);
        auto chooserPanel = r.removeFromTop(20);
        upButton.setBounds(chooserPanel.removeFromRight(50));
        fnamecomp->setBounds(chooserPanel);
        // fileList->setBounds(10, 130, getWidth() - 20, getHeight() - 90);
        r.removeFromTop(10);
        auto statusPanel = r.removeFromBottom(30).withTrimmedTop(10);
        pBar.setBounds(statusPanel.removeFromLeft(100));
        deviceManagerButton.setBounds(statusPanel.removeFromRight(100));

        fileList->setBounds(r);
    }

    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &stretchSlider)
        {
            double scal = stretchSlider.getValue();
            if (scal >= 0.0)
                scal = 1.0/(1.00 - scal * ((maxRatio - 1.0) / maxRatio));
            else
                scal = 1.00 + scal * ((maxRatio - 1.0) / maxRatio);


            // if (scal >= 0.0)
            //     scal = 1.0 + (maxRatio - 1.0) * scal;
            // else
            //     scal = 1.00 + scal * ((maxRatio - 1.0) / maxRatio);
            ssource->setScalingRatio(scal);
            // cout << scal << endl;
        }
    }

    void buttonClicked (Button* b) override
    {
        if (b == &fileButton)
        {
            FileChooser fc ("Choose audio file", File::getSpecialLocation (File::userHomeDirectory), "*", false);
#ifndef JUCE_ANDROID
            if (fc.browseForFileToOpen())
            {
                File f(fc.getResult());

                fsource->setFile(f);
                fsource->start();
            }
#endif
        }
        else if (b == &deviceManagerButton)
        {
            showAudioDeviceManagerDialog();
        }
        else if (b == &upButton)
        {
            File parent = fnamecomp->getCurrentFile().getParentDirectory();
            fnamecomp->setCurrentFile(parent, true, sendNotification);
        }
    }

    bool keyPressed (const KeyPress &key, Component *originatingComponent)
    {
        double jumpby =  stretchSlider.getInterval();

        jumpby *= 10;

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

    void selectionChanged ()
    {}

    void fileClicked (const File &file, const MouseEvent &e)
    {}

    void fileDoubleClicked (const File &file)
    {
        if (file.isDirectory())
        {
            fnamecomp->setCurrentFile(file, true, sendNotification);
        }
        else
        {
            fsource->setFile(file);
            fsource->start();
        }
    }

    void browserRootChanged (const File &newRoot)
    {}

    void filenameComponentChanged (FilenameComponent *fileComponentThatHasChanged)
    {
        dirCont->setDirectory(fileComponentThatHasChanged->getCurrentFile(), true, true);
    }

    void changeListenerCallback(ChangeBroadcaster* source)
    {
        if (fsource->hasStreamFinished())
        {
            if (fnamecomp->getCurrentFile() == fsource->file.getParentDirectory() )
            {
                int currIdx = -1;
                for (int fIdx = 0; fIdx < dirCont->getNumFiles(); fIdx++)
                {
                    File ftmp = dirCont->getFile(fIdx);
                    if (fsource->file == ftmp)
                    {
                        currIdx = fIdx;
                        break;
                    }
                }

                if (currIdx < 0) return;

                String wildcards= MainContentComponent::formatManager.getWildcardForAllFormats().replace("*","");

                for (int fIdx = currIdx + 1; fIdx < dirCont->getNumFiles(); fIdx++)
                {
                    File ftmp = dirCont->getFile(fIdx);
                    if (!ftmp.isDirectory() && ftmp.hasFileExtension(wildcards))
                    {
                        fileList->setSelectedFile(ftmp);
                        fsource->setFile(ftmp);
                        fsource->start();
                        break;
                    }
                }


            }
        }
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
    ScopedPointer<FileAudioSource> fsource;
    ScopedPointer<ScalingAudioSource> ssource;

    ScalingSlider stretchSlider;
    TextButton fileButton{"Load Audio File"};
    TextButton deviceManagerButton{"Open Device Manager"};
    TextButton upButton{"Up"};
    double cpuUsage{0.0};
    ProgressBar pBar{cpuUsage};

    TimeSliceThread directoryMonitoringThread{"dirMonThread"};
    ScopedPointer<DirectoryContentsList> dirCont;
    ScopedPointer<FileListComponent> fileList;
    ScopedPointer<WildcardFileFilter> ffilter;
    ScopedPointer<FilenameComponent> fnamecomp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
public:
    static AudioFormatManager formatManager;
};

AudioFormatManager MainContentComponent::formatManager{};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()
{
    return new MainContentComponent();
}
