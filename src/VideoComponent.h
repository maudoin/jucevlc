
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "VLCMenuTree.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>


//==============================================================================
class VideoComponent   : public juce::Component, public DisplayCallback, juce::Slider::Listener, public VLCMenuTreeListener
{
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
    juce::CriticalSection imgCriticalSection;
    juce::ScopedPointer<juce::Slider> slider;
    juce::ScopedPointer<VLCMenuTree> tree;
    juce::ScopedPointer<juce::Label> mediaTimeLabel;
    juce::ScopedPointer<juce::ImageButton> playPauseButton;
    juce::ScopedPointer<juce::ImageButton> stopButton;
public:
    VideoComponent();
    virtual ~VideoComponent();
	
	void setScaleComponent(juce::Component* scaleComponent);
    void paint (juce::Graphics& g);
	
    virtual void resized();


	void play(char* path);
	

	void *lock(void **p_pixels);

	void unlock(void *id, void *const *p_pixels);

	void display(void *id);
	void frameReady();
    void sliderValueChanged (juce::Slider* slider);
	juce::Slider* getSlider();
	//MenuTreeListener
    virtual void onOpen (const juce::File& file, const juce::MouseEvent& e);
    virtual void onOpenSubtitle (const juce::File& file, const juce::MouseEvent& e);
    virtual void onOpenPlaylist (const juce::File& file, const juce::MouseEvent& e);

    virtual void onCrop (const juce::String& ratio);
    virtual void onSetAspectRatio(const juce::String& ratio);
    virtual void onShiftAudio(const juce::String& ratio);
    virtual void onShiftSubtitles(const juce::String& ratio);
private:
	VLCWrapper vlc;
	bool sliderUpdating;
	bool videoUpdating;
};

#endif //VIDEO_COMPONENT
