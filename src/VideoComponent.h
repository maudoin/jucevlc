
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "AppConfig.h"
#include "juce.h"
#include "VLCWrapper.h"
#include "ControlComponent.h"
#include "AbstractMenu.h"
#include "AppProportionnalComponent.h"
#include <sstream>
#include <set>
#include "LookNFeel.h"
#include "IconMenu.h"


#define BUFFER_DISPLAY
#undef BUFFER_DISPLAY

class InvokeLater;
class TitleComponent;
class BackgoundUPNP;
class VideoComponent   : public juce::Component , public juce::KeyListener, 
	
#ifdef BUFFER_DISPLAY
	DisplayCallback, 
#else
	juce::ComponentListener,
#endif
	juce::Slider::Listener, juce::Button::Listener, EventCallBack, InputCallBack, MouseInputCallBack, juce::TimeSliceClient
{
#ifdef BUFFER_DISPLAY
	juce::ScopedPointer<juce::Image> img;
	juce::ScopedPointer<juce::Image::BitmapData> ptr;
#else
    juce::ScopedPointer<juce::Component> vlcNativePopupComponent;
#endif
    juce::ScopedPointer<juce::Component> m_toolTip;
    juce::ScopedPointer<ControlComponent> controlComponent;
    juce::ScopedPointer<AbstractMenu> menu;
    juce::CriticalSection imgCriticalSection;
	juce::ScopedPointer<VLCWrapper> vlc;
	juce::ScopedPointer<BackgoundUPNP> vlcMediaUPNPList;
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
    juce::ScopedPointer<juce::Drawable> asFrontpageImage;
    juce::ScopedPointer<juce::Drawable> likeAddImage;
    juce::ScopedPointer<juce::Drawable> likeRemoveImage;
    juce::ScopedPointer<juce::Drawable> playAllImage;
    juce::ScopedPointer<juce::Drawable> addAllImage;
    juce::Image appImage;
	LnF lnf;
    juce::ScopedPointer<TitleComponent> titleBar;
    juce::ScopedPointer<juce::ResizableBorderComponent> resizableBorder;
    juce::ComponentBoundsConstrainer defaultConstrainer;
	bool browsingFiles;
	bool mousehookset;
	juce::int64 lastMouseMoveMovieTime;
	juce::PropertiesFile m_settings;
	juce::PropertiesFile m_mediaTimes;
	juce::StringArray m_shortcuts;
    juce::ScopedPointer<InvokeLater> invokeLater;
	bool m_canHideOSD;
	bool m_autoSubtitlesHeight;
	std::set<juce::String> m_videoExtensions;
	std::set<juce::String> m_playlistExtensions;
	std::set<juce::String> m_subtitlesExtensions;
	std::vector< std::set<juce::String> > m_suportedExtensions;
	juce::TimeSliceThread m_backgroundTasks;

	IconMenu m_iconMenu;

public:
    VideoComponent();
    virtual ~VideoComponent();
	
	
    int useTimeSlice();

	void appendAndPlay(std::string const&path);
	void play();
	void pause();
	void stop();
	bool isFullScreen() const;
	void setFullScreen(bool fs = true);
	void switchFullScreen();
	void switchPlayPause();
	void showVolumeSlider(double value);
	void showVolumeSlider();
	void showPlaybackSpeedSlider();
	void showZoomSlider();
	void showAudioOffsetSlider ();
	void showSubtitlesOffsetSlider ();

	
	juce::Drawable const* getIcon(juce::String const&);
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
	
	typedef void (VideoComponent::*FileMethod)(AbstractMenuItem&, juce::File);
	
	/////////////// MenuTree
	void onMenuListFiles(AbstractMenuItem& item, FileMethod fileMethod);
	void onMenuListRootFiles(AbstractMenuItem& item, FileMethod fileMethod);
	void onMenuListUPNPFiles(AbstractMenuItem& item, std::vector<std::string> path);
	void onMenuListFavorites(AbstractMenuItem& item, FileMethod fileMethod);
	
	void onMenuSetFrontPage (AbstractMenuItem& item, juce::String path);
	void onMenuAddFavorite (AbstractMenuItem& item, juce::String path);
	void onMenuRemoveFavorite (AbstractMenuItem& item, juce::String path);
	void mayPurgeFavorites();
	void writeFavorites();
    void onMenuOpenFile (AbstractMenuItem& item, juce::File file);
    void onMenuOpenFolder (AbstractMenuItem& item, juce::File file);
    void onMenuOpenUnconditionnal (AbstractMenuItem& item,  juce::String path);
    void onMenuQueue (AbstractMenuItem& item,  juce::String path);
    void onMenuOpenSubtitleFolder (AbstractMenuItem& item, juce::File file);
    void onMenuOpenSubtitleFile (AbstractMenuItem& item, juce::File file);
    void onMenuOpenPlaylist (AbstractMenuItem& item, juce::File file);
	

	void onMenuVoutIntOption (AbstractMenuItem& item, juce::String label, std::string option, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep = 0.);
	void onVLCOptionIntSelect(AbstractMenuItem& item, std::string, int i);
    void onVLCOptionIntListMenu (AbstractMenuItem& item, std::string);
    void onVLCOptionIntRangeMenu (AbstractMenuItem& item, std::string, const char* format, int min, int max, int defaultVal);
	void onVLCOptionStringSelect(AbstractMenuItem& item, std::string, std::string i);
    void onVLCOptionStringMenu (AbstractMenuItem& item, std::string);
	void onVLCAudioChannelSelect(AbstractMenuItem& item);
	void onVLCAudioOutputDeviceSelect(AbstractMenuItem& item, std::string output, std::string device);
    void onVLCAudioOutputSelect(AbstractMenuItem& item, std::string, std::vector< std::pair<std::string, std::string> >);
    void onVLCAudioOutputList(AbstractMenuItem& item);
	void onMenuSearchOpenSubtitles(AbstractMenuItem& item);
	void onMenuSearchSubtitleSeeker(AbstractMenuItem& item);
	void onMenuSearchSubtitlesManually(AbstractMenuItem& item, juce::String lang);
	void onMenuSearchOpenSubtitlesSelectLanguage(AbstractMenuItem& item, juce::String name);
	void onMenuSearchOpenSubtitles(AbstractMenuItem& item, juce::String lang, juce::String name);
	void onMenuSearchSubtitleSeeker(AbstractMenuItem& item, juce::String name);
	void onMenuSearchSubtitleSeekerImdb(AbstractMenuItem& item, juce::String imdb);
	void onMenuSearchSubtitleSeekerImdbLang(AbstractMenuItem& item, juce::String imdb, juce::String lang);
	void onMenuDowloadOpenSubtitle(AbstractMenuItem& item, juce::String url);
	void onMenuDowloadSubtitleSeeker(AbstractMenuItem& item, juce::String url, juce::String site);
	void onMenuSubtitlePositionMode(AbstractMenuItem& item, bool automatic);
	void onMenuSubtitlePositionMode(AbstractMenuItem& item);
	void onMenuSubtitleSelect(AbstractMenuItem& item, int i);
    void onMenuSubtitlePosition (AbstractMenuItem& item);
	void onVLCOptionColor(AbstractMenuItem& item, std::string);
    void onMenuSubtitleMenu (AbstractMenuItem& item);
    void onMenuZoom (AbstractMenuItem& item, double ratio);
    void onMenuCrop (AbstractMenuItem& item, juce::String  crop);
    void onMenuAutoCrop (AbstractMenuItem& item);
    void onMenuCropList (AbstractMenuItem& item);
    void onMenuRate (AbstractMenuItem& item, double rate);
    void onMenuRateListAndSlider (AbstractMenuItem& item);
    void onMenuSetAspectRatio(AbstractMenuItem& item, juce::String ratio);
    void onMenuShiftAudio(double s);
    void onMenuShiftAudioSlider(AbstractMenuItem& item);
    void onMenuShiftSubtitles(double s);
    void onMenuShiftSubtitlesSlider(AbstractMenuItem& item);
	void onVLCAoutStringSelect(AbstractMenuItem& item, std::string, std::string, std::string i);
    void onVLCAoutStringSelectListMenu (AbstractMenuItem& item, std::string, std::string);
    void onMenuAudioVolume(AbstractMenuItem& item, double volume);
    void onMenuAudioVolumeListAndSlider (AbstractMenuItem& item);
	
    void onMenuAudioTrack (AbstractMenuItem& item, int id);
    void onMenuAudioTrackList (AbstractMenuItem& item);
    void onMenuVideoTrack (AbstractMenuItem& item, int id);
    void onMenuVideoTrackList (AbstractMenuItem& item);
	void onMenuVideoContrast (AbstractMenuItem& item);
	void onMenuVideoBrightness (AbstractMenuItem& item);
	void onMenuVideoHue (AbstractMenuItem& item);
	void onMenuVideoSaturation (AbstractMenuItem& item);
	void onMenuVideoGamma (AbstractMenuItem& item);
	void onMenuVideoAdjust (AbstractMenuItem& item);

    void onMenuFullscreen(AbstractMenuItem& item, bool fs);
	
	void onMenuExit(AbstractMenuItem& item);
	void onMenuSoundOptions(AbstractMenuItem& item);
	void onMenuRatio(AbstractMenuItem& item);
	void onMenuVideoAdjustOptions(AbstractMenuItem& item);
	void onMenuVideoOptions(AbstractMenuItem& item);
	void onPlaylistItem(AbstractMenuItem& item, int index);
	void onShowPlaylist(AbstractMenuItem& item);
	void onLanguageOptions(AbstractMenuItem& item);
	void onLanguageSelect(AbstractMenuItem& item, std::string lang);
	void onSetPlayerFonSize(AbstractMenuItem& item, int size);
	void onPlayerFonSize(AbstractMenuItem& item);
	void onSetVLCOptionInt(AbstractMenuItem& item, std::string name, int enable);
	void onSetVLCOption(AbstractMenuItem& item, std::string name, bool enable);
	void onPlayerOptions(AbstractMenuItem& item);
	void onMenuRoot(AbstractMenuItem& item);
	/////////////// VLC EvtListener
	virtual void vlcTimeChanged(int64_t newTime);
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
	void updateTimeAndSlider(int64_t newTime);

    virtual void sliderValueChanged (juce::Slider* slider);
    virtual void buttonClicked (juce::Button* button);
	void userTriedToCloseWindow();
	bool keyPressed (const juce::KeyPress& key,
								juce::Component* originatingComponent);
    void mouseDown (const juce::MouseEvent& e);
	void mouseDrag (const juce::MouseEvent& e);
    void mouseMove (const juce::MouseEvent& e);
    void mouseExit (const juce::MouseEvent& e);
    void mouseWheelMove (const juce::MouseEvent& e,
                                 const juce::MouseWheelDetails& wheel);
	
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
	bool isFrontpageVisible();
};

#endif //VIDEO_COMPONENT
