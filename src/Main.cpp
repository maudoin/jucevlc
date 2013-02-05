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

Typeface* loadFont( String inPath)
{
	File fontFile;
	if (File::isAbsolutePath (inPath))
	{
		fontFile = File(inPath);
	}
	else
	{
		fontFile = File::getCurrentWorkingDirectory().getChildFile(inPath);
	}

	FileInputStream ins(fontFile);
	return (new CustomTypeface (ins));

}

class LnF : public OldSchoolLookAndFeel
{
	ScopedPointer<Typeface> cFont;
public:
	LnF()
	{
		setColour(DirectoryContentsDisplayComponent::textColourId, Colours::white);
		
		cFont = loadFont( "ForgottenFuturistShadow.bin");
	}
		const Typeface::Ptr getTypefaceForFont (const Font &font)
		{
			if (cFont)
			{
				return (cFont);
			}
			else
			{
				return (font.getTypeface());
			}
		}
	const Font getFontForTextButton (TextButton& button)
	{
		Font f = LookAndFeel::getFontForTextButton(button);
		f.setHeight(24);
		return f;
	}
	
    virtual void drawLinearSlider (Graphics& g,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const Slider::SliderStyle style,
                                   Slider& slider)
	{
		
		LookAndFeel::drawLinearSlider (g,
                                   x, y,
                                   width, height,
                                   sliderPos,
                                   minSliderPos,
                                   maxSliderPos,
                                   style,
                                   slider);
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
	f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
	f.setStyleFlags(Font::FontStyleFlags::plain);
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
class HelloWorldWindow  : public DocumentWindow , public juce::KeyListener
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

		
		addKeyListener(this);
		switchFullScreen();

        // And show it!
        setVisible (true);
    }

    virtual ~HelloWorldWindow()
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


void serializeFont(String fontName, String out, uint32 glyphCount=256)
{
	
	File fontFile;
	if (File::isAbsolutePath (out))
	{
		fontFile = File(out);
	}
	else
	{
		fontFile = File::getCurrentWorkingDirectory().getChildFile(out);
	}
	fontFile.replaceWithData (0,0);
		

	if (!fontFile.hasWriteAccess ())
	{
		printf ("initialise ERROR can't write to destination file: %s\n", fontFile.getFullPathName().toUTF8());
		return;
	}

	if (fontName == String::empty)
	{
		printf ("initialise ERROR no font name given\n");
		return;
	}


	printf ("Fserialize::serializeFont looking for font in system list [%s]\n", fontName.toUTF8());
	
	Array <Font> systemFonts;
	Font::findFonts (systemFonts);
	for (int i=0; i<systemFonts.size(); i++)
	{
		if (systemFonts[i].getTypeface()->getName() == fontName)
		{
			CustomTypeface customTypefacePlain;
			customTypefacePlain.setCharacteristics(systemFonts[i].getTypefaceName(), systemFonts[i].getAscent(),
                                      systemFonts[i].isBold(), systemFonts[i].isItalic(), ' ');

			customTypefacePlain.addGlyphsFromOtherTypeface (*(systemFonts[i].getTypeface()), 0, glyphCount);
			
			FileOutputStream streamPlain(fontFile);
			customTypefacePlain.writeToStream (streamPlain);
		}
	}

	printf ("Fserialize::serializeFont finished\n");

}

	
static juce::JUCEApplicationBase* juce_CreateApplication() { return new JUCEHelloWorldApplication(); } 
extern "C" JUCE_MAIN_FUNCTION 
{ 
	serializeFont("Forgotten Futurist Shadow", "ForgottenFuturistShadow.bin.new");


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