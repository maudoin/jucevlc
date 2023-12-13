#ifndef MENU_BASE_H
#define MENU_BASE_H


#include "AbstractMenu.h"

//==============================================================================
class MenuBase : public AbstractMenu
{
public:
	MenuBase();
	virtual ~MenuBase();

protected:

	void paintMenuBackGround(juce::Graphics& g);

};
//==============================================================================

void paintMenuItem (juce::Graphics& g, int width, int height, bool isItemSelected, juce::String const& name, const juce::Drawable* d, bool isShortcut = false);

#endif //MENU_BASE_H