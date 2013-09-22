
#include "ImageCatalogCache.h"
#include "ImageCatalog.h"

#define CACHE_EXPIRATION_MS (1000*60*60*24*12) //half month
ImageCatalogCache::ImageCatalogCache(juce::File const& f)
	:m_cacheFile(f)
{
	loadCacheIndex();
}

ImageCatalogCache::~ImageCatalogCache()
{
}
//save full cache
void ImageCatalogCache::saveCache(std::map<std::string, juce::Image> const& newEntries)
{
	juce::int64 now = juce::Time::currentTimeMillis();

	
	typedef std::pair<juce::Image, juce::int64> ImageAndTime;
	typedef std::map<std::string, ImageAndTime> TempImageAndTimePerName;
	TempImageAndTimePerName toDump;
	//new
	for(std::map<std::string, juce::Image>::const_iterator it = newEntries.begin();it != newEntries.end();++it)
	{
		if(it->second.isNull())
		{
			continue;
		}
		toDump.insert(TempImageAndTimePerName::value_type(it->first, ImageAndTime(it->second, now)));
	}
	//+old (but not too old)
	for(AddressAndLastUsePerFileMap::const_iterator it = m_imagesCacheAdresses.begin();it != m_imagesCacheAdresses.end();++it)
	{
		if((now - it->second.second)>CACHE_EXPIRATION_MS)
		{
			//too old
			continue;
		}
		if(newEntries.find(it->first) != newEntries.end())
		{
			//already saved above
			continue;
		}

		//recopy, recent image with their original time
		juce::Image image = loadCachedFile(it->first);
		if(image.isNull())
		{
			continue;
		}
		toDump.insert(TempImageAndTimePerName::value_type(it->first, ImageAndTime(image, it->second.second)));
	}

	//write!
	juce::File outFile(m_cacheFile.getFullPathName() + ".new");
	{
		juce::ScopedPointer<juce::FileOutputStream> out(outFile.createOutputStream());
		if( !out.get() )
		{
			return;
		}
		if( out->failedToOpen() )
		{
			delete out;
			return;
		}

		//keep index adresses to store actual images adress afterward
		typedef std::map<std::string, juce::int64> AddressPerFileMap;
		AddressPerFileMap imagesCacheAdressAdress;
		AddressPerFileMap imagesCacheActualAdress;
	
		out->writeInt(toDump.size());
		//index with 0 addresses
		for(TempImageAndTimePerName::const_iterator it = toDump.begin();it != toDump.end();++it)
		{
			out->writeString(it->first.c_str());//name
			imagesCacheAdressAdress.insert(AddressPerFileMap::value_type(it->first, out->getPosition()));
			out->writeInt64(0xFFFFFFFF);//dummy address, fixed below
			out->writeInt64(it->second.second);//time
		}
		//images
		juce::JPEGImageFormat imageFormat;
		imageFormat.setQuality(0.4f);
		for(TempImageAndTimePerName::const_iterator it = toDump.begin();it != toDump.end();++it)
		{
			imagesCacheActualAdress.insert(AddressPerFileMap::value_type(it->first, out->getPosition()));
			imageFormat.writeImageToStream(it->second.first, *out);
		}
		//fix index addresses
	
		AddressAndLastUsePerFileMap newImagesCacheAdresses;
		for(AddressPerFileMap::const_iterator it = imagesCacheAdressAdress.begin();it != imagesCacheAdressAdress.end();++it)
		{
			out->setPosition(it->second);
			out->writeInt64(imagesCacheActualAdress[it->first]);//real address
		}
	}
	//finish
    const juce::ScopedLock myScopedLock (m_mutex);
	outFile.moveFileTo(m_cacheFile);
	loadCacheIndex();

}
//save full cache
void ImageCatalogCache::loadCacheIndex()
{	
    const juce::ScopedLock myScopedLock (m_mutex);
	juce::ScopedPointer<juce::FileInputStream> str(m_cacheFile.createInputStream());

	if( !str.get() )
	{
		return;
	}
	if( str->failedToOpen() || str->isExhausted() )
	{
		delete str;
		return;
	}

	int itemCount = str->readInt();
	for(int i=0;i<itemCount && !str->isExhausted();++i)
	{
		std::string filename = str->readString().toUTF8().getAddress();
		if(!str->isExhausted())
		{
			juce::int64 address = str->readInt64();
			if(!str->isExhausted())
			{
				juce::int64 lastUseTime = str->readInt64();
				m_imagesCacheAdresses.insert(AddressAndLastUsePerFileMap::value_type(filename, AddressAndLastUse(address, lastUseTime)));
			}
		}

	}
}
//save full cache
juce::Image ImageCatalogCache::loadCachedFile(std::string const& f)
{
    const juce::ScopedLock myScopedLock (m_mutex);
	AddressAndLastUsePerFileMap::const_iterator it = m_imagesCacheAdresses.find(f);
	if(it == m_imagesCacheAdresses.end())
	{
		return juce::Image::null;
	}
	juce::ScopedPointer<juce::FileInputStream> str(m_cacheFile.createInputStream());
	if( !str.get() )
	{
		return juce::Image::null;
	}
	if( str->failedToOpen() || str->isExhausted() )
	{
		delete str;
		return juce::Image::null;
	}
	str->setPosition(it->second.first);
	return juce::ImageFileFormat::loadFrom(*str);
}