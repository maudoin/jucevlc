#ifndef MENU_COMPONENT_H
#define MENU_COMPONENT_H


#include "AbstractMenu.h"

class MenuItemList;
//==============================================================================
class MenuComponent : public virtual juce::Component, public virtual AbstractMenu
{
	std::unique_ptr<MenuItemList> menuList;
	std::unique_ptr<MenuItemList> recentList;
    std::unique_ptr<juce::Drawable> itemImage;
    std::unique_ptr<juce::Drawable> folderImage;
    std::unique_ptr<juce::Drawable> playlistImage;
    std::unique_ptr<juce::Drawable> folderShortcutImage;
    std::unique_ptr<juce::Drawable> hideFolderShortcutImage;
    std::unique_ptr<juce::Drawable> audioImage;
    std::unique_ptr<juce::Drawable> displayImage;
    std::unique_ptr<juce::Drawable> subtitlesImage;
    std::unique_ptr<juce::Drawable> likeAddImage;
    std::unique_ptr<juce::Drawable> likeRemoveImage;
    std::unique_ptr<juce::Drawable> backImage;
	std::set<juce::String> m_videoExtensions;
	std::set<juce::String> m_playlistExtensions;
	std::set<juce::String> m_subtitlesExtensions;
	bool m_gradient;
public:

	MenuComponent(bool gradient = true);
	virtual ~MenuComponent();

	void resized();
	virtual void forceMenuRefresh();

	void paint (juce::Graphics& g) final;
	juce::Component* asComponent() final {return this;}
	juce::Component const* asComponent() const final {return this;}

	void menuItemSelected(int /*lastRowselected*/);
	void recentItemSelected(int /*lastRowselected*/);

protected:

	juce::Drawable const* getIcon(juce::String const& e) final;
	juce::Drawable const* getIcon(juce::File const& f) final;
	juce::Drawable const* getItemImage() const final { return itemImage.get(); };
	juce::Drawable const* getFolderImage() const final { return folderImage.get(); };
	juce::Drawable const* getPlaylistImage() const final { return playlistImage.get(); };
	juce::Drawable const* getFolderShortcutImage() const final { return likeAddImage.get(); };
	juce::Drawable const* getAudioImage() const final { return audioImage.get(); };
	juce::Drawable const* getDisplayImage() const final { return displayImage.get(); };
	juce::Drawable const* getSubtitlesImage() const final { return subtitlesImage.get(); };
	juce::Drawable const* getBackImage() const final { return backImage.get(); };

	void forceRecentItem(int index);

	using AbstractMenu::addMenuItem;
	virtual void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon, bool shortcut);
	virtual void addMenuItem(MenuItemList* target, juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon);
};

#endif //MENU_COMPONENT_H