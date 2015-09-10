#ifndef FFMPEG_WRAPPER_H
#define FFMPEG_WRAPPER_H

#include "AppConfig.h"
#include "juce.h"
class DecoderPrivate;
class QueuedFrame;
class VideoDataExchange;

class FrameRead
{
    VideoDataExchange& xchange;
    QueuedFrame& protectedData;
    juce::CriticalSection::ScopedLockType l;
public:
    FrameRead(VideoDataExchange& x);
    ~FrameRead();
    int width();
    int height();
    int size();
    uint8_t * data();
};

class VideoDataExchange
{

    juce::CriticalSection mutex;
    juce::WaitableEvent queueContentEvent;//frame content added/removed
    QueuedFrame* frameQueue;
    int queueSize, readIndex, writeIndex, fillCount;

    friend class FrameRead;
    friend class FrameWrite;
public:
    VideoDataExchange(int newQueueSize);
    ~VideoDataExchange();
    bool isEmpty()const;
    void add(FrameRead & f);
private:
    QueuedFrame& getOrWaitFrameToRead();

    QueuedFrame& getOrWaitFrameToWrite();


};



class FrameWrite
{
    VideoDataExchange& xchange;
    QueuedFrame& protectedData;
    juce::CriticalSection::ScopedLockType l;
public:
    FrameWrite(VideoDataExchange& x,
               int width, int height, int size);
    ~FrameWrite();
    uint8_t * data();

    static void AddFrameToExchange(VideoDataExchange& x, FrameRead &r);
};

class FFMpegHandler
{
public:
    virtual ~FFMpegHandler(){};
    virtual void consumeFrame(FrameRead&) = 0;
};

class FFMpegWrapper
{
    VideoDataExchange* xchange;
    DecoderPrivate* decoder;
    FFMpegHandler& renderer;
public:
    FFMpegWrapper(FFMpegHandler& );

    ~FFMpegWrapper();

    bool open(char*);
    bool seek(double time);
    void play();
};
#endif // GLFFMPEG_H
