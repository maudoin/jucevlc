/*
  ==============================================================================

   Demonstration "Hello World" application in JUCE
   Copyright 2008 by Julian Storer.

  ==============================================================================
*/

#include "juce.h"
#include "MainComponent.h"

String getFileNameWithoutExtension(const String& fullPath)
{
    const int lastSlash = fullPath.lastIndexOfChar (File::separator) + 1;
    const int lastDot   = fullPath.lastIndexOfChar ('.');

    if (lastDot > lastSlash)
        return fullPath.substring (lastSlash, lastDot);
    else
        return fullPath.substring (lastSlash);
}

class LnF : public OldSchoolLookAndFeel
{

public:
	LnF()
	{
		setColour(DirectoryContentsDisplayComponent::textColourId, Colours::white);

	}
	const Font getFontForTextButton (TextButton& button)
	{
		Font f = LookAndFeel::getFontForTextButton(button);
		f.setHeight(24);
		return f;
	}
	
void drawFileBrowserRow (Graphics& g, int width, int height,
                                      const String& filename, Image* icon,
                                      const String& fileSizeDescription,
                                      const String& fileTimeDescription,
                                      const bool isDirectory,
                                      const bool isItemSelected,
                                      const int /*itemIndex*/,
                                      DirectoryContentsDisplayComponent&)
{
	const int filenameWidth = width;//width > 450 ? roundToInt (width * 0.7f) : width;
	
	const int hborder = 5;
	const int roundness = 7;
	
	
	
    if(!isDirectory || isItemSelected)
	{
		g.setGradientFill (ColourGradient (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker(),
										   0, height/2-hborder,
										   Colour (0x8000),
										   0.7*filenameWidth-3, height/2-hborder,
										   false));

		//g.setColour (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker());
		g.fillRoundedRectangle(0, hborder, filenameWidth-3, height-2*hborder, roundness);
	}

    if(!isDirectory)
	{
		g.setGradientFill (ColourGradient(Colours::darkgrey,
										   0, height/2-hborder,
										   Colour (0x8000),
										   0.7*filenameWidth-3, height/2-hborder,
										   false));
		//g.setColour (Colours::darkgrey);
		g.drawRoundedRectangle(0, hborder, filenameWidth-3, height-2*hborder, roundness, 2);
	}
	
	const int iconhborder = 12;
    const int x = 32;
    const int y = height;
    g.setColour (Colours::black);

    if (icon != nullptr && icon->isValid() && !isDirectory)
    {
        g.drawImageWithin (*icon, 2 + iconhborder, 2, x-4, y-4,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);
    }
    else if(isDirectory)
    {
        const Drawable* d = isDirectory ? getDefaultFolderImage()
                                        : getDefaultDocumentFileImage();

        if (d != nullptr)
            d->drawWithin (g, Rectangle<float> (2.0f  + iconhborder, 2.0f, x - 4.0f, y - 4.0f),
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
    }
	
	Font f = g.getCurrentFont();
	f.setTypefaceName("Times New Roman");
	f.setStyleFlags(f.getStyleFlags() | Font::FontStyleFlags::bold);
	g.setFont(f);

    g.setColour (findColour (DirectoryContentsDisplayComponent::textColourId));
    g.setFont (height * 0.7f);

	
	int xText = x + 2*iconhborder;
	/*
    if (width > 450 && ! isDirectory)
    {
        const int sizeX = filenameWidth;
        const int dateX = roundToInt (width * 0.8f);

        g.drawFittedText (filename,
                          xText, 0, sizeX - xText, height,
                          Justification::centredLeft, 1);

        g.setFont (height * 0.5f);
        g.setColour (Colours::darkgrey);

        if (! isDirectory)
        {
            g.drawFittedText (fileSizeDescription,
                              sizeX, 0, dateX - sizeX - 8, height,
                              Justification::centredRight, 1);

            g.drawFittedText (fileTimeDescription,
                              dateX, 0, width - 8 - dateX, height,
                              Justification::centredRight, 1);
        }
    }
    else*/
    {
        g.drawFittedText (filename,
                          xText, 0, width - xText, height,
                          Justification::centredLeft, 1);

    }
}
};
//==============================================================================
/**
    This is the top-level window that we'll pop up. Inside it, we'll create and
    show a component from the MainComponent.cpp file (you can open this file using
    the Jucer to edit it).
*/
class HelloWorldWindow  : public DocumentWindow
{
public:
    //==============================================================================
    HelloWorldWindow(const String& commandLine)
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

		
		switchFullScreen();

        // And show it!
        setVisible (true);
    }

    ~HelloWorldWindow()
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
        // HelloWorldWindow object will be deleted by the JUCEHelloWorldApplication class.
        JUCEApplication::quit();
    }
	void switchFullScreen()
	{
		Desktop& desktop = Desktop::getInstance();

		if (desktop.getKioskModeComponent() == nullptr)
			desktop.setKioskModeComponent (getTopLevelComponent());
		else
			desktop.setKioskModeComponent (nullptr);
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
        helloWorldWindow = new HelloWorldWindow(commandLine);

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
        return "Launch menu";
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
    ScopedPointer<HelloWorldWindow> helloWorldWindow;
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