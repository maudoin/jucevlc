
#include "Thumbnailer.h"
#include "ImageCatalog.h"
#include <math.h>
#include "FileSorter.h"
#include "Icons.h"
#include <boost/format.hpp>
#include <boost/regex.hpp>

#define thunmnailW 300
#define thunmnailH 200
#define thumbnailCount 2

#define THUMB_TIME_POS_PERCENT 20
#define THUMBNAIL_GENERATION_TIMEOUT 5000

Thumbnailer::Thumbnailer(ImageCatalog& imageCatalogToFeed)
	:img(new juce::Image(juce::Image::RGB, thunmnailW, thunmnailH*thumbnailCount, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
	,m_imageCatalogToFeed(imageCatalogToFeed)
{

	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);

}

Thumbnailer::~Thumbnailer()
{
	//let vlc threads finish
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		currentThumbnailIndex=thumbnailCount;
		currentThumbnail = juce::File::nonexistent;
	}

}


bool Thumbnailer::busyOn(juce::File const& f)const
{
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
	return currentThumbnail == f;
}

bool Thumbnailer::startGeneration(juce::File const& entryToCreate, juce::File const& f)
{
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		currentThumbnail = entryToCreate;//nothing writen at destination "f"
		currentThumbnailIndex = 0;//no image ready
		startTime = juce::Time::currentTimeMillis();
	}
	if( f != juce::File::nonexistent)
    {
        /*
      IGraphBuilder graphbuilder = (IGraphBuilder)new FilterGraph();
      ISampleGrabber samplegrabber = (ISampleGrabber) new SampleGrabber();
      graphbuilder.AddFilter((IBaseFilter)samplegrabber, "samplegrabber");

      AMMediaType mt = new AMMediaType();
      mt.majorType = MediaType.Video;
      mt.subType = MediaSubType.RGB24;
      mt.formatType = FormatType.VideoInfo;
      samplegrabber.SetMediaType(mt);

      int hr = graphbuilder.RenderFile("C:\\aaa\\c4.wmv", null);

      IMediaEventEx mediaEvt = (IMediaEventEx)graphbuilder;
      IMediaSeeking mediaSeek = (IMediaSeeking)graphbuilder;
      IMediaControl mediaCtrl = (IMediaControl)graphbuilder;
      IBasicAudio basicAudio = (IBasicAudio)graphbuilder;
      IVideoWindow videoWin = (IVideoWindow)graphbuilder;

      basicAudio.put_Volume(-10000);
      videoWin.put_AutoShow(OABool.False);

      samplegrabber.SetOneShot(true);
      samplegrabber.SetBufferSamples(true);

      long d = 0;
      mediaSeek.GetDuration(out d);
      long numSecs = d / 10000000;

      long secondstocapture = (long)(numSecs * 0.10f);


      DsLong rtStart, rtStop;
      rtStart = new DsLong(secondstocapture * 10000000);
      rtStop = rtStart;
      mediaSeek.SetPositions(rtStart, AMSeekingSeekingFlags.AbsolutePositioning, rtStop, AMSeekingSeekingFlags.AbsolutePositioning);

      mediaCtrl.Run();
      EventCode evcode;
      mediaEvt.WaitForCompletion(-1, out evcode);

      VideoInfoHeader videoheader = new VideoInfoHeader();
      AMMediaType grab = new AMMediaType();
      samplegrabber.GetConnectedMediaType(grab);
      videoheader =
      (VideoInfoHeader)Marshal.PtrToStructure(grab.formatPtr,
      typeof(VideoInfoHeader));


      int width = videoheader.SrcRect.right;
      int height = videoheader.SrcRect.bottom;
      Bitmap b = new Bitmap(width, height, PixelFormat.Format24bppRgb);

      uint bytesPerPixel = (uint)(24 >> 3);
      uint extraBytes = ((uint)width * bytesPerPixel) % 4;
      uint adjustedLineSize = bytesPerPixel * ((uint)width + extraBytes);
      uint sizeOfImageData = (uint)(height) * adjustedLineSize;


      BitmapData bd1 = b.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.ReadWrite, PixelFormat.Format24bppRgb);
      int bufsize = (int)sizeOfImageData;
      int n = samplegrabber.GetCurrentBuffer(ref bufsize, bd1.Scan0);
      b.UnlockBits(bd1);
      b.RotateFlip(RotateFlipType.RotateNoneFlipY);
      b.Save("C:\\aaa\\out.bmp");
      Marshal.ReleaseComObject(graphbuilder);
      Marshal.ReleaseComObject(samplegrabber);
        ffmpeg->open(f.getFullPathName().toUTF8().getAddress());
        ffmpeg->seek(((1+currentThumbnailIndex)*(THUMB_TIME_POS_PERCENT-1)*ffmpeg->duration()/100));
        ffmpeg->play();*/
    }
}

bool Thumbnailer::newImageReady()
{
    const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
	return( currentThumbnailIndex>=thumbnailCount && currentThumbnail != juce::File::nonexistent);
}
/*
interface ISampleGrabberCB  : public IUnknown
{
    virtual STDMETHODIMP SampleCB (double, IMediaSample*) = 0;
    virtual STDMETHODIMP BufferCB (double, BYTE*, long) = 0;
};

    class GrabberCallback   : public ComBaseClassHelperBase <ISampleGrabberCB>
    {
    public:
        GrabberCallback (DShowCameraDeviceInteral& cam)
            : ComBaseClassHelperBase <ISampleGrabberCB> (0), owner (cam) {}

        JUCE_COMRESULT QueryInterface (REFIID refId, void** result)
        {
            if (refId == IID_ISampleGrabberCB)
                return castToType <ISampleGrabberCB> (result);

            return ComBaseClassHelperBase<ISampleGrabberCB>::QueryInterface (refId, result);
        }

        STDMETHODIMP SampleCB (double, IMediaSample*)  { return E_FAIL; }

        STDMETHODIMP BufferCB (double time, BYTE* buffer, long bufferSize)
        {
            owner.consumeFrame (buffer);
            return S_OK;
        }

    private:
        DShowCameraDeviceInteral& owner;

        JUCE_DECLARE_NON_COPYABLE (GrabberCallback)
    };
/*
void Thumbnailer::consumeFrame(BYTE* buffer)
{
    int processedThumbnailIndex;
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		processedThumbnailIndex = currentThumbnailIndex;
	}

	//imgCriticalSection.enter();

        {
            const int lineStride = width * 3;
            const ScopedLock sl (imageSwapLock);

            {
                const Image::BitmapData destData (loadingImage, 0, 0, width, height, Image::BitmapData::writeOnly);

                for (int i = 0; i < height; ++i)
                    memcpy (destData.getLinePointer ((height - 1) - i),
                            buffer + lineStride * i,
                            lineStride);
            }
        }
	//if(ptr)
	//{
	//	memcpy(ptr->getLinePointer(std::min(processedThumbnailIndex,thumbnailCount-1)*thunmnailH), f.data(), f.size());
	//}
	//{
	//	const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
	//	currentThumbnailIndex++;//prev image ready
	//}
	//juce::Image copy = img->createCopy();
	//imgCriticalSection.exit();


	juce::File processedThumbnail;
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		processedThumbnail = currentThumbnail;//done writing (and where)
		processedThumbnailIndex = currentThumbnailIndex;//images ready?
	}
	if( processedThumbnailIndex>=thumbnailCount)
	{
		//images ready and not done writing
		m_imageCatalogToFeed.storeImageInCacheAndSetChanged(processedThumbnail, copy);
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		currentThumbnail = juce::File::nonexistent;//done writing
	}
	else
	{
	    //next snapshot
        //ffmpeg->seek(((1+currentThumbnailIndex)*(THUMB_TIME_POS_PERCENT-1)*ffmpeg->duration()/100));
        //ffmpeg->play();
	}

}
*/
