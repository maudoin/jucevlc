#include "VideoComponent.h"

#include <JuceHeader.h>

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
    void initialise (const juce::String& /*commandLine*/)
    {
        // create the main window...
        window = std::make_unique<VideoComponent>();
    }

    void shutdown()
    {
        //  clear-up app's resources..
        window.reset();
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

    void anotherInstanceStarted (const juce::String& /*commandLine*/)
    {
    }

private:
    std::unique_ptr<VideoComponent> window;
};
#ifdef JUCE_64BIT
#include <juce_core/native/juce_BasicNativeHeaders.h>
#endif

//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (VLCApplication)
