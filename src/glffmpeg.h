#ifndef GLFFMPEG_H
#define GLFFMPEG_H


#include "AppConfig.h"
#include "juce.h"
class OpenGLVideoRenderer;
class FFMpegWrapper;

class OpenGLFFMpegComponent  : public juce::Component//DocumentWindow,//
{
    juce::OpenGLContext openGLContext;
    FFMpegWrapper* wrapper;
    OpenGLVideoRenderer* renderer;
public:
    OpenGLFFMpegComponent();

    ~OpenGLFFMpegComponent();

    bool open(char*);
    bool seek(double time);
    void play();
};
#endif // GLFFMPEG_H
