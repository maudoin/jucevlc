#ifndef MENU_TREE_H
#define MENU_TREE_H


#include "juce.h"
#include "AppProportionnalComponent.h"
#include "MenuBase.h"
	
//==============================================================================
class MenuTree : public virtual juce::TreeView, public virtual MenuBase
{
public:
	MenuTree();
	virtual ~MenuTree();

	void resized();
	virtual void fillWith(AbstractAction rootAction_);
	
	void paint (juce::Graphics& g){MenuBase::paintMenuBackGround(g);}
	juce::Component* asComponent() {return this;}
	juce::Component const* asComponent() const {return this;}
};

#endif //MENU_TREE_H