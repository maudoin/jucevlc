
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "VLCMenuTree.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>


#define BUFFER_DISPLAY
#undef BUFFER_DISPLAY

//==============================================================================
class VideoComponent   : public juce::Component, 
	
#ifdef BUFFER_DISPLAY
	DisplayCallback, 
#endif
	juce::Slider::Listener, juce::Button::Listener, VLCMenuTreeListener, EventCallBack
{
#ifdef BUFFER_DISPLAY
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
#else
    juce::ScopedPointer<juce::Component> videoComponent;
#endif
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
	
#ifdef BUFFER_DISPLAY
	//VLC DiaplListener
	void *lock(void **p_pixels);
	void unlock(void *id, void *const *p_pixels);
	void display(void *id);
#endif

	void showPlayingControls();
	void showPausedControls();
	void hidePlayingControls();
	void updateTimeAndSlider();

    virtual void sliderValueChanged (juce::Slider* slider);
    virtual void buttonClicked (juce::Button* button);

	juce::Slider* getSlider();
	//MenuTreeListener
    virtual void onOpen (juce::File file);
    virtual void onOpenSubtitle (juce::File file);
    virtual void onOpenPlaylist (juce::File file);

    virtual void onCrop (float ratio);
    virtual void onRate (float rate);
    virtual void onSetAspectRatio(juce::String ratio);
    virtual void onShiftAudio(float ms);
    virtual void onShiftSubtitles(float ms);
    virtual void onAudioVolume(int volume);
	//VLC EvtListener
	virtual void timeChanged();
	virtual void paused();
	virtual void started();
	virtual void stopped();
};

#endif //VIDEO_COMPONENT
