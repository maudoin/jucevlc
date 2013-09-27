#ifndef IMAGECATALOG_H
#define IMAGECATALOG_H


#include "AppConfig.h"
#include "juce.h"
#include "ImageCatalogCache.h"
#include <string>
#include <map>

//==============================================================================
class ImageCatalog
{
protected:
	juce::CriticalSection m_imagesMutex;
	std::map<std::string, juce::Image> m_iconPerFile;
	ImageCatalogCache m_cache;
	bool m_changedSinceLastSave;
	
	
public:
	ImageCatalog();
	virtual ~ImageCatalog();
	
	void storeImageInCacheAndSetChanged(juce::File const& f, juce::Image const& i = juce::Image::null);
	void storeImageInCache(juce::File const& path, juce::Image const& i = juce::Image::null);
	juce::Image get(juce::File const& file);
	void maySaveCache();
	
	void preload(juce::Array<std::pair<juce::File, bool> > & files, juce::int64 maxTimeMs);


};
//==============================================================================

#endif //IMAGECATALOG_H