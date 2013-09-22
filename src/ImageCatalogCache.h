#ifndef IMAGECATALOGCACHE_H
#define IMAGECATALOGCACHE_H


#include "juce.h"
#include <string>
#include <map>
class ImageCatalog;
//==============================================================================
class ImageCatalogCache
{
protected:
	juce::CriticalSection m_mutex;
	juce::File m_cacheFile;
	typedef std::pair<juce::int64, juce::int64> AddressAndLastUse;
	typedef std::map<std::string, AddressAndLastUse> AddressAndLastUsePerFileMap;
	AddressAndLastUsePerFileMap m_imagesCacheAdresses;

	
	
public:
	ImageCatalogCache(juce::File const& f);
	~ImageCatalogCache();
	//save full cache
	void saveCache(std::map<std::string, juce::Image> const& newEntries);
	//save full cache
	void loadCacheIndex();
	//save full cache
	juce::Image loadCachedFile(std::string const& f);

};
//==============================================================================

#endif //IMAGECATALOGCACHE_H