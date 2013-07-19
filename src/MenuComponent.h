#ifndef MENU_COMPONENT_H
#define MENU_COMPONENT_H


#include "MenuBase.h"
	
class MenuList;
class RecentList;
//==============================================================================
class MenuComponent : public virtual juce::Component, public virtual MenuBase
{
	juce::ScopedPointer<MenuList> menuList;
	juce::ScopedPointer<RecentList> recentList;
public:
	MenuComponent();
	virtual ~MenuComponent();

	void resized();
	virtual void fillWith(AbstractAction rootAction_);
	
	void paint (juce::Graphics& g);
	juce::Component* asComponent() {return this;}
	juce::Component const* asComponent() const {return this;}
};

#endif //MENU_COMPONENT_H