
#ifndef SETTING_SLIDER_H
#define SETTING_SLIDER_H


#include <JuceHeader.h>
#include "AppProportionnalComponent.h"
#include <sstream>
#include <functional>

//==============================================================================

juce::String toString(juce::int64 time);

typedef std::function<void (double)> ActionSliderCallback;

class SettingSlider   : public juce::Component, public juce::Button::Listener, public AppProportionnalComponent
{
    std::unique_ptr<juce::Drawable> m_leftImage;
    std::unique_ptr<juce::Drawable> m_rightImage;
    std::unique_ptr<juce::Drawable> m_resetImage;
    std::unique_ptr<juce::DrawableButton> m_leftButton;
    std::unique_ptr<juce::DrawableButton> m_rightButton;
    std::unique_ptr<juce::DrawableButton> m_resetButton;
    std::unique_ptr<juce::Slider> m_slider;
	double m_buttonsStep;
	double m_resetValue;
	juce::String m_labelFormat;
public:

	SettingSlider();
	virtual ~SettingSlider();

	void addListener(juce::Slider::Listener* l);

	//juce GUI overrides
	void resized() override;
	void buttonClicked (juce::Button* button) override;
	void paint(juce::Graphics& g) override;

	struct Params{juce::String label; double init, defaultVal, min, max, step=1., arrowsStep=0.;};
	void setup(Params const&params);

	bool is(juce::Slider* s)const{return m_slider.get()==s;}

	double getValue()const;

};

#endif //SETTING_SLIDER_H
