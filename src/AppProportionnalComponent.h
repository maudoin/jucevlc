#ifndef APPPROPORTIONNALCOMPONENT
#define APPPROPORTIONNALCOMPONENT

#include "AppConfig.h"
#include "juce.h"

class AppProportionnalComponent : public juce::ComponentListener
{
	juce::Component* m_scaleComponent;
	static int m_itemHeightScalePercent;

public:
	AppProportionnalComponent();
    virtual ~AppProportionnalComponent();

	static int getItemHeightPercentageRelativeToScreen();
	static void setItemHeightPercentageRelativeToScreen(int percent, AppProportionnalComponent* = nullptr);

	void setScaleComponent(juce::Component* scaleComponent);
	float getFontHeight();
	float getItemHeight();
    virtual void componentMovedOrResized (juce::Component& component,
                                          bool wasMoved,
                                          bool wasResized);
	
	virtual void appProportionnalComponentResized() {};
};

#endif