#ifndef ICON_MENU_H
#define ICON_MENU_H


#include "juce.h"
#include <string>
#include <set>

//==============================================================================
class IconMenu
{
protected:
    juce::ScopedPointer<juce::Drawable> folderImage;
    juce::ScopedPointer<juce::Drawable> upImage;
	std::set<juce::String> m_videoExtensions;
	std::string m_mediaPostersRoot;
	std::string m_mediaPostersAbsoluteRoot;
	int m_mediaPostersStartIndex;
	int m_mediaPostersHightlight;
	int m_mediaPostersXCount;
	int m_mediaPostersYCount;
	juce::CriticalSection m_mutex;
	juce::Array<juce::File> m_currentFiles;
	bool m_leftArrowHighlighted;
	bool m_rightArrowHighlighted;
	bool m_sliderHighlighted;

	
	juce::Rectangle<float> computeSliderRect(float w, float h) const;
	juce::Rectangle<float> computeLeftArrowRect(juce::Rectangle<float> const& slider) const;
	juce::Rectangle<float> computeRightArrowRect(juce::Rectangle<float> const& slider) const;
	
	juce::Rectangle<float> getButtonAt(int index, float w, float h);
	int getButtonIndexAt(float xPos, float yPos, float w, float h);
	std::string getMediaAt(int index);
	juce::File getMediaFileAt(int index);

	void paintItem(juce::Graphics& g, juce::Image const & i, int index, float w, float h);

	int mediaCount();
	
	static const int InvalidIndex;
public:
	IconMenu();
	virtual ~IconMenu();

	void setFilter(std::set<juce::String> const & s){m_videoExtensions = s;};

	void setMediaRootPath(std::string const& path);
	void setCurrentMediaRootPath(std::string const& path);
	void setMediaStartIndex(int index);
	bool clickOrDrag(float xPos, float yPos, float w, float h);
	std::string getMediaAt(float xPos, float yPos, float w, float h);

	
	bool highlight(float xPos, float yPos, float w, float h);

	void paintMenu(juce::Graphics& g, juce::Image const & i,  float w, float h);

};
//==============================================================================

#endif //MENU_BASE_H