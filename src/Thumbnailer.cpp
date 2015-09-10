
#include "Thumbnailer.h"
#include "ImageCatalog.h"
#include "ffmpegWrapper.h"
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
	ffmpeg = new FFMpegWrapper(*this, thunmnailW, thunmnailH);

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
        ffmpeg->open(f.getFullPathName().toUTF8().getAddress());
        ffmpeg->seek(((1+currentThumbnailIndex)*(THUMB_TIME_POS_PERCENT-1)*ffmpeg->duration()/100));
        ffmpeg->play();
    }
}

bool Thumbnailer::newImageReady()
{
    const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
	return( currentThumbnailIndex>=thumbnailCount && currentThumbnail != juce::File::nonexistent);
}

void Thumbnailer::consumeFrame(FrameRead &f)
{
    int processedThumbnailIndex;
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		processedThumbnailIndex = currentThumbnailIndex;
	}

	imgCriticalSection.enter();
	if(ptr)
	{
		memcpy(ptr->getLinePointer(std::min(processedThumbnailIndex,thumbnailCount-1)*thunmnailH), f.data(), f.size());
	}
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		currentThumbnailIndex++;//prev image ready
	}


	juce::Image copy = img->createCopy();
	imgCriticalSection.exit();


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
        ffmpeg->seek(((1+currentThumbnailIndex)*(THUMB_TIME_POS_PERCENT-1)*ffmpeg->duration()/100));
        ffmpeg->play();
	}

}
