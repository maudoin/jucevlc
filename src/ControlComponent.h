
#ifndef CONTORL_COMPONENT_H
#define CONTORL_COMPONENT_H


#include <JuceHeader.h>
#include "AppProportionnalComponent.h"
#include <sstream>
#include <functional>

//==============================================================================

juce::String toString(juce::int64 time);

typedef std::function<void (double)> ActionSliderCallback;

class SecondaryControlComponent   : public juce::Component, public juce::Button::Listener, public juce::Slider::Listener, public AppProportionnalComponent
{
    std::unique_ptr<juce::Slider> m_slider;
    std::unique_ptr<juce::DrawableButton> m_leftButton;
    std::unique_ptr<juce::DrawableButton> m_rightButton;
    std::unique_ptr<juce::Drawable> m_leftImage;
    std::unique_ptr<juce::Drawable> m_rightImage;
	double m_buttonsStep;
	double m_resetValue;
	ActionSliderCallback m_sliderAction;
	juce::String m_labelFormat;
public:
	SecondaryControlComponent();
	virtual ~SecondaryControlComponent();

	//juce GUI overrides
	void resized() override;
	void buttonClicked (juce::Button* button) override;
	void sliderValueChanged (juce::Slider* slider) override;
	void paint(juce::Graphics& g) override;

	void reset();
	void disableAndHide();
	void show(juce::String const& label, ActionSliderCallback const& f, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep = 0.f);

};

class TimeSlider   : public juce::Slider, public AppProportionnalComponent
{
	juce::String mouseOverTimeString;
	int mouseOverTimeStringPos;
public:
	TimeSlider();
	virtual ~TimeSlider();


    void setMouseOverTime(int pos, juce::int64 time);
	void resetMouseOverTime();

	//juce GUI overrides
    virtual void paint (juce::Graphics& g);
};

class ControlComponent   : public juce::Component, public AppProportionnalComponent
{
    std::unique_ptr<TimeSlider> m_slider;
    std::unique_ptr<juce::DrawableButton> m_playPauseButton;
    std::unique_ptr<juce::DrawableButton> m_stopButton;
    std::unique_ptr<juce::DrawableButton> m_menuButton;
    std::unique_ptr<juce::DrawableButton> m_fullscreenButton;
    std::unique_ptr<juce::DrawableButton> m_auxilliarySliderModeButton;
    std::unique_ptr<juce::DrawableButton> m_resetButton;
    std::unique_ptr<juce::Drawable> m_playImage;
    std::unique_ptr<juce::Drawable> m_pauseImage;
    std::unique_ptr<juce::Drawable> m_stopImage;
    std::unique_ptr<juce::Drawable> m_itemImage;
    std::unique_ptr<juce::Drawable> m_folderImage;
    std::unique_ptr<juce::Drawable> m_starImage;
    std::unique_ptr<juce::Drawable> m_fullscreenImage;
    std::unique_ptr<juce::Drawable> m_windowImage;
    std::unique_ptr<juce::Drawable> m_undoImage;
    std::unique_ptr<SecondaryControlComponent> m_auxilliaryControlComponent;
	juce::String timeString;
public:
	ControlComponent();
	virtual ~ControlComponent();

	//juce GUI overrides
    virtual void paint (juce::Graphics& g);
    virtual void resized();

    void setTime(juce::String const& timeString);

	void showPlayingControls();
	void showPausedControls();
	void hidePlayingControls();
	void showFullscreenControls();
	void showWindowedControls();
	void setScaleComponent(juce::Component* scaleComponent) override;

	TimeSlider& slider(){return *m_slider.get();}
	juce::DrawableButton& playPauseButton(){return *m_playPauseButton.get();}
	juce::DrawableButton& stopButton(){return *m_stopButton.get();}
	juce::DrawableButton& menuButton(){return *m_menuButton.get();}
	juce::DrawableButton& fullscreenButton(){return *m_fullscreenButton.get();}
	juce::DrawableButton& auxilliarySliderModeButton(){return *m_auxilliarySliderModeButton.get();}
	juce::DrawableButton& resetButton(){return *m_resetButton.get();}
	SecondaryControlComponent& auxilliaryControlComponent(){return *m_auxilliaryControlComponent.get();}

};
#endif //CONTORL_COMPONENT_H
