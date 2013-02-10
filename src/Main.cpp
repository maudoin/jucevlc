
#include "juce.h"
#include "VideoComponent.h"
#include "LookNFeel.h"

//==============================================================================
/**
    This is the top-level window that we'll pop up. Inside it, we'll create and
    show a component from the VideoComponent.cpp file (you can open this file using
    the Jucer to edit it).
*/
class VLCWindow  : public juce::DocumentWindow , public juce::KeyListener
{
	LnF lnf;
public:
    //==============================================================================
    VLCWindow()
        : DocumentWindow ("JucyVLC",
                          juce::Colours::lightgrey,
                          DocumentWindow::allButtons,
                          true)
    {
        juce::LookAndFeel::setDefaultLookAndFeel (&lnf);

        // Create an instance of our main content component, and add it to our window..
		VideoComponent* content = new VideoComponent();
        setContentOwned (content, true);
		content->setScaleComponent(this);

        // Centre the window on the screen
        centreWithSize (getWidth(), getHeight());


		addKeyListener(this);

		setFullScreen();

        // And show it!
        setVisible (true);
    }

    ~VLCWindow()
    {
		removeKeyListener(this);

		juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
        // (the content component will be deleted automatically, so no need to do it here)
    }

    //==============================================================================
    void closeButtonPressed()
    {
        // When the user presses the close button, we'll tell the app to quit. This
        // VLCWindow object will be deleted by the VLCApplication class.
        juce::JUCEApplication::quit();
    }
    bool keyPressed (const juce::KeyPress& key,
                             Component* originatingComponent)
	{
		if(key.isKeyCurrentlyDown(juce::KeyPress::returnKey) && key.getModifiers().isAltDown())
		{
			vf::MessageThread::getInstance().queuef(std::bind  (&VLCWindow::switchFullScreen,this));
			return true;
		}
		return false;

	}
	void setFullScreen()
	{
		setResizable(false, false);
		setUsingNativeTitleBar(true);
		juce::Desktop::getInstance().setKioskModeComponent (getTopLevelComponent());
		//setTitleBarHeight(0);
		//setFullScreen(true);
	}
	void switchFullScreen()
	{
		if (juce::Desktop::getInstance().getKioskModeComponent() == nullptr)
		{
			setFullScreen();
		}
		else
		{
			setResizable(true, false);
		    setUsingNativeTitleBar(true);
			juce::Desktop::getInstance().setKioskModeComponent (nullptr);
			//setTitleBarHeight(20);
		    //setFullScreen(false);
			//setTitleBarButtonsRequired(DocumentWindow::TitleBarButtons::closeButton, false);
		}
    }
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
        window = new VLCWindow();

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
        window = 0;
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
    juce::ScopedPointer<VLCWindow> window;
};


//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (VLCApplication)
