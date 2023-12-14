#ifndef ABSTRACT_MENU_H
#define ABSTRACT_MENU_H



#include <JuceHeader.h>
#include "AppProportionnalComponent.h"
#include <functional>

class AbstractMenuItem;
typedef std::function<void (AbstractMenuItem&)> AbstractAction;

//==============================================================================
class AbstractMenuItem
{
public:
	enum ActionEffect
	{
		EXECUTE_ONLY,
		REFRESH_MENU,
		STORE_AND_OPEN_CHILDREN
	};
	virtual ~AbstractMenuItem(){}
	virtual bool isMenuShortcut() = 0;
};
//==============================================================================
class AbstractMenu : public AppProportionnalComponent
{
	std::set<juce::String> m_videoExtensions;
	std::set<juce::String> m_playlistExtensions;
	std::set<juce::String> m_subtitlesExtensions;
	std::vector< std::set<juce::String> > m_supportedExtensions;
public:
	using FileMethod = std::function<void(AbstractMenuItem&, juce::File)>;

	AbstractMenu();
	virtual ~AbstractMenu() = default;

	virtual void forceMenuRefresh() = 0;

	virtual juce::Component* asComponent() = 0;
	virtual juce::Component const* asComponent() const = 0;

	void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon = nullptr)
	{
		addMenuItem(name, actionEffect, action, icon, false);
	}
	void addRecentMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon = nullptr)
	{
		addMenuItem(name, actionEffect, action, icon, true);
	}

	void listRecentPath(AbstractMenuItem& item, FileMethod const& fileMethod, juce::File const& path);
	void listFiles(AbstractMenuItem& item, juce::File const& file, FileMethod const& fileMethod, FileMethod const& folderMethod);
	void listShortcuts(AbstractMenuItem&, FileMethod const& fileMethod, juce::StringArray const& shortcuts);
	void listRootFiles(AbstractMenuItem& item, FileMethod const& fileMethod);

	virtual juce::Drawable const* getIcon(juce::String const& e) = 0;
	virtual juce::Drawable const* getIcon(juce::File const& f) = 0;
	virtual juce::Drawable const* getItemImage() const = 0;
	virtual juce::Drawable const* getFolderImage() const = 0;
	virtual juce::Drawable const* getPlaylistImage() const = 0;
	virtual juce::Drawable const* getFolderShortcutImage() const = 0;
	virtual juce::Drawable const* getAudioImage() const = 0;
	virtual juce::Drawable const* getDisplayImage() const = 0;
	virtual juce::Drawable const* getSubtitlesImage() const = 0;
	virtual juce::Drawable const* getBackImage() const = 0;

protected:
	virtual void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon, bool shortcut) = 0;
};



#endif //ABSTRACT_MENU_H