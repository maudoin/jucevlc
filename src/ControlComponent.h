
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
	typedef std::function<void (double)> VolumeSliderCallback;
private:
    std::unique_ptr<TimeSlider> m_slider;
    std::unique_ptr<juce::DrawableButton> m_playPauseButton;
    std::unique_ptr<juce::DrawableButton> m_stopButton;
    std::unique_ptr<juce::DrawableButton> m_menuButton;
    std::unique_ptr<juce::DrawableButton> m_fullscreenButton;
    std::unique_ptr<juce::DrawableButton> m_volumeButton;
    std::unique_ptr<juce::Drawable> m_playImage;
    std::unique_ptr<juce::Drawable> m_pauseImage;
    std::unique_ptr<juce::Drawable> m_stopImage;
    std::unique_ptr<juce::Drawable> m_settingsImage;
    std::unique_ptr<juce::Drawable> m_audioImage;
    std::unique_ptr<juce::Drawable> m_fullscreenImage;
    std::unique_ptr<juce::Drawable> m_windowImage;
    std::unique_ptr<juce::Slider> m_volumeSlider;
	juce::String timeString;
	VolumeSliderCallback m_volumeSliderAction;
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

	void setupVolumeSlider(VolumeSliderCallback const& f,  double value, double min, double max, double step=1.);

	TimeSlider& slider(){return *m_slider.get();}
	juce::DrawableButton& playPauseButton(){return *m_playPauseButton.get();}
	juce::DrawableButton& stopButton(){return *m_stopButton.get();}
	juce::DrawableButton& menuButton(){return *m_menuButton.get();}
	juce::DrawableButton& fullscreenButton(){return *m_fullscreenButton.get();}
	juce::DrawableButton& volumeButton(){return *m_volumeButton.get();}

	void sliderValueChanged (Slider* slider) final;
};
#endif //CONTORL_COMPONENT_H
