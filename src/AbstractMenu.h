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
enum Icon
{
	None = 0,
	Back,
	Check,
	Folder,
	FolderShortcut,
	FolderShortcutOutline,
	Audio,
	Display,
	Subtitles,
	PlayAll,
	AddAll,
	Settings,
	Sliders,
	Exit
};
}
template <typename Enum> struct EnumSize;
template<> struct EnumSize<AbstractMenuItem::Icon> : std::integral_constant<int, AbstractMenuItem::Icon::Exit+1>{};
//==============================================================================
class AbstractMenu : public AppProportionnalComponent
{
public:
	using FileMethod = std::function<void(MenuComponentValue const&, juce::File const&)>;

	virtual ~AbstractMenu() = default;

	virtual void forceMenuRefresh() = 0;

	virtual bool isShown() const = 0;
	virtual void setShown(bool show) = 0;

	virtual void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, AbstractMenuItem::Icon icon = AbstractMenuItem::Icon::None, MenuComponentParams const& params = {}) = 0;
	virtual void addRecentMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, AbstractMenuItem::Icon icon = AbstractMenuItem::Icon::None) = 0;

	//TBR
	virtual juce::Component* asComponent() = 0;
	virtual juce::Component const* asComponent() const = 0;
	virtual int preferredHeight()const = 0;
};



#endif //ABSTRACT_MENU_H