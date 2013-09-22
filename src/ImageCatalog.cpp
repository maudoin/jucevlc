
#include "ImageCatalog.h"


ImageCatalog::ImageCatalog()
{
}

ImageCatalog::~ImageCatalog()
{
}

void ImageCatalog::storeImageInCache(juce::File const& f, juce::Image const& i)
{
	const juce::ScopedLock myScopedLock (m_imagesMutex);
	std::cerr << f.getFileName().toUTF8().getAddress() << std::endl;
	m_iconPerFile.insert(std::map<std::string, juce::Image>::value_type(f.getFileName().toUTF8().getAddress(), i));
}

bool ImageCatalog::contains(juce::File const& file)const
{
	const juce::ScopedLock myScopedLock (m_imagesMutex);
	std::map<std::string, juce::Image>::const_iterator itImage = m_iconPerFile.find(file.getFileName().toUTF8().getAddress());
	return (itImage != m_iconPerFile.end());
}
juce::Image ImageCatalog::get(juce::File const& file)const
{
	const juce::ScopedLock myScopedLock (m_imagesMutex);
	std::map<std::string, juce::Image>::const_iterator it = m_iconPerFile.find(file.getFileName().toUTF8().getAddress());
	if(it != m_iconPerFile.end())
	{
		return it->second;
	}
	else
	{
		return juce::Image::null;
	}

}