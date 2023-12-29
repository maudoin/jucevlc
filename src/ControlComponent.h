
#ifndef CONTORL_COMPONENT_H
#define CONTORL_COMPONENT_H


#include <JuceHeader.h>
#include "AppProportionnalComponent.h"
#include "SettingSlider.h"
#include <sstream>
#include <functional>

juce::String toString(juce::int64 time);

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

class ControlComponent   : public juce::Component, public AppProportionnalComponent, public juce::Slider::Listener
{
public:
	typedef std::function<void (double)> ActionSliderCallback;
private:
    std::unique_ptr<TimeSlider> m_slider;
    std::unique_ptr<juce::DrawableButton> m_playPauseButton;
    std::unique_ptr<juce::DrawableButton> m_stopButton;
    std::unique_ptr<juce::DrawableButton> m_menuButton;
    std::unique_ptr<juce::DrawableButton> m_fullscreenButton;
    std::unique_ptr<juce::DrawableButton> m_auxilliarySliderModeButton;
    std::unique_ptr<juce::Drawable> m_playImage;
    std::unique_ptr<juce::Drawable> m_pauseImage;
    std::unique_ptr<juce::Drawable> m_stopImage;
    std::unique_ptr<juce::Drawable> m_itemImage;
    std::unique_ptr<juce::Drawable> m_folderImage;
    std::unique_ptr<juce::Drawable> m_starImage;
    std::unique_ptr<juce::Drawable> m_fullscreenImage;
    std::unique_ptr<juce::Drawable> m_windowImage;
    std::unique_ptr<SettingSlider> m_auxilliaryControlComponent;
	juce::String timeString;
	ActionSliderCallback m_auxilliarySliderAction;
public:
	ControlComponent();
	virtual ~ControlComponent();

	//juce GUI overrides
    void paint (juce::Graphics& g) final;
    void resized() final;

    void setTime(juce::String const& timeString);

	void showPlayingControls();
	void showPausedControls();
	void hidePlayingControls();
	void showFullscreenControls();
	void showWindowedControls();
	void setScaleComponent(juce::Component* scaleComponent) override;

	void setupAuxilliaryControlComponent(ActionSliderCallback const& f, SettingSlider::Params const& params);

	TimeSlider& slider(){return *m_slider.get();}
	juce::DrawableButton& playPauseButton(){return *m_playPauseButton.get();}
	juce::DrawableButton& stopButton(){return *m_stopButton.get();}
	juce::DrawableButton& menuButton(){return *m_menuButton.get();}
	juce::DrawableButton& fullscreenButton(){return *m_fullscreenButton.get();}
	juce::DrawableButton& auxilliarySliderModeButton(){return *m_auxilliarySliderModeButton.get();}
	SettingSlider& auxilliaryControlComponent(){return *m_auxilliaryControlComponent.get();}

	void sliderValueChanged (Slider* slider) final;
};
#endif //CONTORL_COMPONENT_H
