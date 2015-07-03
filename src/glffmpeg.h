#ifndef GLFFMPEG_H
#define GLFFMPEG_H

#include "AppConfig.h"
#include "juce.h"
class OpenGLFFMpegPrivate;

    class OpenGLFFMpegComponent  : public juce::Component,//DocumentWindow,//
                        private juce::OpenGLRenderer
    {
        juce::OpenGLContext openGLContext;
        OpenGLFFMpegPrivate* ffmpeg;
    public:
        OpenGLFFMpegComponent();

        ~OpenGLFFMpegComponent();

        void newOpenGLContextCreated() override;

        void openGLContextClosing() override;

        // This is a virtual method in OpenGLRenderer, and is called when it's time
        // to do your GL rendering.
        void renderOpenGL() override;


        bool open(char*);
        void play();
    };
#endif // GLFFMPEG_H
