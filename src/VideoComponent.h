
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "MenuTree.h"
#include "AppProportionnalComponent.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>
#include "LookNFeel.h"


#define BUFFER_DISPLAY
#undef BUFFER_DISPLAY

//==============================================================================

class AlternateControlComponent;
class ControlComponent   : public juce::Component, public AppProportionnalComponent
{
    juce::ScopedPointer<juce::Slider> m_slider;
    juce::ScopedPointer<juce::DrawableButton> m_playPauseButton;
    juce::ScopedPointer<juce::DrawableButton> m_stopButton;
    juce::ScopedPointer<juce::Drawable> m_playImage;
    juce::ScopedPointer<juce::Drawable> m_pauseImage;
    juce::ScopedPointer<juce::Drawable> m_stopImage;
    juce::ScopedPointer<AlternateControlComponent> m_alternateControlComponent;
	juce::String timeString;
public:
	ControlComponent();
	virtual ~ControlComponent();
    virtual void paint (juce::Graphics& g);
    virtual void resized();
    void setTime(int64_t time, int64_t len);

	void showPlayingControls();
	void showPausedControls();
	void hidePlayingControls();
	
	juce::Slider& slider(){return *m_slider.get();}
	juce::DrawableButton& playPauseButton(){return *m_playPauseButton.get();}
	juce::DrawableButton& stopButton(){return *m_stopButton.get();}
	AlternateControlComponent& alternateControlComponent(){return *m_alternateControlComponent.get();}

};
class VideoComponent   : public juce::Component , public juce::KeyListener, 
	
#ifdef BUFFER_DISPLAY
	DisplayCallback, 
#else
	juce::ComponentListener,
#endif
	juce::Slider::Listener, juce::Button::Listener, EventCallBack, InputCallBack, MouseInputCallBack
{
#ifdef BUFFER_DISPLAY
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
#else
    juce::ScopedPointer<juce::Component> videoComponent;
#endif
    juce::ScopedPointer<ControlComponent> controlComponent;
    juce::ScopedPointer<MenuTree> tree;
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
	LnF lnf;
	juce::ComponentDragger dragger;
    juce::ScopedPointer<juce::ResizableBorderComponent> resizableBorder;
    juce::ComponentBoundsConstrainer defaultConstrainer;
	bool browsingFiles;
	bool mousehookset;

public:
    VideoComponent();
    virtual ~VideoComponent();
	

	void play(char* path);
	void play();
	void pause();
	void stop();
	bool isFullScreen() const;
	void setFullScreen(bool fs = true);
	void switchFullScreen();
	void switchPlayPause();
	void showVolumeSlider();
	void showPlaybackSpeedSlider();

	juce::Drawable const* getItemImage() const { return itemImage; };
	juce::Drawable const* getFolderImage() const { return folderImage; };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage; };
	juce::Drawable const* getAudioImage() const { return audioImage; };
	juce::Drawable const* getDisplayImage() const { return displayImage; };
	juce::Drawable const* getSubtitlesImage() const { return subtitlesImage; };
	juce::Drawable const* getExitImage() const { return exitImage; };
	
#ifdef BUFFER_DISPLAY
	//VLC DiaplListener
	void *lock(void **p_pixels);
	void unlock(void *id, void *const *p_pixels);
	void display(void *id);
#else
    void componentMovedOrResized(Component& component,bool wasMoved, bool wasResized);
    void componentVisibilityChanged(Component& component);
#endif

	/////////////// MenuTree
	void onListFiles(MenuTreeItem& item, AbstractFileAction* fileMethod);

    void onOpen (MenuTreeItem& item, juce::File const& file);
    void onOpenSubtitle (MenuTreeItem& item, juce::File const& file);
    void onOpenPlaylist (MenuTreeItem& item, juce::File const& file);
	
	void onSubtitleSelect(MenuTreeItem& item, int i);
    void onSubtitleMenu (MenuTreeItem& item);
    void onCrop (MenuTreeItem& item, double ratio);
    void onCropSlider (MenuTreeItem& item);
    void onRate (MenuTreeItem& item, double rate);
    void onRateSlider (MenuTreeItem& item);
    void onSetAspectRatio(MenuTreeItem& item, juce::String ratio);
    void onShiftAudio(MenuTreeItem& item, double s);
    void onShiftAudioSlider(MenuTreeItem& item);
    void onShiftSubtitles(MenuTreeItem& item, double s);
    void onShiftSubtitlesSlider(MenuTreeItem& item);
    void onAudioVolume(MenuTreeItem& item, double volume);
    void onAudioVolumeSlider (MenuTreeItem& item);
	
    void onFullscreen(MenuTreeItem& item, bool fs);
	
	void onExit(MenuTreeItem& item);
	void onSoundOptions(MenuTreeItem& item);
	void onRatio(MenuTreeItem& item);
	void onVideoOptions(MenuTreeItem& item);
	void getRootITems(MenuTreeItem& item);
	/////////////// VLC EvtListener
	virtual void timeChanged();
	virtual void paused();
	virtual void started();
	virtual void stopped();
	virtual void vlcPopupCallback(bool show);
	virtual void vlcFullScreenControlCallback();
	virtual void vlcMouseMove(int x, int y, int button);
	virtual void vlcMouseClick(int x, int y, int button);
	
	void startedSynchronous();
	void stoppedSynchronous();
	

	/////////////// GUI CALLBACKS
    void paint (juce::Graphics& g);
	
    virtual void resized();
	void updateTimeAndSlider();

    virtual void sliderValueChanged (juce::Slider* slider);
    virtual void buttonClicked (juce::Button* button);
	void userTriedToCloseWindow();
	bool keyPressed (const juce::KeyPress& key,
								juce::Component* originatingComponent);
    void mouseDown (const juce::MouseEvent& e);
	void mouseDrag (const juce::MouseEvent& e);
private:
	
	void setBrowsingFiles(bool newBrowsingFiles = true);
};

#endif //VIDEO_COMPONENT
