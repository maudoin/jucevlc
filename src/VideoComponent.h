
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "VLCWrapper.h"
#include "ControlComponent.h"
#include "MenuTree.h"
#include "AppProportionnalComponent.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>
#include <set>
#include "LookNFeel.h"


#define BUFFER_DISPLAY
#undef BUFFER_DISPLAY

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
    juce::ScopedPointer<juce::Component> vlcNativePopupComponent;
#endif
    juce::ScopedPointer<juce::Component> m_toolTip;
    juce::ScopedPointer<ControlComponent> controlComponent;
    juce::ScopedPointer<MenuTree> tree;
    juce::CriticalSection imgCriticalSection;
	juce::ScopedPointer<VLCWrapper> vlc;
	bool sliderUpdating;
	bool videoUpdating;
    juce::ScopedPointer<juce::Drawable> itemImage;
    juce::ScopedPointer<juce::Drawable> folderImage;
    juce::ScopedPointer<juce::Drawable> playlistImage;
    juce::ScopedPointer<juce::Drawable> folderShortcutImage;
    juce::ScopedPointer<juce::Drawable> hideFolderShortcutImage;
    juce::ScopedPointer<juce::Drawable> audioImage;
    juce::ScopedPointer<juce::Drawable> displayImage;
    juce::ScopedPointer<juce::Drawable> subtitlesImage;
    juce::ScopedPointer<juce::Drawable> settingsImage;
    juce::ScopedPointer<juce::Drawable> exitImage;
    juce::ScopedPointer<juce::Drawable> speedImage;
    juce::ScopedPointer<juce::Drawable> audioShiftImage;
    juce::ScopedPointer<juce::Drawable> clockImage;
    juce::Image appImage;
	LnF lnf;
    juce::ScopedPointer<juce::Component> titleBar;
    juce::ScopedPointer<juce::ResizableBorderComponent> resizableBorder;
    juce::ComponentBoundsConstrainer defaultConstrainer;
	bool browsingFiles;
	bool mousehookset;
	juce::int64 lastMouseMoveMovieTime;
	juce::PropertiesFile m_settings;
	juce::PropertiesFile m_mediaTimes;
	juce::StringArray m_shortcuts;
    juce::ScopedPointer<vf::GuiCallQueue> invokeLater;
	bool m_canHideOSD;
	bool m_autoSubtitlesHeight;
	std::set<juce::String> m_videoExtensions;
	std::set<juce::String> m_playlistExtensions;
	std::set<juce::String> m_subtitlesExtensions;
	std::vector< std::set<juce::String> > m_suportedExtensions;

public:
    VideoComponent();
    virtual ~VideoComponent();
	

	void appendAndPlay(std::string const&path);
	void play();
	void pause();
	void stop();
	bool isFullScreen() const;
	void setFullScreen(bool fs = true);
	void switchFullScreen();
	void switchPlayPause();
	void showVolumeSlider();
	void showPlaybackSpeedSlider();
	void showZoomSlider();
	void showAudioOffsetSlider ();
	void showSubtitlesOffsetSlider ();

	
	juce::Drawable const* getIcon(juce::File const&);
	juce::Drawable const* getItemImage() const { return itemImage; };
	juce::Drawable const* getFolderImage() const { return folderImage; };
	juce::Drawable const* getPlaylistImage() const { return playlistImage; };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage; };
	juce::Drawable const* getAudioImage() const { return audioImage; };
	juce::Drawable const* getDisplayImage() const { return displayImage; };
	juce::Drawable const* getSubtitlesImage() const { return subtitlesImage; };
	juce::Drawable const* getExitImage() const { return exitImage; };
	juce::Drawable const* getSettingsImage() const { return settingsImage; };
	
#ifdef BUFFER_DISPLAY
	//VLC DiaplListener
	void *vlcLock(void **p_pixels);
	void vlcUnlock(void *id, void *const *p_pixels);
	void vlcDisplay(void *id);
#else
    void componentMovedOrResized(Component& component,bool wasMoved, bool wasResized);
    void componentVisibilityChanged(Component& component);
#endif

	/////////////// MenuTree
	void onMenuListMediaFiles(MenuTreeItem& item);
	void onMenuListSubtitlesFiles(MenuTreeItem& item);
	void onMenuListFiles(MenuTreeItem& item, AbstractFileAction* fileMethod);
	void onMenuListRootFiles(MenuTreeItem& item, AbstractFileAction* fileMethod);
	void onMenuListUPNPFiles(MenuTreeItem& item, std::vector<std::string> path);
	void onMenuListFavorites(MenuTreeItem& item, AbstractFileAction* fileMethod);
	
	void onMenuAddFavorite (MenuTreeItem& item, juce::String path);
	void onMenuRemoveFavorite (MenuTreeItem& item, juce::String path);
	void mayPurgeFavorites();
	void writeFavorites();
    void onMenuOpen (MenuTreeItem& item, juce::File const& file);
    void onMenuOpenUnconditionnal (MenuTreeItem& item,  juce::String path);
    void onMenuQueue (MenuTreeItem& item,  juce::String path);
    void onMenuOpenSubtitle (MenuTreeItem& item, juce::File const& file);
    void onMenuOpenPlaylist (MenuTreeItem& item, juce::File const& file);
	

	void onMenuVoutIntOption (MenuTreeItem& item, juce::String label, std::string option, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep = 0.);
	void onVLCOptionIntSelect(MenuTreeItem& item, std::string, int i);
    void onVLCOptionIntListMenu (MenuTreeItem& item, std::string);
    void onVLCOptionIntRangeMenu (MenuTreeItem& item, std::string, const char* format, int min, int max, int defaultVal);
	void onVLCOptionStringSelect(MenuTreeItem& item, std::string, std::string i);
    void onVLCOptionStringMenu (MenuTreeItem& item, std::string);
	void onMenuSubtitlePositionMode(MenuTreeItem& item, bool automatic);
	void onMenuSubtitlePositionMode(MenuTreeItem& item);
	void onMenuSubtitleSelect(MenuTreeItem& item, int i);
    void onMenuSubtitlePosition (MenuTreeItem& item);
	void onVLCOptionColor(MenuTreeItem& item, std::string);
    void onMenuSubtitleMenu (MenuTreeItem& item);
    void onMenuZoom (MenuTreeItem& item, double ratio);
    void onMenuCrop (MenuTreeItem& item, juce::String  crop);
    void onMenuAutoCrop (MenuTreeItem& item);
    void onMenuCropList (MenuTreeItem& item);
    void onMenuRate (MenuTreeItem& item, double rate);
    void onMenuRateSlider (MenuTreeItem& item);
    void onMenuSetAspectRatio(MenuTreeItem& item, juce::String ratio);
    void onMenuShiftAudio(double s);
    void onMenuShiftAudioSlider(MenuTreeItem& item);
    void onMenuShiftSubtitles(double s);
    void onMenuShiftSubtitlesSlider(MenuTreeItem& item);
	void onVLCAoutStringSelect(MenuTreeItem& item, std::string, std::string, std::string i);
    void onVLCAoutStringSelectListMenu (MenuTreeItem& item, std::string, std::string);
    void onMenuAudioVolume(MenuTreeItem& item, double volume);
    void onMenuAudioVolumeSlider (MenuTreeItem& item);
	
    void onMenuAudioTrack (MenuTreeItem& item, int id);
    void onMenuAudioTrackList (MenuTreeItem& item);
    void onMenuVideoTrack (MenuTreeItem& item, int id);
    void onMenuVideoTrackList (MenuTreeItem& item);
	void onMenuVideoContrast (MenuTreeItem& item);
	void onMenuVideoBrightness (MenuTreeItem& item);
	void onMenuVideoHue (MenuTreeItem& item);
	void onMenuVideoSaturation (MenuTreeItem& item);
	void onMenuVideoGamma (MenuTreeItem& item);
	void onMenuVideoAdjust (MenuTreeItem& item);

    void onMenuFullscreen(MenuTreeItem& item, bool fs);
	
	void onMenuExit(MenuTreeItem& item);
	void onMenuSoundOptions(MenuTreeItem& item);
	void onMenuRatio(MenuTreeItem& item);
	void onMenuVideoAdjustOptions(MenuTreeItem& item);
	void onMenuVideoOptions(MenuTreeItem& item);
	void onPlaylistItem(MenuTreeItem& item, int index);
	void onShowPlaylist(MenuTreeItem& item);
	void onLanguageOptions(MenuTreeItem& item);
	void onLanguageSelect(MenuTreeItem& item, std::string lang);
	void onSetPlayerFonSize(MenuTreeItem& item, int size);
	void onPlayerFonSize(MenuTreeItem& item);
	void onSetVLCOptionInt(MenuTreeItem& item, std::string name, int enable);
	void onSetVLCOption(MenuTreeItem& item, std::string name, bool enable);
	void onPlayerOptions(MenuTreeItem& item);
	void onMenuRoot(MenuTreeItem& item);
	/////////////// VLC EvtListener
	virtual void vlcTimeChanged();
	virtual void vlcPaused();
	virtual void vlcStarted();
	virtual void vlcStopped();
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
    void mouseMove (const juce::MouseEvent& e);
    void mouseExit (const juce::MouseEvent& e);
	
	//void minimisationStateChanged (bool isNowMinimised){if(!isNowMinimised)resized();}
    void broughtToFront();

	enum SliderModeButton
	{
		  E_POPUP_ITEM_VOLUME_SLIDER = 1
		, E_POPUP_ITEM_SUBTITLES_DELAY_SLIDER
		, E_POPUP_ITEM_VOLUME_DELAY_SLIDER
		, E_POPUP_ITEM_PLAY_SPEED_SLIDER
		, E_POPUP_ITEM_SHOW_CURRENT_TIME
	};
	void auxilliarySliderModeButton(int result);
private:
	
	void handleIdleTimeAndControlsVisibility();
	void setBrowsingFiles(bool newBrowsingFiles = true);
	void saveCurrentMediaTime();
	void initBoolSetting(const char* name);
	void initIntSetting(const char* name);
	void initIntSetting(const char* name, int defaultVal);
	void initStrSetting(const char* name);
	void initFromSettings();
	void initFromMediaDependantSettings();
	void setMenuTreeVisibleAndUpdateMenuButtonIcon(bool visible);
	void updateSubComponentsBounds();
	void forceSetVideoTime(int64_t start);
	void forceSetVideoTime(std::string const& name);
	void restart(MenuTreeItem& item);
};

#endif //VIDEO_COMPONENT
