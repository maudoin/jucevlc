
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
	vlc = new VLCWrapper();
	vlc->SetBufferFormat(thunmnailW, thunmnailH, ptr->lineStride);
	vlc->SetDisplayCallback(this);
	vlc->SetAudioCallback(this);
	vlc->SetEventCallBack(this);

}

Thumbnailer::~Thumbnailer()
{
	//let vlc threads finish
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		thumbTimeOK = true;
		currentThumbnailIndex=thumbnailCount;
		currentThumbnail = juce::File::nonexistent;
	}

	vlc->Stop();
	vlc->clearPlayList();
	while(vlc->getCurrentPlayList().size()>0 ||vlc->isPlaying() )
	{
		//busy 
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
		thumbTimeOK = false;//current image at wrong time
		currentThumbnailIndex = 0;//no image ready
		startTime = juce::Time::currentTimeMillis();
	}
	vlc->addPlayListItem(f.getFullPathName().toUTF8().getAddress());
	vlc->play();
	vlc->setAudioTrack(-1);
	vlc->Mute();
	if(vlc->isSeekable())
	{
		//start playing to generate thumbnail
		return true;
	}
	else
	{
		//cancel thumbnail && store nothing
		vlc->clearPlayList();
		vlc->Stop();
		currentThumbnail == juce::File::nonexistent;
		m_imageCatalogToFeed.storeImageInCache(f, juce::Image::null);
		return false;
	}
}
void Thumbnailer::vlcTimeChanged(int64_t newTime)
{
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
	thumbTimeOK = newTime > ((1+currentThumbnailIndex)*(THUMB_TIME_POS_PERCENT-1)*vlc->GetLength()/100);//current image at right time
}
bool Thumbnailer::workStep()
{
	if(vlc->getCurrentPlayList().size()>0 || vlc->isPlaying())
	{
		bool done;
		{
			const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
			done = currentThumbnail == juce::File::nonexistent || ((juce::Time::currentTimeMillis() - startTime)>THUMBNAIL_GENERATION_TIMEOUT);
		}
		if(done)
		{
			//stop outside vlc callbacks
			vlc->clearPlayList();
			vlc->Stop();
			return true;
		}
		else
		{
			const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
			if(vlc->isPlaying() && vlc->GetTime() < ((1+currentThumbnailIndex)*(THUMB_TIME_POS_PERCENT-1)*vlc->GetLength()/100))
			{
				vlc->SetTime((int64_t)((1+currentThumbnailIndex)*THUMB_TIME_POS_PERCENT*vlc->GetLength()/100));
				thumbTimeOK = false;//current image at wrong time
			}
		}
		return true;//busy but no image done
	}
	
	return false;
}


void *Thumbnailer::vlcLock(void **p_pixels)
{
	int processedThumbnailIndex;
	int startingThumbTimeOK;
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		processedThumbnailIndex = currentThumbnailIndex;
		startingThumbTimeOK = thumbTimeOK;//current image at right time
	}

	imgCriticalSection.enter();
	if(ptr)
	{
		*p_pixels = ptr->getLinePointer(std::min(processedThumbnailIndex,thumbnailCount-1)*thunmnailH);
	}
	if(startingThumbTimeOK)
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		currentThumbnailIndex++;//prev image ready
		thumbTimeOK = false;//current image at wrong time
	}
	return NULL; /* picture identifier, not needed here */
}

void Thumbnailer::vlcUnlock(void *id, void *const *p_pixels)
{
	
	juce::Image copy = img->createCopy();
	imgCriticalSection.exit();
			
	juce::File processedThumbnail;
	int processedThumbnailIndex;
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		processedThumbnail = currentThumbnail;//done writing (and where)
		processedThumbnailIndex = currentThumbnailIndex;//images ready?
	}
	if( processedThumbnailIndex>=thumbnailCount && processedThumbnail != juce::File::nonexistent)
	{
		//images ready and not done writing
		m_imageCatalogToFeed.storeImageInCacheAndSetChanged(processedThumbnail, copy);
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		currentThumbnail = juce::File::nonexistent;//done writing
	}
	jassert(id == NULL); /* picture identifier, not needed here */
}

void Thumbnailer::vlcDisplay(void *id)
{
	jassert(id == NULL);
}