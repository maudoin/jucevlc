#ifndef MENU_COMPONENT_H
#define MENU_COMPONENT_H


#include "MenuBase.h"
	
class MenuItemList;
//==============================================================================
class MenuComponent : public virtual juce::Component, public virtual MenuBase
{
	juce::ScopedPointer<MenuItemList> menuList;
	juce::ScopedPointer<MenuItemList> recentList;
public:
	MenuComponent();
	virtual ~MenuComponent();

	void resized();
	virtual void forceMenuRefresh();
	
	void paint (juce::Graphics& g);
	juce::Component* asComponent() {return this;}
	juce::Component const* asComponent() const {return this;}

	void menuItemSelected(int /*lastRowselected*/);
	void recentItemSelected(int /*lastRowselected*/);

protected:
	
	void forceRecentItem(int index);

	virtual void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon, bool shortcut);
	virtual void addMenuItem(MenuItemList* target, juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon);
};

#endif //MENU_COMPONENT_H