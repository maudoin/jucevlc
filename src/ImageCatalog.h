#ifndef IMAGECATALOG_H
#define IMAGECATALOG_H


#include "juce.h"
#include <string>
#include <map>

//==============================================================================
class ImageCatalog
{
protected:
	juce::CriticalSection m_imagesMutex;
	std::map<std::string, juce::Image> m_iconPerFile;
	
	
public:
	ImageCatalog();
	virtual ~ImageCatalog();
	
	void storeImageInCache(juce::File const& path, juce::Image const& i = juce::Image::null);
	juce::Image get(juce::File const& file)const;
	bool contains(juce::File const& file)const;


};
//==============================================================================

#endif //IMAGECATALOG_H