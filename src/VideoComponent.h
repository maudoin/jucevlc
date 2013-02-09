
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "VLCMenuTree.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>


//==============================================================================
class VideoComponent   : public Component, public DisplayCallback, juce::Slider::Listener, public VLCMenuTreeListener
{
	ScopedPointer<juce::Image> img;
	ScopedPointer<juce::Image::BitmapData> ptr;
    juce::CriticalSection imgCriticalSection;
    ScopedPointer<Slider> slider;
    ScopedPointer<VLCMenuTree> tree;
public:
    VideoComponent();
    virtual ~VideoComponent();
	
	void setScaleComponent(Component* scaleComponent);
    void paint (Graphics& g);
	
    virtual void resized();


	void play(char* path);
	

	void *lock(void **p_pixels);

	void unlock(void *id, void *const *p_pixels);

	void display(void *id);
	void frameReady();
    void sliderValueChanged (Slider* slider);
	Slider* getSlider();
	//MenuTreeListener
    virtual void onOpen (const File& file, const MouseEvent& e);
    virtual void onOpenSubtitle (const File& file, const MouseEvent& e);
    virtual void onOpenPlaylist (const File& file, const MouseEvent& e);

    virtual void onCrop (const String& ratio);
    virtual void onSetAspectRatio(const String& ratio);
    virtual void onShiftAudio(const String& ratio);
    virtual void onShiftSubtitles(const String& ratio);
private:
	VLCWrapper vlc;
	bool sliderUpdating;
	bool videoUpdating;
};

#endif //VIDEO_COMPONENT
