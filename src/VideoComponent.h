
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "VLCMenuTree.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>


//==============================================================================
class VideoComponent   : public juce::Component, DisplayCallback, juce::Slider::Listener, juce::Button::Listener, VLCMenuTreeListener, EventCallBack
{
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
    juce::CriticalSection imgCriticalSection;
    juce::ScopedPointer<juce::Slider> slider;
    juce::ScopedPointer<VLCMenuTree> tree;
    juce::ScopedPointer<juce::DrawableButton> playPauseButton;
    juce::ScopedPointer<juce::DrawableButton> stopButton;
    juce::ScopedPointer<juce::Drawable> playImage;
    juce::ScopedPointer<juce::Drawable> pauseImage;
    juce::ScopedPointer<juce::Drawable> stopImage;
	juce::ScopedPointer<VLCWrapper> vlc;
	bool sliderUpdating;
	bool videoUpdating;

	
	juce::String getTimeString() const;
public:
    VideoComponent();
    virtual ~VideoComponent();
	
	void setScaleComponent(juce::Component* scaleComponent);
    void paint (juce::Graphics& g);
	
    virtual void resized();

	
	void play(char* path);
	void play();
	void pause();
	void stop();
	
	//VLC DiaplListener
	void *lock(void **p_pixels);
	void unlock(void *id, void *const *p_pixels);
	void display(void *id);

	void showPlayingControls();
	void showPausedControls();
	void hidePlayingControls();
	void updateTimeAndSlider();

    virtual void sliderValueChanged (juce::Slider* slider);
    virtual void buttonClicked (juce::Button* button);

	juce::Slider* getSlider();
	//MenuTreeListener
    virtual void onOpen (const juce::File& file, const juce::MouseEvent& e);
    virtual void onOpenSubtitle (const juce::File& file, const juce::MouseEvent& e);
    virtual void onOpenPlaylist (const juce::File& file, const juce::MouseEvent& e);

    virtual void onCrop (double ratio);
    virtual void onSetAspectRatio(const juce::String& ratio);
    virtual void onShiftAudio(const juce::String& ratio);
    virtual void onShiftSubtitles(const juce::String& ratio);
	//VLC EvtListener
	virtual void timeChanged();
	virtual void paused();
	virtual void started();
	virtual void stopped();
};

#endif //VIDEO_COMPONENT
