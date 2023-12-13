#ifndef ICON_MENU_H
#define ICON_MENU_H



#include <JuceHeader.h>
#include "ImageCatalog.h"
#include <string>
#include <set>

//==============================================================================
class IconMenu
{
public:

	typedef std::pair<juce::File, bool> PathAndImage;
protected:

    juce::Image appImage;
    std::unique_ptr<juce::Drawable> folderBackImage;
    std::unique_ptr<juce::Drawable> folderFrontImage;
    std::unique_ptr<juce::Drawable> driveImage;
    std::unique_ptr<juce::Drawable> diskImage;
    std::unique_ptr<juce::Drawable> usbImage;
    std::unique_ptr<juce::Drawable> upImage;
	std::set<juce::String> m_videoExtensions;
	std::string m_mediaPostersRoot;
	std::string m_mediaPostersAbsoluteRoot;
	int m_mediaPostersStartIndex;
	int m_mediaPostersHightlight;
	int m_mediaPostersXCount;
	int m_mediaPostersYCount;
	mutable juce::CriticalSection m_currentFilesMutex;
	juce::Array<PathAndImage> m_currentFiles;
	mutable juce::CriticalSection m_mutex;
	bool m_leftArrowHighlighted;
	bool m_rightArrowHighlighted;
	bool m_sliderHighlighted;

	mutable ImageCatalog m_imageCatalog;//its a cache

	int m_colorThemeHue;


	juce::Rectangle<float> computeSliderRect(float w, float h) const;
	juce::Rectangle<float> computeLeftArrowRect(juce::Rectangle<float> const& slider) const;
	juce::Rectangle<float> computeRightArrowRect(juce::Rectangle<float> const& slider) const;

	juce::Rectangle<float> getButtonAt(int index, float w, float h)const;
	int getButtonIndexAt(float xPos, float yPos, float w, float h)const;
	std::string getMediaAtIndexOnScreen(int index)const;
	juce::File getMediaFileAtIndexOnScreen(int index)const;
	juce::File getMediaFileAtIndexInfolder(int indexInfolder)const;
	int getFileIndexAtScreenIndex(int indexOnScreen)const;

	void paintItem(juce::Graphics& g,  int index, float w, float h)const;
	bool updateFilePreview(PathAndImage& pathAndImageLoadedFlag);
	juce::File findFirstMovie(juce::File const& f)const;

	int mediaCount()const;

	static const int InvalidIndex;
	static const int UpFolderIndex;
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
	std::string getMediaAtIndexOnScreen(float xPos, float yPos, float w, float h)const;


	bool highlight(float xPos, float yPos, float w, float h);

	void paintMenu(juce::Graphics& g, float w, float h)const;

	bool updatePreviews();


};
//==============================================================================

#endif //MENU_BASE_H