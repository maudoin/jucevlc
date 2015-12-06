
#include "AppConfig.h"
#include "juce.h"
#include "VideoComponent.h"

//==============================================================================
class DirectShowWindowWithFileBrowser  : public juce::Component,
                                         public juce::FilenameComponentListener
{
public:
    DirectShowWindowWithFileBrowser (juce::DirectShowComponent::VideoRendererType type)
        : fileChooser ("movie", juce::File::nonexistent, true, false, false,
                       "*", juce::String::empty, "(choose a video file to play)"),
          dshowComp (type)
    {
        addAndMakeVisible (&dshowComp);
        addAndMakeVisible (&fileChooser);
        fileChooser.addListener (this);
        fileChooser.setBrowseButtonText ("browse");
    }

    void resized()
    {
        dshowComp.setBounds (0, 0, getWidth(), getHeight() - 60);

        if (transportControl != 0)
            transportControl->setBounds (0, dshowComp.getBottom() + 4, getWidth(), 26);

        fileChooser.setBounds (0, getHeight() - 24, getWidth(), 24);
    }

    void filenameComponentChanged (juce::FilenameComponent*)
    {
        juce::String err="none";
        // this is called when the user changes the filename in the file chooser box
        if (dshowComp.loadMovie (fileChooser.getCurrentFile(), err))
        {
            addAndMakeVisible (transportControl = new TransportControl (dshowComp));
            resized();

            dshowComp.play();
        }
        else
        {
            juce::AlertWindow::showMessageBox (juce::AlertWindow::WarningIcon,
                                         err+" --> Couldn't load the file!",
                                         "Sorry, DirectShow didn't manage to load that file!");
        }
    }

private:
    juce::DirectShowComponent dshowComp;
    juce::FilenameComponent fileChooser;

    //==============================================================================
    // A quick-and-dirty transport control, containing a play button and a position slider..
    class TransportControl  : public juce::Component,
                              public juce::ButtonListener,
                              public juce::SliderListener,
                              public juce::Timer
    {
    public:
        TransportControl (juce::DirectShowComponent& dshowComp_)
            : playButton ("Play/Pause"),
              position (juce::String::empty),
              dshowComp (dshowComp_)
        {
            addAndMakeVisible (&playButton);
            playButton.addListener (this);

            addAndMakeVisible (&position);
            position.setRange (0, dshowComp.getMovieDuration(), 0);
            position.setSliderStyle (juce::Slider::LinearHorizontal);
            position.setTextBoxStyle (juce::Slider::NoTextBox, false, 80, 20);
            position.addListener (this);

            startTimer (1000 / 50);
        }

        void buttonClicked (juce::Button* buttonThatWasClicked)
        {
            if (dshowComp.isPlaying())
                dshowComp.stop();
            else
                dshowComp.play();
        }

        void sliderValueChanged (juce::Slider* sliderThatWasMoved)
        {
            dshowComp.setPosition (position.getValue());
        }

        void resized()
        {
            const int playButtonWidth = 90;
            playButton.setBounds (0, 0, playButtonWidth, getHeight());
            position.setBounds (playButtonWidth, 0, getWidth() - playButtonWidth, getHeight());
        }

        void timerCallback()
        {
            if (! position.isMouseButtonDown())
                position.setValue (dshowComp.getPosition());
        }

    private:
        juce::TextButton playButton;
        juce::Slider position;
        juce::DirectShowComponent& dshowComp;
    };

    juce::ScopedPointer<TransportControl> transportControl;
};


//==============================================================================
class DirectShowDemo  : public juce::Component
{
public:
    //==============================================================================
    DirectShowDemo()
        : dsComp1 (juce::DirectShowComponent::dshowVMR7)
#if JUCE_MEDIAFOUNDATION
        , dsComp2 (juce::DirectShowComponent::dshowEVR)
#endif
    {
        setName ("DirectShow");

        // add a movie component..
        addAndMakeVisible (&dsComp1);
#if JUCE_MEDIAFOUNDATION
        addAndMakeVisible (&dsComp2);
#endif
    }

    ~DirectShowDemo()
    {
        dsComp1.setVisible (false);
#if JUCE_MEDIAFOUNDATION
        dsComp2.setVisible (false);
#endif
    }

    void paint (juce::Graphics& g) override
    {
		g.fillAll (juce::Colours::black);
    }
    void resized()
    {
#if JUCE_MEDIAFOUNDATION
        dsComp1.setBoundsRelative (0.05f, 0.05f, 0.425f, 0.9f);
        dsComp2.setBoundsRelative (0.525f, 0.05f, 0.425f, 0.9f);
#else
        dsComp1.setBoundsRelative (0.05f, 0.05f, 0.9f, 0.9f);
#endif
    }

private:
    //==============================================================================
    DirectShowWindowWithFileBrowser dsComp1;
#if JUCE_MEDIAFOUNDATION
    DirectShowWindowWithFileBrowser dsComp2;
#endif
};


//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class VLCApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    VLCApplication()
    {
    }

    ~VLCApplication()
    {
    }

    //==============================================================================
    void initialise (const juce::String& commandLine)
    {
        // For this demo, we'll just create the main window...
        window = new VideoComponent();

        dshow = new DirectShowDemo();
        dshow->addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar);
        dshow->setSize(800, 600);
        dshow->setVisible (true);
        /*  ..and now return, which will fall into to the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().

            In this case, JUCEAppliction::quit() will be called by the
            hello world window being clicked.
        */
    }

    void shutdown()
    {
        // This method is where you should clear-up your app's resources..

        // The window variable is a ScopedPointer, so setting it to a null
        // pointer will delete the window.
        delete window;
    }

    //==============================================================================
    const juce::String getApplicationName()
    {
        return "JucyVLC";
    }

    const juce::String getApplicationVersion()
    {
        // The ProjectInfo::versionString value is automatically updated by the Jucer, and
        // can be found in the JuceHeader.h file that it generates for our project.
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }

    void anotherInstanceStarted (const juce::String& commandLine)
    {
    }

private:
    VideoComponent* window;
    DirectShowDemo* dshow;
};
#ifdef JUCE_64BIT
#include <modules\juce_core\native/juce_BasicNativeHeaders.h>
#endif

//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (VLCApplication)
