#ifndef MENU_COMPONENT_H
#define MENU_COMPONENT_H


#include "MenuBase.h"
	
class MenuItemList;
//==============================================================================
class MenuComponent : public virtual juce::Component, public virtual MenuBase, public virtual AbstractMenuItem
{
	juce::ScopedPointer<MenuItemList> menuList;
	juce::ScopedPointer<MenuItemList> recentList;
public:
	MenuComponent();
	virtual ~MenuComponent();

	void resized();
	virtual void fillWith(AbstractAction rootAction_);
	
	void paint (juce::Graphics& g);
	juce::Component* asComponent() {return this;}
	juce::Component const* asComponent() const {return this;}

	void menuItemSelected(int /*lastRowselected*/);
	void recentItemSelected(int /*lastRowselected*/);

	
	virtual AbstractMenuItem* addAction(juce::String const& name, AbstractAction action, const juce::Drawable* icon = nullptr);
	virtual void focusItemAsMenuShortcut();
	virtual void forceSelection(bool force = true);
	virtual void forceParentSelection(bool force = true);
	virtual bool isMenuShortcut();

};

#endif //MENU_COMPONENT_H