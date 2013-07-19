#ifndef MENU_BASE_H
#define MENU_BASE_H


#include "AbstractMenu.h"

//==============================================================================
class MenuBase : public AbstractMenu
{
protected:
    juce::Drawable const* itemImage;
public:
	MenuBase();
	virtual ~MenuBase();
	
	void setItemImage(juce::Drawable const* itemImage_){itemImage=itemImage_;}
	juce::Drawable const* getItemImage() const { return itemImage; };

protected:
	
	void paintMenuBackGround(juce::Graphics& g);

};
//==============================================================================

void paintMenuItem (juce::Graphics& g, int width, int height, bool isItemSelected, juce::String const& name, const juce::Drawable* d, bool isShortcut = false);

#endif //MENU_BASE_H