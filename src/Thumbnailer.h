#ifndef THUMBNAILER_H
#define THUMBNAILER_H


#include "AppConfig.h"
#include "juce.h"
#include "FFMpegWrapper.h"
#include <string>
#include <map>

class ImageCatalog;
//==============================================================================
class Thumbnailer : public FFMpegHandler
{
protected:
    juce::CriticalSection imgCriticalSection;
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
	juce::ScopedPointer<FFMpegWrapper> ffmpeg;

    juce::CriticalSection imgStatusCriticalSection;
	juce::File currentThumbnail;
	int currentThumbnailIndex;
	juce::int64 startTime;
	ImageCatalog& m_imageCatalogToFeed;

public:
	Thumbnailer(ImageCatalog& imageCatalogToFeed);
	virtual ~Thumbnailer();

	//return false is file does not support generation
	bool startGeneration(juce::File const& entryToCreate, juce::File const& f);
	//return true if an image was generated
	bool newImageReady();

	bool busyOn(juce::File const& f)const;


    void consumeFrame(FrameRead &f)override;

};
//==============================================================================

#endif //THUMBNAILER_H
