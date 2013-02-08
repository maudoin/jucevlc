
#include "juce.h"
#include "MainComponent.h"
#include "LookNFeel.h"

String getFileNameWithoutExtension(const String& fullPath)
{
    const int lastSlash = fullPath.lastIndexOfChar (File::separator) + 1;
    const int lastDot   = fullPath.lastIndexOfChar ('.');

    if (lastDot > lastSlash)
        return fullPath.substring (lastSlash, lastDot);
    else
        return fullPath.substring (lastSlash);
}


//==============================================================================
/**
    This is the top-level window that we'll pop up. Inside it, we'll create and
    show a component from the MainComponent.cpp file (you can open this file using
    the Jucer to edit it).
*/
class VLCWindow  : public DocumentWindow , public juce::KeyListener
{
	LnF lnf;
public:
    //==============================================================================
    VLCWindow()
        : DocumentWindow ("JucyVLC",
                          Colours::lightgrey,
                          DocumentWindow::allButtons,
                          true)
    {
        LookAndFeel::setDefaultLookAndFeel (&lnf);

        // Create an instance of our main content component, and add it to our window..
		MainComponent* content = new MainComponent();
        setContentOwned (content, true);
		content->setScaleComponent(this);

        // Centre the window on the screen
        centreWithSize (getWidth(), getHeight());


		addKeyListener(this);

		switchFullScreen();

        // And show it!
        setVisible (true);
    }

    ~VLCWindow()
    {
		removeKeyListener(this);

		LookAndFeel::setDefaultLookAndFeel (nullptr);
        // (the content component will be deleted automatically, so no need to do it here)
    }

    //==============================================================================
    void closeButtonPressed()
    {
        // When the user presses the close button, we'll tell the app to quit. This
        // VLCWindow object will be deleted by the VLCApplication class.
        JUCEApplication::quit();
    }
    bool keyPressed (const KeyPress& key,
                             Component* originatingComponent)
	{
		if(key.isKeyCurrentlyDown(KeyPress::returnKey) && key.getModifiers().isAltDown())
		{
			switchFullScreen();
			return true;
		}
		return false;

	}
	void switchFullScreen()
	{
		Desktop& desktop = Desktop::getInstance();

		if (desktop.getKioskModeComponent() == nullptr)
		{
			desktop.setKioskModeComponent (getTopLevelComponent());
			setTitleBarHeight(0);
		    setResizable(false, false);
		    //setFullScreen(true);
		}
		else
		{
			desktop.setKioskModeComponent (nullptr);
			setTitleBarHeight(20);
			setResizable(true, false);
		    //setFullScreen(false);
			//setTitleBarButtonsRequired(DocumentWindow::TitleBarButtons::closeButton, false);
		}
    }
};

//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class VLCApplication : public JUCEApplication
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
    void initialise (const String& commandLine)
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
    const String getApplicationName()
    {
        return "JucyVLC";
    }

    const String getApplicationVersion()
    {
        // The ProjectInfo::versionString value is automatically updated by the Jucer, and
        // can be found in the JuceHeader.h file that it generates for our project.
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }

    void anotherInstanceStarted (const String& commandLine)
    {
    }

private:
    ScopedPointer<VLCWindow> window;
};


//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (VLCApplication)
