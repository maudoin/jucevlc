#ifndef THUMBNAILER_H
#define THUMBNAILER_H


#include "AppConfig.h"
#include "juce.h"
#include <string>
#include <map>

class ImageCatalog;
//==============================================================================
class Thumbnailer
{
protected:
    juce::CriticalSection imgCriticalSection;
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;

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
};
//==============================================================================

#endif //THUMBNAILER_H
