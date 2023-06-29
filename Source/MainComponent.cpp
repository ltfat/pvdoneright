/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "FileAudioSource.h"
#include "ScalingAudioSource.h"

enum class PlayerMode
{
    TIME_SCALING = 1,
    PITCH_SHIFTING = 2,
    TIME_PITCH_SCALING = 3
};

enum class PlayerState
{
    PAUSED = 1,
    PLAYING = 2,
    STOPPED = 3
};

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
    public ComboBox::Listener,
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
        ffilter = std::make_unique<WildcardFileFilter>(MainContentComponent::formatManager.getWildcardForAllFormats(),
                                         String("*"), String("Audio Files"));
        wildcards = MainContentComponent::formatManager.getWildcardForAllFormats().replace("*","");

        fsource = std::make_unique<FileAudioSource>(&MainContentComponent::formatManager, false);
        fsource->addChangeListener(this);
        pre_ressource = std::make_unique<ResamplingAudioSource>(fsource.get(), false);
        ssource = std::make_unique<ScalingAudioSource>(pre_ressource.get(), false, 2, maxRatio);
        post_ressource = std::make_unique<ResamplingAudioSource>(ssource.get(), false);
        dirCont = std::make_unique<DirectoryContentsList>(ffilter.get(), directoryMonitoringThread);
        fileList = std::make_unique<FileListComponent>(*dirCont.get());
        fileList->addListener(this);

        fnamecomp = std::make_unique<FilenameComponent>(String("Directory"), File(), false, true, false, String(), String(), String());
        addAndMakeVisible(fnamecomp.get());
        fnamecomp->addListener(this);

        addAndMakeVisible(&modelabel);
        modebox.addItem("Time scaling",static_cast<int>(PlayerMode::TIME_SCALING));
        modebox.addItem("Pitch shifting",static_cast<int>(PlayerMode::PITCH_SHIFTING));
        modebox.addItem("Both",static_cast<int>(PlayerMode::TIME_PITCH_SCALING));
        modebox.setSelectedId(static_cast<int>(PlayerMode::TIME_SCALING));
        modebox.addListener(this);
        addAndMakeVisible(&modebox);

        addAndMakeVisible(&playpauseButton);
        playpauseButton.addListener(this);
        addAndMakeVisible(&prevTrackButton);
        prevTrackButton.addListener(this);
        addAndMakeVisible(&nextTrackButton);
        nextTrackButton.addListener(this);

        addAndMakeVisible(fileList.get());

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
        File folder(String("~"));
        fileButton.setButtonText("Load audio file");
        setAudioChannels (0, 2);
#ifdef JUCE_ANDROID
#else
//        fsource.setFile(File("~/Desktop/noise.wav"));
#endif

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
        //ssource->prepareToPlay(samplesPerBlockExpected, sampleRate);
        post_ressource->prepareToPlay(samplesPerBlockExpected, sampleRate);
        // ssource.setScalingRatio(1.0/2.0596);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        post_ressource->getNextAudioBlock(bufferToFill);

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
        modebox.setBounds(r.removeFromTop(40).reduced(60,4));
        stretchSlider.setBounds(r.removeFromTop(40).reduced(0,10));
        // auto buttonPannel = r.removeFromTop(40).withSizeKeepingCentre(90,30);


        // fileButton.setBounds(10, 45, getWidth() / 4, 20);
        // deviceManagerButton.setBounds(20 + getWidth() / 4, 45, getWidth() / 4 , 20);
        // fnamecomp->setBounds(10, 100, getWidth() - 80, 20);
        auto chooserPanel = r.removeFromTop(20);
        upButton.setBounds(chooserPanel.removeFromRight(50));
        fnamecomp->setBounds(chooserPanel);
        // fileList->setBounds(10, 130, getWidth() - 20, getHeight() - 90);
        r.removeFromTop(10);
        auto statusPanel = r.removeFromBottom(30).withTrimmedTop(10);

        auto buttonPannel = statusPanel.withSizeKeepingCentre(90,30);

        prevTrackButton.setBounds(buttonPannel.removeFromLeft(30).reduced(5,5));
        playpauseButton.setBounds(buttonPannel.removeFromLeft(30));
        nextTrackButton.setBounds(buttonPannel.reduced(5,5));

        pBar.setBounds(statusPanel.removeFromLeft(100));
        deviceManagerButton.setBounds(statusPanel.removeFromRight(100));

        fileList->setBounds(r);
    }

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
    {
        mode = static_cast<PlayerMode>(comboBoxThatHasChanged->getSelectedId());
        sliderValueChanged(&stretchSlider);
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
            //
            if( mode == PlayerMode::PITCH_SHIFTING )
            {
                pre_ressource->setResamplingRatio(jmax(1.0,scal));
                post_ressource->setResamplingRatio(jmin(1.0,scal));
                ssource->setScalingRatio(1.0/scal);
            }
            else if( mode == PlayerMode::TIME_SCALING)
            {
                pre_ressource->setResamplingRatio(1);
                post_ressource->setResamplingRatio(1);
                ssource->setScalingRatio(scal);
            }
            else if ( mode == PlayerMode::TIME_PITCH_SCALING)
            {
                pre_ressource->setResamplingRatio(jmax(1.0,scal));
                post_ressource->setResamplingRatio(jmin(1.0,scal));
                ssource->setScalingRatio(1.0);
            }

            // cout << scal << endl;
        }
    }

    bool tryToPlayFile(File ftmp)
    {
        if (!ftmp.isDirectory() && ftmp.hasFileExtension(wildcards))
        {
            fileList->setSelectedFile(ftmp);
            fsource->setFile(ftmp);
            fsource->start();
            playpauseButton.setToggleState(true,dontSendNotification);
            return true;
        }
        playpauseButton.setToggleState(false,dontSendNotification);
        return false;
    }

    void playNextSong(bool nextSong)
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


                if (nextSong)
                {
                    for (int fIdx = currIdx + 1; fIdx < dirCont->getNumFiles(); fIdx++)
                        if(tryToPlayFile( dirCont->getFile(fIdx))) break;
                }
                else
                {
                    for (int fIdx = currIdx - 1; fIdx >= 0; fIdx--)
                        if(tryToPlayFile( dirCont->getFile(fIdx))) break;
                }

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
                tryToPlayFile(f);
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
        else if (b == &nextTrackButton)
        {
            playNextSong(true);
        }
        else if (b == &prevTrackButton)
        {
            playNextSong(false);
        }
        else if (b == &playpauseButton)
        {
            if( playpauseButton.getToggleState())
                fsource->start();
            else
                fsource->stop();

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
        else if (key == KeyPress::spaceKey)
        {
            playpauseButton.setToggleState(!playpauseButton.getToggleState(),sendNotification);
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
            tryToPlayFile(file);
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
            playNextSong(true);
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
    std::unique_ptr<FileAudioSource> fsource;
    std::unique_ptr<ResamplingAudioSource> pre_ressource;
    std::unique_ptr<ScalingAudioSource> ssource;
    std::unique_ptr<ResamplingAudioSource> post_ressource;

    ScalingSlider stretchSlider;
    TextButton fileButton{"Load Audio File"};
    TextButton deviceManagerButton{"Open Device Manager"};
    TextButton upButton{"Up"};

    ToggleButton playpauseButton;
    ArrowButton prevTrackButton{String("Prev"), 0.5f, Colours::white};
    ArrowButton nextTrackButton{String("Next"), 0.0f, Colours::white};

    double cpuUsage{0.0};
    ProgressBar pBar{cpuUsage};
    Label modelabel{"Playback mode:"};
    ComboBox modebox;

    TimeSliceThread directoryMonitoringThread{"dirMonThread"};
    std::unique_ptr<DirectoryContentsList> dirCont;
    std::unique_ptr<FileListComponent> fileList;
    std::unique_ptr<WildcardFileFilter> ffilter;
    std::unique_ptr<FilenameComponent> fnamecomp;
    PlayerMode mode {PlayerMode::TIME_SCALING};

    String wildcards;

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
