
#include "AppConfig.h"
#include "juce.h"
#include "VideoComponent.h"
#include "glffmpeg.h"

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
//#define FFMPEGTEST
#ifdef FFMPEGTEST
        OpenGLFFMpegComponent* openGLFFMpegComponent = new OpenGLFFMpegComponent();
        openGLFFMpegComponent->addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar);
        openGLFFMpegComponent->setSize(800, 600);
        openGLFFMpegComponent->setVisible (true);
        openGLFFMpegComponent->open("data/test7.mp4");
        openGLFFMpegComponent->seek(19.);
        openGLFFMpegComponent->play();
#endif //FFMPEGTEST
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
};
#ifdef JUCE_64BIT
#include <modules\juce_core\native/juce_BasicNativeHeaders.h>
#endif

//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (VLCApplication)
