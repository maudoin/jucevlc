#ifndef MENU_BASE_H
#define MENU_BASE_H


#include "AbstractMenu.h"

//==============================================================================
class MenuBase : public AbstractMenu
{
protected:
	AbstractAction rootAction;
    juce::Drawable const* itemImage;
public:
	MenuBase();
	virtual ~MenuBase();

	virtual void fillWith(AbstractAction rootAction_){rootAction=rootAction_;}

	
	void setItemImage(juce::Drawable const* itemImage_){itemImage=itemImage_;}
	juce::Drawable const* getItemImage() const { return itemImage; };

protected:
	
	void paintMenuBackGround(juce::Graphics& g);

};
//==============================================================================
class MenuItemBase : public AbstractMenuItem
{
protected:
	AbstractMenu& owner;
	bool m_isShortcut;
	juce::String name;
	const juce::Drawable* icon;
	AbstractAction action;

    MenuItemBase (AbstractMenu& owner, juce::String name, AbstractAction action, const juce::Drawable* icon = nullptr);
	void execute();
	virtual const juce::Drawable* getIcon(bool isItemSelected);
    void paintMenuItem (juce::Graphics& g, int width, int height, bool isItemSelected);
public:
	
	virtual bool isMenuShortcut(){return m_isShortcut;}
};



#endif //MENU_BASE_H