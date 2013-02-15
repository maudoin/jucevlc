
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "MenuTree.h"
#include "AppProportionnalComponent.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>


#define BUFFER_DISPLAY
#undef BUFFER_DISPLAY

//==============================================================================
class VideoComponent;
class OverlayComponent;

class ControlComponent   : public juce::Component, public AppProportionnalComponent
{
    juce::ScopedPointer<juce::Slider> slider;
    juce::ScopedPointer<juce::DrawableButton> playPauseButton;
    juce::ScopedPointer<juce::DrawableButton> stopButton;
    juce::ScopedPointer<juce::Drawable> playImage;
    juce::ScopedPointer<juce::Drawable> pauseImage;
    juce::ScopedPointer<juce::Drawable> stopImage;
	juce::String timeString;
public:
	ControlComponent();
	virtual ~ControlComponent();
    virtual void paint (juce::Graphics& g);
    virtual void resized();
    void setTimeString(juce::String const& s);

	void showPlayingControls();
	void showPausedControls();
	void hidePlayingControls();

	friend class VideoComponent;
	friend class OverlayComponent;
};

class OverlayComponent   : public juce::Component
{
    juce::ScopedPointer<ControlComponent> controlComponent;
    juce::ScopedPointer<MenuTree> tree;
public:
	OverlayComponent();
	virtual ~OverlayComponent();

	void setScaleComponent(juce::Component* scaleComponent);
    virtual void resized();
	
	friend class VideoComponent;
};

class VideoComponent   : public juce::Component, 
	
#ifdef BUFFER_DISPLAY
	DisplayCallback, 
#else
	juce::Timer, juce::ComponentListener,
#endif
	juce::Slider::Listener, juce::Button::Listener, EventCallBack
{
#ifdef BUFFER_DISPLAY
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
#else
    juce::ScopedPointer<juce::Component> videoComponent;
#endif
    juce::ScopedPointer<OverlayComponent> overlayComponent;
    juce::CriticalSection imgCriticalSection;
	juce::ScopedPointer<VLCWrapper> vlc;
	bool sliderUpdating;
	bool videoUpdating;
    juce::ScopedPointer<juce::Drawable> itemImage;
    juce::ScopedPointer<juce::Drawable> folderImage;
    juce::ScopedPointer<juce::Drawable> folderShortcutImage;
    juce::ScopedPointer<juce::Drawable> audioImage;
    juce::ScopedPointer<juce::Drawable> displayImage;
    juce::ScopedPointer<juce::Drawable> subtitlesImage;
    juce::ScopedPointer<juce::Drawable> exitImage;
		
public:
    VideoComponent();
    virtual ~VideoComponent();
	
	void setScaleComponent(juce::Component* scaleComponent){overlayComponent->setScaleComponent(scaleComponent);};
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
#else
	void timerCallback();
    void componentMovedOrResized(Component& component,bool wasMoved, bool wasResized);
    void componentVisibilityChanged(Component& component);
#endif
	void updateTimeAndSlider();

    virtual void sliderValueChanged (juce::Slider* slider);
    virtual void buttonClicked (juce::Button* button);

	//MenuTree
    virtual void onOpen (MenuTreeItem& item, juce::File const& file);
    virtual void onOpenSubtitle (MenuTreeItem& item, juce::File const& file);
    virtual void onOpenPlaylist (MenuTreeItem& item, juce::File const& file);

    virtual void onCrop (MenuTreeItem& item, float ratio);
    virtual void onRate (MenuTreeItem& item, float rate);
    virtual void onSetAspectRatio(MenuTreeItem& item, juce::String ratio);
    virtual void onShiftAudio(MenuTreeItem& item, float ms);
    virtual void onShiftSubtitles(MenuTreeItem& item, float ms);
    virtual void onAudioVolume(MenuTreeItem& item, int volume);
	
    virtual void onFullscreen(MenuTreeItem& item, bool fs);
	//VLC EvtListener
	virtual void timeChanged();
	virtual void paused();
	virtual void started();
	virtual void stopped();

	
	juce::Drawable const* getItemImage() const { return itemImage; };
	juce::Drawable const* getFolderImage() const { return folderImage; };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage; };
	juce::Drawable const* getAudioImage() const { return audioImage; };
	juce::Drawable const* getDisplayImage() const { return displayImage; };
	juce::Drawable const* getSubtitlesImage() const { return subtitlesImage; };
	juce::Drawable const* getExitImage() const { return exitImage; };
};

#endif //VIDEO_COMPONENT
