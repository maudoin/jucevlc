#include "ffmpegWrapper.h"



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


#define FRAME_QUEUE_LENGTH 100

class QueuedFrame
{

    juce::CriticalSection mutex;
    int currentWidth, currentHeight, currentSize;
    uint8_t *internal_buffer;
public:

    friend class FrameRead;
    friend class FrameWrite;

    QueuedFrame()
    : currentWidth(1)
    , currentHeight(1)
    , currentSize(1)
    , internal_buffer(NULL)
    {
        internal_buffer = new uint8_t[1];
    }
    ~QueuedFrame()
    {
        juce::CriticalSection::ScopedLockType l(mutex);
        delete[] internal_buffer;
    }

};
VideoDataExchange::VideoDataExchange(int newQueueSize)
: queueSize(newQueueSize)
, readIndex(0)
, writeIndex(0)
, fillCount(0)
{
    frameQueue = new QueuedFrame[queueSize];
}
VideoDataExchange::~VideoDataExchange()
{
    delete[] frameQueue;
}
bool VideoDataExchange::isEmpty()const
{

    juce::CriticalSection::ScopedLockType l(mutex);
    return fillCount<=0;
}

QueuedFrame& VideoDataExchange::getOrWaitFrameToRead()
{

    juce::CriticalSection::ScopedLockType l(mutex);
    while(fillCount<=0)
    {
        queueContentEvent.wait();
    }
    return frameQueue[readIndex];
}

QueuedFrame& VideoDataExchange::getOrWaitFrameToWrite()
{

    juce::CriticalSection::ScopedLockType l(mutex);
    while(fillCount>=queueSize)
    {
        queueContentEvent.wait();
    }
    return frameQueue[writeIndex];
}

void VideoDataExchange::add(FrameRead & f)
{
    FrameWrite::AddFrameToExchange(*this, f);
}
////////////////////////////////////////

FrameRead::FrameRead(VideoDataExchange& x)
: xchange(x)
, protectedData(x.getOrWaitFrameToRead())
, l(protectedData.mutex)
{
}
FrameRead::~FrameRead()
{
    juce::CriticalSection::ScopedLockType l(xchange.mutex);
    xchange.readIndex++;
    if(xchange.readIndex>=xchange.queueSize)
    {
        xchange.readIndex = 0;
    }
    xchange.fillCount--;
    xchange.queueContentEvent.signal();
}
int FrameRead::width()
{
    return protectedData.currentWidth;
}
int FrameRead::height()
{
    return protectedData.currentHeight;
}
int FrameRead::size()
{
    return protectedData.currentSize;
}
uint8_t * FrameRead::data()
{
    return protectedData.internal_buffer;
}

////////////////////////////////////////

FrameWrite::FrameWrite(VideoDataExchange& x,
               int width, int height, int size)
: xchange(x)
, protectedData(x.getOrWaitFrameToWrite())
, l(protectedData.mutex)
{
    //fill protectedData properly
    if( size != protectedData.currentSize)
    {
        delete[] protectedData.internal_buffer;
        protectedData.internal_buffer = new uint8_t[size];
        protectedData.currentSize = size;
    }
    protectedData.currentWidth = width;
    protectedData.currentHeight = height;

}
FrameWrite::~FrameWrite()
{
    juce::CriticalSection::ScopedLockType l(xchange.mutex);
    xchange.writeIndex++;
    if(xchange.writeIndex>=xchange.queueSize)
    {
        xchange.writeIndex = 0;
    }
    xchange.fillCount++;
    xchange.queueContentEvent.signal();
}
uint8_t * FrameWrite::data()
{
    return protectedData.internal_buffer;
}

void FrameWrite::AddFrameToExchange(VideoDataExchange& x, FrameRead &r)
{
    FrameWrite w(x, r.width(), r.height(), r.size());
    memcpy(w.data(), r.data(), r.size());

}

class DecoderPrivate
{
    VideoDataExchange& xchange;
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
    int  dstStride[3];
    uint8_t * dst[3];
    int  targetW, targetH;


    public:

    // initialize the app data structure
    DecoderPrivate(VideoDataExchange& exchange ,int targetW=-1, int targetH=-1)
    :xchange(exchange)
    ,fmt_ctx(0)
    ,video_stream(0)
    ,codec_ctx(0)
    ,decoder(0)
    ,packet(0)
    ,av_frame(0)
    ,gl_frame(0)
    ,targetW(targetW)
    ,targetH(targetH)
    {
        initializeAppData();
    }
    ~DecoderPrivate()
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
        this->conv_ctx = NULL;
    }
    // clean up the app data structure
    void clearAppData() {
        if (this->av_frame) av_free(this->av_frame);
        if (this->packet) av_free(this->packet);
        if (this->codec_ctx) avcodec_close(this->codec_ctx);
        if (this->fmt_ctx) avformat_free_context(this->fmt_ctx);
        initializeAppData();
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

        this->packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    }

    // read a video frame
    bool seek(double pos) {

        int64_t seek_pos = (int64_t)(pos * AV_TIME_BASE); // Seconds
        bool backseek = seek_pos <= (int64_t)(this->packet->pts*AV_TIME_BASE);
        int64_t seek_target = av_rescale_q(seek_pos, AV_TIME_BASE_Q, this->video_stream->time_base);
        int flags = seek_pos ? AVSEEK_FLAG_BACKWARD : 0;
        av_seek_frame(this->fmt_ctx,this->stream_idx, seek_target, flags);
    }

    double duration () const
    {
        return this->fmt_ctx->duration / (double)AV_TIME_BASE;
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
                    int outW = targetW>0?targetW:this->codec_ctx->width;
                    int outH = targetH>0?targetH:this->codec_ctx->height;
                    if (!this->conv_ctx) {
                        this->conv_ctx = sws_getContext(this->codec_ctx->width,
                            this->codec_ctx->height, this->codec_ctx->pix_fmt,
                            outW,outH,PIX_FMT_RGB24,
                            SWS_BICUBIC, NULL, NULL, NULL);
                    }

                    int size = avpicture_get_size(PIX_FMT_RGB24, outW,outH);
                    FrameWrite f(xchange, outW, outH, size);

                    dstStride[0]= dstStride[1] = dstStride[2] = 3*outW;

                    dst[0]=f.data();
                    dst[1]=f.data()+1;
                    dst[2]=f.data()+2;

                    sws_scale(this->conv_ctx, this->av_frame->data, this->av_frame->linesize, 0,
                        this->codec_ctx->height, dst, dstStride);

                }
            }

            av_free_packet(this->packet);
        } while (this->packet->stream_index != this->stream_idx);

        return true;
    }

};

FFMpegWrapper::FFMpegWrapper(FFMpegHandler& handler,int targetW, int targetH)
:renderer(handler)
{
    xchange = new VideoDataExchange(FRAME_QUEUE_LENGTH);
    decoder = new DecoderPrivate(*xchange,targetW, targetH);
}

FFMpegWrapper::~FFMpegWrapper()
{
    delete decoder;
    delete xchange;// last

}

bool FFMpegWrapper::open(char*s)
{
    return decoder->open(s);
}
bool FFMpegWrapper::seek(double time)
{
    return decoder->seek(time);
}

double FFMpegWrapper::duration () const
{
    return decoder->duration();
}

void FFMpegWrapper::play()
{

    int frame_finished = 0;
    do
    {
        decoder->readFrame(frame_finished);
    }
    while(!frame_finished);

    //!
    {
        FrameRead f(*xchange);
        renderer.consumeFrame(f);
    }
}
