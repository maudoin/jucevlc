#ifndef ABSTRACT_MENU_H
#define ABSTRACT_MENU_H


#include "juce.h"
#include <boost/function.hpp>
	
class AbstractMenuItem;

typedef boost::function<void (AbstractMenuItem&)> AbstractAction;

class AbstractMenuItem
{
public:
	virtual ~AbstractMenuItem(){}
    virtual AbstractMenuItem* addAction(juce::String const& name, AbstractAction action, const juce::Drawable* icon = nullptr) = 0;
	virtual void focusItemAsMenuShortcut() = 0;
	virtual void forceSelection(bool force = true) = 0;
	virtual void forceParentSelection(bool force = true) = 0;
	virtual bool isMenuShortcut() = 0;
};


//==============================================================================
class AbstractMenu : public AppProportionnalComponent
{
protected:
	AbstractAction rootAction;
    juce::Drawable const* itemImage;
public:
	AbstractMenu(){}
	virtual ~AbstractMenu(){}

	virtual void setRootAction(AbstractAction rootAction_){rootAction=rootAction_;}

	
	void setItemImage(juce::Drawable const* itemImage_){itemImage=itemImage_;}
	juce::Drawable const* getItemImage() const { return itemImage; };

	virtual juce::Component* asComponent() = 0;
	virtual juce::Component const* asComponent() const = 0;

};

#endif //ABSTRACT_MENU_H