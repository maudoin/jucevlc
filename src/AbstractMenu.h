#ifndef ABSTRACT_MENU_H
#define ABSTRACT_MENU_H


#include "juce.h"
#include "AppProportionnalComponent.h"
#include <boost/function.hpp>
	
class AbstractMenuItem;
typedef boost::function<void (AbstractMenuItem&)> AbstractAction;

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
public:
	AbstractMenu(){}
	virtual ~AbstractMenu(){}

	virtual void fillWith(AbstractAction rootAction_) = 0;
	virtual void forceMenuRefresh() = 0;

	
	virtual void setItemImage(juce::Drawable const* itemImage_) = 0;
	virtual juce::Drawable const* getItemImage() const = 0;

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
protected:
	virtual void addMenuItem(juce::String const& name, AbstractMenuItem::ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon, bool shortcut) = 0;
};



#endif //ABSTRACT_MENU_H