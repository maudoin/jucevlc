
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
class FrontendMainWindow  : public DocumentWindow , public juce::KeyListener
{
public:
    //==============================================================================
    FrontendMainWindow(const String& commandLine)
        : DocumentWindow ("",
                          Colours::lightgrey,
                          DocumentWindow::allButtons,
                          true)
    {
		

        // Create an instance of our main content component, and add it to our window..
        setContentOwned (new MainComponent(commandLine), true);

        // Centre the window on the screen
        centreWithSize (getWidth(), getHeight());

		//setResizable(true, false);
		//setTitleBarHeight(20);
		//setFullScreen(true);
		//setTitleBarButtonsRequired(DocumentWindow::TitleBarButtons::closeButton, false);

		
		addKeyListener(this);
		switchFullScreen();

        // And show it!
        setVisible (true);
    }

    virtual ~FrontendMainWindow()
    {
        // (the content component will be deleted automatically, so no need to do it here)
    }
	
	void paint (Graphics& g)
	{
        g.fillAll (Colours::black);
	}

    //==============================================================================
    void closeButtonPressed()
    {
        // When the user presses the close button, we'll tell the app to quit. This
        // FrontendMainWindow object will be deleted by the JUCEHelloWorldApplication class.
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
		}
		else
		{
			desktop.setKioskModeComponent (nullptr);
			setTitleBarHeight(20);
			setResizable(true, true);
		}
    }
};

//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class JUCEHelloWorldApplication : public JUCEApplication
{
	LnF lnf;
public:
    //==============================================================================
    JUCEHelloWorldApplication()
    {

    }

    ~JUCEHelloWorldApplication()
    {
    }

    //==============================================================================
    void initialise (const String& commandLine)
    {

        LookAndFeel::setDefaultLookAndFeel (&lnf);

        // For this demo, we'll just create the main window...
        helloWorldWindow = new FrontendMainWindow(commandLine);

		lnf.setScaleComponent(helloWorldWindow);

		vf::MessageThread::getInstance();

        /*  ..and now return, which will fall into to the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().

            In this case, JUCEAppliction::quit() will be called by the
            hello world window being clicked.
        */
    }

    void shutdown()
    {
        LookAndFeel::setDefaultLookAndFeel (nullptr);
        // This method is where you should clear-up your app's resources..

        // The helloWorldWindow variable is a ScopedPointer, so setting it to a null
        // pointer will delete the window.
        helloWorldWindow = 0;
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
    ScopedPointer<FrontendMainWindow> helloWorldWindow;
};


//==============================================================================



	
static juce::JUCEApplicationBase* juce_CreateApplication() { return new JUCEHelloWorldApplication(); } 
extern "C" JUCE_MAIN_FUNCTION 
{ 


    juce::JUCEApplication::createInstance = &juce_CreateApplication; 
	try
	{
		return juce::JUCEApplication::main (JUCE_MAIN_FUNCTION_ARGS); 
	}
	catch(std::exception const&e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}