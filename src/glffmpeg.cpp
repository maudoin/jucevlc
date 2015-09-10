#include "glffmpeg.h"


#include "FFMpegWrapper.h"

#define __STDC_CONSTANT_MACROS
#include <boost/cstdint.hpp>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <sys/time.h>
}

#include <iostream>
#include <fstream>
#include <string>


std::string const vert_shader_source =
	"#version 150\n"
	"in vec3 vertex;\n"
	"in vec2 texCoord0;\n"
	//"uniform mat4 mvpMatrix;\n"
	"out vec2 texCoord;\n"
	"void main() {\n"
	//"	gl_Position = mvpMatrix * vec4(vertex, 1.0);\n"
	"	gl_Position = vec4(vertex, 1.0);\n"
	"	texCoord = texCoord0;\n"
	"}\n";

std::string const frag_shader_source =
	"#version 150\n"
	"uniform sampler2D frameTex;\n"
	"in vec2 texCoord;\n"
	"out vec4 fragColor;\n"
	"void main() {\n"
	"	fragColor = texture(frameTex, texCoord);\n"
	"}\n";

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
class OpenGLVideoRenderer : public juce::OpenGLRenderer,
                    public FFMpegHandler
{
    // attribute indices
    enum {
        VERTICES = 0,
        TEX_COORDS
    };

    // uniform indices
    enum {
        MVP_MATRIX = 0,
        FRAME_TEX
    };
    GLuint vao;
    GLuint vert_buf;
    GLuint elem_buf;
    GLuint program;
    GLuint attribs[2];
    GLuint uniforms[2];
    int frameWidth, frameHeight;
    GLuint frame_tex;

    VideoDataExchange exchange;
public:
    OpenGLVideoRenderer()
    :frameWidth(1)
    ,frameHeight(1)
    ,exchange(1)
    {

    }
    ~OpenGLVideoRenderer()
    {
        //release should be called in the gl context, may not be the case here
    }
    void openGLContextClosing()override//juce::OpenGLRenderer
    {
        glDeleteVertexArrays(1, &this->vao);
        glDeleteBuffers(1, &this->vert_buf);
        glDeleteBuffers(1, &this->elem_buf);

        glDeleteTextures(1, &this->frame_tex);
    }

    template <typename valType>
    void ortho
    (
        valType const & left,
        valType const & right,
        valType const & bottom,
        valType const & top,
        valType const & zNear,
        valType const & zFar,
        valType Result[16]
    )
    {
        Result[0*4+0] = valType(2) / (right - left);
        Result[0*4+1] = 0.;
        Result[0*4+2] = 0.;
        Result[0*4+3] = 0.;
        Result[1*4+0] = 0.;
        Result[1*4+1] = valType(2) / (top - bottom);
        Result[1*4+2] = 0.;
        Result[1*4+3] = 0.;
        Result[2*4+0] = 0.;
        Result[2*4+1] = 0.;
        Result[2*4+2] = - valType(2) / (zFar - zNear);
        Result[2*4+3] = 0.;
        Result[3*4+0] = - (right + left) / (right - left);
        Result[3*4+1] = - (top + bottom) / (top - bottom);
        Result[3*4+2] = - (zFar + zNear) / (zFar - zNear);
        Result[3*4+3] = 0.;
    }
    bool buildShader(std::string const &shader_source, GLuint &shader, GLenum type) {
        int size = shader_source.length();

        shader = glCreateShader(type);
        char const *c_shader_source = shader_source.c_str();
        glShaderSource(shader, 1, (GLchar const **)&c_shader_source, &size);
        glCompileShader(shader);
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            std::cout << "failed to compile shader" << std::endl;
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            char *log = new char[length];
            glGetShaderInfoLog(shader, length, &length, log);
            std::cout << log << std::endl;
            delete[] log;
            return false;
        }

        return true;
    }

    // initialize shaders
    bool buildProgram() {
        GLuint v_shader, f_shader;
        if (!buildShader(vert_shader_source, v_shader, GL_VERTEX_SHADER)) {
            std::cout << "failed to build vertex shader" << std::endl;
            return false;
        }

        if (!buildShader(frag_shader_source, f_shader, GL_FRAGMENT_SHADER)) {
            std::cout << "failed to build fragment shader" << std::endl;
            return false;
        }

        this->program = glCreateProgram();
        glAttachShader(this->program, v_shader);
        glAttachShader(this->program, f_shader);
        glLinkProgram(this->program);
        GLint status;
        glGetProgramiv(this->program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            std::cout << "failed to link program" << std::endl;
            int length;
            glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &length);
            char *log = new char[length];
            glGetShaderInfoLog(this->program, length, &length, log);
            std::cout << log << std::endl;
            delete[] log;
            return false;
        }

        //this->uniforms[MVP_MATRIX] = glGetUniformLocation(this->program, "mvpMatrix");
        this->uniforms[FRAME_TEX] = glGetUniformLocation(this->program, "frameTex");

        this->attribs[VERTICES] = glGetAttribLocation(this->program, "vertex");
        this->attribs[TEX_COORDS] = glGetAttribLocation(this->program, "texCoord0");

        return true;
    }
    void newOpenGLContextCreated() override//juce::OpenGLRenderer
    {
        jassert (juce::OpenGLHelpers::isContextActive());

        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
            return;
        }
        if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
        {
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
            return;
        }
        // initialize opengl
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_TEXTURE_2D);

        // initialize shaders
        if (!buildProgram()) {
            std::cout << "failed to initialize shaders" << std::endl;
            return;
        }

        // initialize renderable
        glGenVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);

        glGenBuffers(1, &this->vert_buf);
        glBindBuffer(GL_ARRAY_BUFFER, this->vert_buf);
        float quad[20] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 0.0f
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
        glVertexAttribPointer(this->attribs[VERTICES], 3, GL_FLOAT, GL_FALSE, 20,
            BUFFER_OFFSET(0));
        glEnableVertexAttribArray(this->attribs[VERTICES]);
        glVertexAttribPointer(this->attribs[TEX_COORDS], 2, GL_FLOAT, GL_FALSE, 20,
            BUFFER_OFFSET(12));
        glEnableVertexAttribArray(this->attribs[TEX_COORDS]);
        glGenBuffers(1, &this->elem_buf);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elem_buf);
        unsigned char elem[6] = {
            0, 1, 2,
            0, 2, 3
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elem), elem, GL_STATIC_DRAW);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &this->frame_tex);
        glBindTexture(GL_TEXTURE_2D, this->frame_tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight,
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glUniform1i(this->uniforms[FRAME_TEX], 0);

        GLfloat mvp[16];
        ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f ,mvp);
        //glUniformMatrix4fv(this->uniforms[MVP_MATRIX], 1, GL_FALSE, mvp);

    }

    void consumeFrame(FrameRead& f)override//FFMpegHandler
    {
        exchange.add(f);
    }
    // draw frame in opengl context
    void renderOpenGL() override//juce::OpenGLRenderer
    {
        if(!exchange.isEmpty())
        {
            FrameRead f(exchange);

            glBindTexture(GL_TEXTURE_2D, this->frame_tex);
            if(frameWidth != f.width() || frameHeight != f.height())
            {

                frameWidth = f.width();
                frameHeight = f.height();
                glDeleteTextures(1, &this->frame_tex);
                glGenTextures(1, &this->frame_tex);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frameWidth, frameHeight,
                    0, GL_RGB, GL_UNSIGNED_BYTE, 0);//f.data()
            }
            //else
            {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameWidth, frameHeight,
                    GL_RGB, GL_UNSIGNED_BYTE, f.data());
            }
            glViewport( 0, 0, f.width(), f.height() );
        }
        jassert (juce::OpenGLHelpers::isContextActive());


        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(this->program);
        glUniform1i(this->uniforms[FRAME_TEX], this->frame_tex);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(this->vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
        glBindVertexArray(0);

        glUseProgram(0);
    }

};

/*
// RENDERING

    int _idxU = mFrameW * mFrameH;
    int _idxV = _idxU + (_idxU / 4);

    // U data
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sampler1Texture);

    GLint sampler1Uniform = glGetUniformLocation(programStandard, "sampler2");

    glUniform1i(sampler1Uniform, 1);

    glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    mFrameW / 2,            // source width
                    mFrameH / 2,            // source height
                    GL_LUMINANCE,
                    GL_UNSIGNED_BYTE,
                    &_frameData[_idxU]);

    // V data
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, sampler2Texture);

    GLint sampler2Uniform = glGetUniformLocation(programStandard, "sampler1");
    glUniform1i(sampler2Uniform, 2);

    glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    mFrameW / 2,            // source width
                    mFrameH / 2,            // source height
                    GL_LUMINANCE,
                    GL_UNSIGNED_BYTE,
                    &_frameData[_idxV]);

    // Y data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sampler0Texture);

    GLint sampler0Uniform = glGetUniformLocation(programStandard, "sampler0");
    glUniform1i(sampler0Uniform, 0);

    glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    mFrameW,            // source width
                    mFrameH,            // source height
                    GL_LUMINANCE,
                    GL_UNSIGNED_BYTE,
                    _frameData);


    //draw RECT
    glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, squareVertices);
    glEnableVertexAttribArray(ATTRIB_VERTEX);

    //ATTRIB_TEXTUREPOSITON
    glVertexAttribPointer(ATTRIB_TEXTUREPOSITON, 2, GL_FLOAT, 0, 0, textureCoords);
    glEnableVertexAttribArray(ATTRIB_TEXTUREPOSITON);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    free(_frameData);

    [(EAGLView *)self.view presentFramebuffer];
    */

    /*
    uniform sampler2D sampler0; // Y Texture Sampler
uniform sampler2D sampler1; // U Texture Sampler
uniform sampler2D sampler2; // V Texture Sampler

varying highp vec2 TexCoordOut;

void main()
{
    highp float y = texture2D(sampler0, TexCoordOut).r;
    highp float u = texture2D(sampler2, TexCoordOut).r - 0.5;
    highp float v = texture2D(sampler1, TexCoordOut).r - 0.5;

    highp float r = y + 1.13983 * v;
    highp float g = y - 0.39465 * u - 0.58060 * v;
    highp float b = y + 2.03211 * u;

    gl_FragColor = vec4(r, g, b, 1.0);
}
    */

    /*
        for (int i = 0, nDataLen = 0; i < 3; i++) {
        int nShift = (i == 0) ? 0 : 1;
        uint8_t *pYUVData = (uint8_t *)_frame->data[i];
        for (int j = 0; j < (mHeight >> nShift); j++) {
            memcpy(&pData->pOutBuffer[nDataLen], pYUVData, (mWidth >> nShift));
            pYUVData += _frame->linesize[i];
            nDataLen += (mWidth >> nShift);
        }
    }
    */

    /*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 texW / 2,
                 texH / 2,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 NULL);

    //: Y Texture
    if (sampler0Texture) glDeleteTextures(1, &sampler0Texture);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &sampler0Texture);
    glBindTexture(GL_TEXTURE_2D, sampler0Texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // This is necessary for non-power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_2D);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 texW,
                 texH,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 NULL);
    */


    //==============================================================================

OpenGLFFMpegComponent::OpenGLFFMpegComponent()
/*:juce::DocumentWindow ("GL",
                      juce::Colours::lightgrey,
                      juce::DocumentWindow::allButtons)*/
{
    //MainAppWindow::getMainAppWindow()->setRenderingEngine (0);

    //setUsingNativeTitleBar (true);
    //setResizable (true, false);

    setOpaque (true);
    renderer = new OpenGLVideoRenderer();
    openGLContext.setRenderer (renderer);
    openGLContext.attachTo (*this);
    openGLContext.setContinuousRepainting (true);

    wrapper = new FFMpegWrapper(*renderer);
}

OpenGLFFMpegComponent::~OpenGLFFMpegComponent()
{
    openGLContext.detach();
    delete renderer;

    delete wrapper;// last

}

bool OpenGLFFMpegComponent::open(char*s)
{
    return wrapper->open(s);
}
bool OpenGLFFMpegComponent::seek(double time)
{
    return wrapper->seek(time);
}
void OpenGLFFMpegComponent::play()
{
    wrapper->play();
}
