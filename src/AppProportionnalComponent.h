#ifndef APPPROPORTIONNALCOMPONENT
#define APPPROPORTIONNALCOMPONENT

#include <JuceHeader.h>

class AppProportionnalComponent : public juce::ComponentListener
{
	juce::Component* m_scaleComponent;
	static int m_itemHeightScalePercent;
	juce::Font m_font;

public:
	AppProportionnalComponent();
    virtual ~AppProportionnalComponent();

	static int getItemHeightPercentageRelativeToScreen();
	static void setItemHeightPercentageRelativeToScreen(int percent, AppProportionnalComponent* = nullptr);

	virtual void setScaleComponent(juce::Component* scaleComponent);
	float getFontHeight() const;
	float getItemHeight() const;
	juce::Font const& getFont()const{return m_font;}
    virtual void componentMovedOrResized (juce::Component& component,
                                          bool wasMoved,
                                          bool wasResized);

	virtual void appProportionnalComponentResized() {};
	virtual float getFontItemScaleRatio() const{return 1.;}
protected:
	void resizeFont();
};

#endif