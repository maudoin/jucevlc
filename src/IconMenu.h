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
	int m_mediaPostersStartIndex;
	int m_mediaPostersHightlight;
	int m_mediaPostersXCount;
	int m_mediaPostersYCount;
	juce::CriticalSection m_mutex;
	juce::Array<juce::File> m_currentFiles;

public:
	IconMenu();
	virtual ~IconMenu();

	void setFilter(std::set<juce::String> const & s){m_videoExtensions = s;};

	void setMediaRootPath(std::string const& path){m_mediaPostersRoot=path;setMediaStartIndex(0);}
	void setMediaStartIndex(int index);
	
	juce::Rectangle<float> getButtonAt(int index, float w, float h);
	int getButtonIndexAt(float xPos, float yPos, float w, float h);
	std::string clickOrGetMediaAt(float xPos, float yPos, float w, float h);
	std::string getMediaAt(float xPos, float yPos, float w, float h);
	std::string getMediaAt(int index);
	juce::File getMediaFileAt(int index);

	
	void highlight(float xPos, float yPos, float w, float h);

	void paintItem(juce::Graphics& g, juce::Image const & i, int index, float w, float h);
	void paintMenu(juce::Graphics& g, juce::Image const & i,  float w, float h);

};
//==============================================================================

#endif //MENU_BASE_H