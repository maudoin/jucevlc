#ifndef IMAGECATALOGCACHE_H
#define IMAGECATALOGCACHE_H


#include "juce.h"
#include <string>
#include <map>

//==============================================================================
class ImageCatalogCache
{
protected:
	std::map<std::string, juce::Image> m_iconPerFile;

	
	
public:
	ImageCatalogCache();
	~ImageCatalogCache();
	//save full cache
	void saveCache();
	//save full cache
	void loadCacheIndex();
	//save full cache
	juce::Image loadCachedFile(juce::File const& f);

};
//==============================================================================

#endif //IMAGECATALOGCACHE_H