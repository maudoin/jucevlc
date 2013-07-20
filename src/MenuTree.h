#ifndef MENU_TREE_H
#define MENU_TREE_H


#include "MenuBase.h"
	
//==============================================================================
class MenuTree : public virtual juce::TreeView, public virtual MenuBase
{
public:
	MenuTree();
	virtual ~MenuTree();

	void resized();
	
	void paint (juce::Graphics& g){MenuBase::paintMenuBackGround(g);}
	juce::Component* asComponent() {return this;}
	juce::Component const* asComponent() const {return this;}
	
	virtual void forceMenuRefresh();
	void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon, bool shortcut);
};

#endif //MENU_TREE_H