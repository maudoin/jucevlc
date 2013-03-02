
#ifndef CONTORL_COMPONENT_H
#define CONTORL_COMPONENT_H

#include "juce.h"
#include "AppProportionnalComponent.h"
#include <sstream>
#include <boost/function.hpp>

//==============================================================================

juce::String toString(juce::int64 time);

class ActionSlider;

typedef boost::function<void (double)> ActionSliderCallback;

class SecondaryControlComponent   : public juce::Component, public juce::Button::Listener
{
    juce::ScopedPointer<ActionSlider> m_slider;
    juce::ScopedPointer<juce::DrawableButton> m_leftButton;
    juce::ScopedPointer<juce::DrawableButton> m_rightButton;
    juce::ScopedPointer<juce::Drawable> m_leftImage;
    juce::ScopedPointer<juce::Drawable> m_rightImage;
	double m_buttonsStep;
public:
	SecondaryControlComponent();
	virtual ~SecondaryControlComponent();

	//juce GUI overrides
	virtual void resized();
	void buttonClicked (juce::Button* button);
	
	void paint(juce::Graphics& g);
	void show(juce::String const& label, ActionSliderCallback const& f, double value, double volumeMin, double volumeMax, double step, double buttonsStep = 0.f);
	
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
    juce::ScopedPointer<TimeSlider> m_slider;
    juce::ScopedPointer<juce::DrawableButton> m_playPauseButton;
    juce::ScopedPointer<juce::DrawableButton> m_stopButton;
    juce::ScopedPointer<juce::DrawableButton> m_menuButton;
    juce::ScopedPointer<juce::DrawableButton> m_alternateSliderModeButton;
    juce::ScopedPointer<juce::Drawable> m_playImage;
    juce::ScopedPointer<juce::Drawable> m_pauseImage;
    juce::ScopedPointer<juce::Drawable> m_stopImage;
    juce::ScopedPointer<juce::Drawable> m_itemImage;
    juce::ScopedPointer<juce::Drawable> m_folderImage;
    juce::ScopedPointer<SecondaryControlComponent> m_alternateControlComponent;
	juce::String timeString;
	juce::String currentTimeString;
public:
	ControlComponent();
	virtual ~ControlComponent();

	//juce GUI overrides
    virtual void paint (juce::Graphics& g);
    virtual void resized();

    void setTime(juce::int64 time, juce::int64 len);

	void showPlayingControls();
	void showPausedControls();
	void hidePlayingControls();
	
	TimeSlider& slider(){return *m_slider.get();}
	juce::DrawableButton& playPauseButton(){return *m_playPauseButton.get();}
	juce::DrawableButton& stopButton(){return *m_stopButton.get();}
	juce::DrawableButton& menuButton(){return *m_menuButton.get();}
	juce::DrawableButton& alternateSliderModeButton(){return *m_alternateSliderModeButton.get();}
	SecondaryControlComponent& alternateControlComponent(){return *m_alternateControlComponent.get();}

};
#endif //CONTORL_COMPONENT_H
