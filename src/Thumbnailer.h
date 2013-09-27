#ifndef THUMBNAILER_H
#define THUMBNAILER_H


#include "AppConfig.h"
#include "juce.h"
#include "VLCWrapper.h"
#include <string>
#include <map>

class ImageCatalog;
//==============================================================================
class Thumbnailer : public DisplayCallback, public EventCallBack, public AudioCallback
{
protected:
    juce::CriticalSection imgCriticalSection;
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
	juce::ScopedPointer<VLCWrapper> vlc;
	
    juce::CriticalSection imgStatusCriticalSection;
	juce::File currentThumbnail;
	int currentThumbnailIndex;
	bool thumbTimeOK;
	juce::int64 startTime;
	ImageCatalog& m_imageCatalogToFeed;
	
public:
	Thumbnailer(ImageCatalog& imageCatalogToFeed);
	virtual ~Thumbnailer();
	
	//return false is file does not support generation
	bool startGeneration(juce::File const& entryToCreate, juce::File const& f);
	//return true if an image was generated
	bool workStep();

	bool busyOn(juce::File const& f)const;

	
	void *vlcLock(void **p_pixels);
	void vlcUnlock(void *id, void *const *p_pixels);
	void vlcDisplay(void *id);

	void vlcTimeChanged(int64_t newTime);
	void vlcPaused() {};
	void vlcStarted() {};
	void vlcStopped() {};
	
	void vlcAudioPlay(const void *samples, unsigned count, int64_t pts) {};
	void vlcAudioPause(int64_t pts) {};
	void vlcAudioResume(int64_t pts) {};
	void vlcAudioFush(int64_t pts) {};
	void vlcAudioDrain() {};

};
//==============================================================================

#endif //THUMBNAILER_H