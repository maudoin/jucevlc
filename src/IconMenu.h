#ifndef ICON_MENU_H
#define ICON_MENU_H


#include "juce.h"
#include "ImageCatalog.h"
#include "Thumbnailer.h"
#include <string>
#include <set>

//==============================================================================
class IconMenu
{
protected:

    juce::Image appImage;
    juce::ScopedPointer<juce::Drawable> folderImage;
    juce::ScopedPointer<juce::Drawable> upImage;
	std::set<juce::String> m_videoExtensions;
	std::string m_mediaPostersRoot;
	std::string m_mediaPostersAbsoluteRoot;
	int m_mediaPostersStartIndex;
	int m_mediaPostersHightlight;
	int m_mediaPostersXCount;
	int m_mediaPostersYCount;
	mutable juce::CriticalSection m_mutex;
	juce::Array<juce::File> m_currentFiles;
	bool m_leftArrowHighlighted;
	bool m_rightArrowHighlighted;
	bool m_sliderHighlighted;
	
	mutable ImageCatalog m_imageCatalog;//its a cache
	Thumbnailer m_thumbnailer;

	
	juce::Rectangle<float> computeSliderRect(float w, float h) const;
	juce::Rectangle<float> computeLeftArrowRect(juce::Rectangle<float> const& slider) const;
	juce::Rectangle<float> computeRightArrowRect(juce::Rectangle<float> const& slider) const;
	
	juce::Rectangle<float> getButtonAt(int index, float w, float h)const;
	int getButtonIndexAt(float xPos, float yPos, float w, float h)const;
	std::string getMediaAt(int index)const;
	juce::File getMediaFileAt(int index)const;

	void paintItem(juce::Graphics& g,  int index, float w, float h)const;
	bool updateFilePreview(juce::File const& f);
	juce::File findFirstMovie(juce::File const& f)const;

	int mediaCount()const;
	
	static const int InvalidIndex;
public:
	IconMenu();
	virtual ~IconMenu();

	void setFilter(std::set<juce::String> const & s){m_videoExtensions = s;};

	void setMediaRootPath(std::string const& path);
	void scrollDown();
	void scrollUp();
	void setCurrentMediaRootPath(std::string const& path);
	void setMediaStartIndex(int index);
	bool clickOrDrag(float xPos, float yPos, float w, float h);
	std::string getMediaAt(float xPos, float yPos, float w, float h)const;

	
	bool highlight(float xPos, float yPos, float w, float h);

	void paintMenu(juce::Graphics& g, float w, float h)const;

	bool updatePreviews();


};
//==============================================================================

#endif //MENU_BASE_H