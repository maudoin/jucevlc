#ifndef MENU_TREE_H
#define MENU_TREE_H


#include "juce.h"
#include "AppProportionnalComponent.h"
#include "AbstractMenu.h"
	
//==============================================================================
class MenuTree : public virtual juce::TreeView, public virtual AbstractMenu
{
public:
	MenuTree();
	virtual ~MenuTree();
	virtual void refresh();
	void setInitialMenu();

	void resized();
	void paint (juce::Graphics& g);

	virtual void setRootAction(AbstractAction rootAction_){AbstractMenu::setRootAction(rootAction_);refresh();}
	
	juce::Component* asComponent() {return this;}
	juce::Component const* asComponent() const {return this;}
};

#endif //MENU_TREE_H