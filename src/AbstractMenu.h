#ifndef ABSTRACT_MENU_H
#define ABSTRACT_MENU_H


#include "AppProportionnalComponent.h"
#include "MenuComponentValue.h"

#include <JuceHeader.h>
#include <functional>

using AbstractAction = std::function<void (MenuComponentValue const&)> ;

//==============================================================================
namespace AbstractMenuItem{
enum ActionEffect
{
	EXECUTE_ONLY,
	REFRESH_MENU,
	STORE_AND_OPEN_CHILDREN,
	STORE_AND_OPEN_COLOR,
	STORE_AND_OPEN_SLIDER
};
}
//==============================================================================
class AbstractMenu : public AppProportionnalComponent
{
	std::set<juce::String> m_videoExtensions;
	std::set<juce::String> m_playlistExtensions;
	std::set<juce::String> m_subtitlesExtensions;
	std::vector< std::set<juce::String> > m_supportedExtensions;
public:
	using FileMethod = std::function<void(MenuComponentValue const&, juce::File)>;

	AbstractMenu();
	virtual ~AbstractMenu() = default;

	virtual void forceMenuRefresh() = 0;

	virtual juce::Component* asComponent() = 0;
	virtual juce::Component const* asComponent() const = 0;

	virtual int preferredHeight()const = 0;

	virtual void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon = nullptr, MenuComponentParams const& params = {}) = 0;
	virtual void addRecentMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon = nullptr) = 0;

	void listRecentPath(MenuComponentValue const&, FileMethod const& fileMethod, juce::File const& path);
	void listFiles(MenuComponentValue const&, juce::File const& file, FileMethod const& fileMethod, FileMethod const& folderMethod);
	void listShortcuts(MenuComponentValue const&, FileMethod const& fileMethod, juce::StringArray const& shortcuts);
	void listRootFiles(MenuComponentValue const&, FileMethod const& fileMethod);

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

};



#endif //ABSTRACT_MENU_H