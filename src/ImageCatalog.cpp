
#include "ImageCatalog.h"


ImageCatalog::ImageCatalog()
	:m_cache(juce::File::getCurrentWorkingDirectory().getChildFile("thumbnails"))
	,m_changedSinceLastSave(false)
{
}

ImageCatalog::~ImageCatalog()
{
	maySaveCache();
}

void ImageCatalog::storeImageInCacheAndSetChanged(juce::File const& f, juce::Image const& i)
{
	storeImageInCache(f, i);
	m_changedSinceLastSave = true;
}

void ImageCatalog::storeImageInCache(juce::File const& f, juce::Image const& i)
{
	const juce::ScopedLock myScopedLock (m_imagesMutex);
	std::cerr << f.getFileName().toUTF8().getAddress() << std::endl;
	m_iconPerFile.insert(std::map<std::string, juce::Image>::value_type(f.getFileName().toUTF8().getAddress(), i));
}
void ImageCatalog::preload(juce::Array<std::pair<juce::File, bool> > & files, juce::int64 maxTimeMs)
{
	juce::int64 now = juce::Time::currentTimeMillis();
	
	for(std::pair<juce::File, bool>* it = files.begin();(it != files.end()) && ((juce::Time::currentTimeMillis() - now )<maxTimeMs);++it)
	{
		juce::File const& file(it->first);
		juce::Image cached = m_cache.loadCachedFile(file.getFileName().toUTF8().getAddress());
		if(!cached.isNull())
		{
			storeImageInCache(file, cached);
			it->second = true;
		}
	}
}
juce::Image ImageCatalog::get(juce::File const& file)
{
	const juce::ScopedLock myScopedLock (m_imagesMutex);
	std::map<std::string, juce::Image>::const_iterator it = m_iconPerFile.find(file.getFileName().toUTF8().getAddress());
	if(it != m_iconPerFile.end())
	{
		return it->second;
	}
	else
	{
		juce::Image cached = m_cache.loadCachedFile(file.getFileName().toUTF8().getAddress());
		if(!cached.isNull())
		{
			storeImageInCache(file, cached);
			return cached;
		}
		return juce::Image::null;
	}

}


void ImageCatalog::maySaveCache()
{
	{
		const juce::ScopedLock myScopedLock (m_imagesMutex);
		if(!m_changedSinceLastSave)
		{
			return;
		}
	}
	std::map<std::string, juce::Image> copy;
	{
		const juce::ScopedLock myScopedLock (m_imagesMutex);
		copy = m_iconPerFile;
	}
	m_cache.saveCache(copy);
	m_changedSinceLastSave = false;
}