#include "glffmpeg.h"



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
	"uniform mat4 mvpMatrix;\n"
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

class OpenGLFFMpegPrivate
{
        AVFormatContext *fmt_ctx;
        int stream_idx;
        AVStream *video_stream;
        AVCodecContext *codec_ctx;
        AVCodec *decoder;
        AVPacket *packet;
        AVFrame *av_frame;
        AVFrame *gl_frame;
        double durationMSec, frameRate;
        struct SwsContext *conv_ctx;
        GLuint vao;
        GLuint vert_buf;
        GLuint elem_buf;
        GLuint frame_tex;
        GLuint program;
        GLuint attribs[2];
        GLuint uniforms[2];

    public:
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

    // initialize the app data structure
    OpenGLFFMpegPrivate() {
        initializeAppData();
    }
    ~OpenGLFFMpegPrivate()
    {

        avformat_close_input(&fmt_ctx);

        // clean up
        clearAppData();
    }
    void initializeAppData(){
        this->fmt_ctx = NULL;
        this->stream_idx = -1;
        this->video_stream = NULL;
        this->codec_ctx = NULL;
        this->decoder = NULL;
        this->av_frame = NULL;
        this->gl_frame = NULL;
        this->conv_ctx = NULL;
    }
    // clean up the app data structure
    void clearAppData() {
        if (this->av_frame) av_free(this->av_frame);
        if (this->gl_frame) av_free(this->gl_frame);
        if (this->packet) av_free(this->packet);
        if (this->codec_ctx) avcodec_close(this->codec_ctx);
        if (this->fmt_ctx) avformat_free_context(this->fmt_ctx);
        glDeleteVertexArrays(1, &this->vao);
        glDeleteBuffers(1, &this->vert_buf);
        glDeleteBuffers(1, &this->elem_buf);
        glDeleteTextures(1, &this->frame_tex);
        initializeAppData();
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

    int open(char *filename) {

        // initialize libav
        av_register_all();
        avformat_network_init();

        // initialize custom data structure
        initializeAppData();

        // open video
        if (avformat_open_input(&this->fmt_ctx, filename, NULL, NULL) < 0) {
            std::cout << "failed to open input" << std::endl;
            clearAppData();
            return -1;
        }

        // find stream info
        if (avformat_find_stream_info(this->fmt_ctx, NULL) < 0) {
            std::cout << "failed to get stream info" << std::endl;
            clearAppData();
            return -1;
        }

        // dump debug info
        av_dump_format(this->fmt_ctx, 0, filename, 0);

        // find the video stream
        for (unsigned int i = 0; i < this->fmt_ctx->nb_streams; ++i)
        {
            if (this->fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                this->stream_idx = i;
                break;
            }
        }

        if (this->stream_idx == -1)
        {
            std::cout << "failed to find video stream" << std::endl;
            clearAppData();
            return -1;
        }

        this->video_stream = this->fmt_ctx->streams[this->stream_idx];
        this->codec_ctx = this->video_stream->codec;

        // find the decoder
        this->decoder = avcodec_find_decoder(this->codec_ctx->codec_id);
        if (this->decoder == NULL)
        {
            std::cout << "failed to find decoder" << std::endl;
            clearAppData();
            return -1;
        }

/////////////////////////

    /**
      * This is the fundamental unit of time (in seconds) in terms
      * of which frame timestamps are represented. For fixed-fps content,
      * timebase should be 1/framerate and timestamp increments should be
      * identically 1.
      * - encoding: MUST be set by user.
      * - decoding: Set by libavcodec.
      */
    AVRational avr = this->codec_ctx->time_base;
    /**
     * For some codecs, the time base is closer to the field rate than the frame rate.
     * Most notably, H.264 and MPEG-2 specify time_base as half of frame duration
     * if no telecine is used ...
     *
     * Set to time_base ticks per frame. Default 1, e.g., H.264/MPEG-2 set it to 2.
     */
    durationMSec = static_cast<double>(this->video_stream->duration) * static_cast<double>(this->codec_ctx->ticks_per_frame) / static_cast<double>(avr.den);
    // calculate frame rate based on time_base and ticks_per_frame
    frameRate = static_cast<double>(avr.den) / static_cast<double>(avr.num * this->codec_ctx->ticks_per_frame);

/////////////////////////
        // open the decoder
        if (avcodec_open2(this->codec_ctx, this->decoder, NULL) < 0)
        {
            std::cout << "failed to open codec" << std::endl;
            clearAppData();
            return -1;
        }

        // allocate the video frames
        this->av_frame = av_frame_alloc();
        this->gl_frame = av_frame_alloc();
        int size = avpicture_get_size(PIX_FMT_RGB24, this->codec_ctx->width,
            this->codec_ctx->height);
        uint8_t *internal_buffer = (uint8_t *)av_malloc(size * sizeof(uint8_t));
        avpicture_fill((AVPicture *)this->gl_frame, internal_buffer, PIX_FMT_RGB24,
            this->codec_ctx->width, this->codec_ctx->height);
        this->packet = (AVPacket *)av_malloc(sizeof(AVPacket));

        // initialize opengl
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_TEXTURE_2D);

        // initialize shaders
        if (!buildProgram()) {
            std::cout << "failed to initialize shaders" << std::endl;
            clearAppData();
            return -1;
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->codec_ctx->width, this->codec_ctx->height,
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glUniform1i(this->uniforms[FRAME_TEX], 0);

        GLfloat mvp[16];
        ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f ,mvp);
        glUniformMatrix4fv(this->uniforms[MVP_MATRIX], 1, GL_FALSE, mvp);

        bool running = true;
    }

    // read a video frame
    bool seek(double pos) {

        int64_t seek_pos = (int64_t)(pos * AV_TIME_BASE); // Seconds
        bool backseek = seek_pos <= (int64_t)(this->packet->pts*AV_TIME_BASE);
        int64_t seek_target = av_rescale_q(seek_pos, AV_TIME_BASE_Q, this->video_stream->time_base);
        int flags = seek_pos ? AVSEEK_FLAG_BACKWARD : 0;
        av_seek_frame(this->fmt_ctx,this->stream_idx, seek_target, flags);
    }

    // read a video frame
    bool readFrame(int &frame_finished) {


        frame_finished = 0;
        do {
            if (av_read_frame(this->fmt_ctx, this->packet) < 0) {
                av_free_packet(this->packet);
                return false;
            }

            if (this->packet->stream_index == this->stream_idx) {
                //int frame_finished = 0;

                if (avcodec_decode_video2(this->codec_ctx, this->av_frame, &frame_finished,
                    this->packet) < 0) {
                    av_free_packet(this->packet);
                    return false;
                }

                if (frame_finished) {
                    if (!this->conv_ctx) {
                        this->conv_ctx = sws_getContext(this->codec_ctx->width,
                            this->codec_ctx->height, this->codec_ctx->pix_fmt,
                            this->codec_ctx->width, this->codec_ctx->height, PIX_FMT_RGB24,
                            SWS_BICUBIC, NULL, NULL, NULL);
                    }

                    sws_scale(this->conv_ctx, this->av_frame->data, this->av_frame->linesize, 0,
                        this->codec_ctx->height, this->gl_frame->data, this->gl_frame->linesize);

                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->codec_ctx->width,
                        this->codec_ctx->height, GL_RGB, GL_UNSIGNED_BYTE,
                        this->gl_frame->data[0]);
                }
            }

            av_free_packet(this->packet);
        } while (this->packet->stream_index != this->stream_idx);

        return true;
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

        this->uniforms[MVP_MATRIX] = glGetUniformLocation(this->program, "mvpMatrix");
        this->uniforms[FRAME_TEX] = glGetUniformLocation(this->program, "frameTex");

        this->attribs[VERTICES] = glGetAttribLocation(this->program, "vertex");
        this->attribs[TEX_COORDS] = glGetAttribLocation(this->program, "texCoord0");

        return true;
    }

    // draw frame in opengl context
    void drawFrame() {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(this->program);

        glBindTexture(GL_TEXTURE_2D, this->frame_tex);
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
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    openGLContext.setContinuousRepainting (true);

    ffmpeg = new OpenGLFFMpegPrivate();
}

OpenGLFFMpegComponent::~OpenGLFFMpegComponent()
{
    openGLContext.detach();
    delete ffmpeg;
}

bool OpenGLFFMpegComponent::open(char*s)
{
    openGLContext.makeActive();
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(1); // or handle the error in a nicer way
    }
    if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(1); // or handle the error in a nicer way
    }
    ffmpeg->open(s);
}
void OpenGLFFMpegComponent::play()
{
}
void OpenGLFFMpegComponent::newOpenGLContextCreated()
{
	open("data/test7.mp4");
	ffmpeg->seek(19.);

    int frame_finished = 0;
    do
    {
        ffmpeg->readFrame(frame_finished);
    }
    while(!frame_finished);

}

void OpenGLFFMpegComponent::openGLContextClosing()
{
    ffmpeg->clearAppData();
}

// This is a virtual method in OpenGLRenderer, and is called when it's time
// to do your GL rendering.
void OpenGLFFMpegComponent::renderOpenGL()
{
    openGLContext.makeActive();
    //jassert (OpenGLHelpers::isContextActive());

    glViewport( 0, 0, getWidth(), getHeight() );

    ffmpeg->drawFrame();
/*
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport( 0, 0, getWidth(), getHeight() );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
*/
}
