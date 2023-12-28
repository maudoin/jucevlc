
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include <JuceHeader.h>
#include "VLCWrapper.h"
#include "ControlComponent.h"
#include "AbstractMenu.h"
#include "AppProportionnalComponent.h"
#include <sstream>
#include <set>
#include "LookNFeel.h"


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
	std::unique_ptr<juce::Image> img;
	std::unique_ptr<juce::Image::BitmapData> ptr;
#else
    std::unique_ptr<juce::Component> vlcNativePopupComponent;
#endif
    std::unique_ptr<juce::Component> m_toolTip;
    std::unique_ptr<ControlComponent> controlComponent;
    std::unique_ptr<AbstractMenu> m_optionsMenu;
    juce::CriticalSection imgCriticalSection;
	std::unique_ptr<VLCWrapper> vlc;
	std::unique_ptr<BackgoundUPNP> vlcMediaUPNPList;
	bool sliderUpdating;
	bool videoUpdating;
    std::unique_ptr<juce::Drawable> itemImage;
    std::unique_ptr<juce::Drawable> folderImage;
    std::unique_ptr<juce::Drawable> playlistImage;
    std::unique_ptr<juce::Drawable> folderShortcutImage;
    std::unique_ptr<juce::Drawable> hideFolderShortcutImage;
    std::unique_ptr<juce::Drawable> audioImage;
    std::unique_ptr<juce::Drawable> displayImage;
    std::unique_ptr<juce::Drawable> subtitlesImage;
    std::unique_ptr<juce::Drawable> settingsImage;
    std::unique_ptr<juce::Drawable> exitImage;
    std::unique_ptr<juce::Drawable> speedImage;
    std::unique_ptr<juce::Drawable> audioShiftImage;
    std::unique_ptr<juce::Drawable> clockImage;
    std::unique_ptr<juce::Drawable> asFrontpageImage;
    std::unique_ptr<juce::Drawable> likeAddImage;
    std::unique_ptr<juce::Drawable> likeRemoveImage;
    std::unique_ptr<juce::Drawable> playAllImage;
    std::unique_ptr<juce::Drawable> addAllImage;
    juce::Image appImage;
	LnF lnf;
    std::unique_ptr<TitleComponent> titleBar;
    std::unique_ptr<juce::ResizableBorderComponent> resizableBorder;
    juce::ComponentBoundsConstrainer defaultConstrainer;
	bool browsingFiles;
	juce::int64 lastMouseMoveMovieTime;
	juce::PropertiesFile m_settings;
	juce::PropertiesFile m_mediaTimes;
	juce::StringArray m_shortcuts;
    std::unique_ptr<InvokeLater> invokeLater;
	bool m_canHideOSD;
	bool m_autoSubtitlesHeight;
	juce::TimeSliceThread m_backgroundTasks;

	std::unique_ptr<AbstractMenu> m_fileMenu;

public:
    VideoComponent();
    virtual ~VideoComponent();


    int useTimeSlice() override;

	void appendAndPlay(std::string const&path);
	void play();
	void pause();
	void stop();
	void rewindTime ();
	void advanceTime ();
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

	juce::Drawable const* getItemImage() const { return itemImage.get(); };
	juce::Drawable const* getFolderImage() const { return folderImage.get(); };
	juce::Drawable const* getPlaylistImage() const { return playlistImage.get(); };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage.get(); };
	juce::Drawable const* getAudioImage() const { return audioImage.get(); };
	juce::Drawable const* getDisplayImage() const { return displayImage.get(); };
	juce::Drawable const* getSubtitlesImage() const { return subtitlesImage.get(); };
	juce::Drawable const* getExitImage() const { return exitImage.get(); };
	juce::Drawable const* getSettingsImage() const { return settingsImage.get(); };

#ifdef BUFFER_DISPLAY
	//VLC DiaplListener
	void *vlcLock(void **p_pixels);
	void vlcUnlock(void *id, void *const *p_pixels);
	void vlcDisplay(void *id);
#else
    void componentMovedOrResized(Component& component,bool wasMoved, bool wasResized) override;
    void componentVisibilityChanged(Component& component) override;
#endif

	using FileMethod = AbstractMenu::FileMethod;

	/////////////// MenuTree
	void onFileMenuRoot(MenuComponentValue const&, FileMethod fileMethod);
	void onMenuLoadSubtitle(MenuComponentValue const&, FileMethod fileMethod);
	void onMenuListRootFiles(MenuComponentValue const&, FileMethod fileMethod);
	void onMenuListUPNPFiles(MenuComponentValue const&, std::vector<std::string> path);
	void onMenuListFavorites(MenuComponentValue const&, FileMethod fileMethod);

	void onMenuAddFavorite (MenuComponentValue const&, juce::String path);
	void onMenuRemoveFavorite (MenuComponentValue const&, juce::String path);
	void mayPurgeFavorites();
	void writeFavorites();
    void onMenuOpenFile (MenuComponentValue const&, juce::File file);
    void onMenuOpenFolder (MenuComponentValue const&, juce::File file);
    void onMenuOpenUnconditionnal (MenuComponentValue const&,  juce::String path);
    void onMenuQueue (MenuComponentValue const&,  juce::String path);
    void onMenuOpenSubtitleFolder (MenuComponentValue const&, juce::File file);
    void onMenuOpenSubtitleFile (MenuComponentValue const&, juce::File file);
    void onMenuOpenPlaylist (MenuComponentValue const&, juce::File file);

	void onMenuVoutIntOption (MenuComponentValue const&, juce::String label, std::string option, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep = 0.);
	void onVLCOptionIntSelect(MenuComponentValue const&, std::string, int i);
    void onVLCOptionIntListMenu (MenuComponentValue const&, std::string);
    void onVLCOptionIntRangeMenu (MenuComponentValue const&, const char* );
	void onVLCOptionStringSelect(MenuComponentValue const&, std::string, std::string i);
    void onVLCOptionStringMenu (MenuComponentValue const&, std::string);
	void onVLCAudioChannelSelect(MenuComponentValue const&);
	void onVLCAudioOutputDeviceSelect(MenuComponentValue const&, std::string output, std::string device);
    void onVLCAudioOutputSelect(MenuComponentValue const&, std::string, std::vector< std::pair<std::string, std::string> >);
    void onVLCAudioOutputList(MenuComponentValue const&);
	void onMenuSearchOpenSubtitles(MenuComponentValue const&);
	void onMenuSearchSubtitleSeeker(MenuComponentValue const&);
	void onMenuSearchSubtitlesManually(MenuComponentValue const&, juce::String lang);
	void onMenuSearchOpenSubtitlesSelectLanguage(MenuComponentValue const&, juce::String name);
	void onMenuSearchOpenSubtitles(MenuComponentValue const&, juce::String lang, juce::String name);
	void onMenuSearchSubtitleSeeker(MenuComponentValue const&, juce::String name);
	void onMenuSearchSubtitleSeekerImdb(MenuComponentValue const&, juce::String imdb, bool tvEpisode, int season, int episode);
	void onMenuSearchSubtitleSeekerImdbLang(MenuComponentValue const&, juce::String imdb, juce::String lang, bool tvEpisode, int season, int episode);
	void onMenuDowloadOpenSubtitle(MenuComponentValue const&, juce::String url);
	void onMenuDowloadSubtitleSeeker(MenuComponentValue const&, juce::String url, juce::String site);
	void onMenuSubtitlePositionMode(MenuComponentValue const&, bool automatic);
	void onMenuSubtitlePositionMode(MenuComponentValue const&);
	void onMenuSubtitleSelect(MenuComponentValue const&, int i);
    void onMenuSubtitlePosition (MenuComponentValue const&);
	void onVLCOptionColor(MenuComponentValue const&, std::string);
    void onMenuSubtitleMenu (MenuComponentValue const&);
    void onMenuZoom (MenuComponentValue const&, double ratio);
    void onMenuCrop (MenuComponentValue const&, juce::String  crop);
    void onMenuAutoCrop (MenuComponentValue const&);
    void onMenuCropList (MenuComponentValue const&);
    void onMenuRate (MenuComponentValue const&, double rate);
    void onMenuRateListAndSlider (MenuComponentValue const&);
    void onMenuSetAspectRatio(MenuComponentValue const&, juce::String ratio);
    void onMenuShiftAudio(double s);
    void onMenuShiftAudioSlider(MenuComponentValue const&);
    void onMenuShiftSubtitles(double s);
    void onMenuShiftSubtitlesSlider(MenuComponentValue const&);
	void onVLCAoutStringSelect(MenuComponentValue const&, std::string, std::string, std::string i);
    void onVLCAoutStringSelectListMenu (MenuComponentValue const&, std::string, std::string);
    void onMenuAudioVolume(MenuComponentValue const&, double volume);
    void onMenuAudioVolumeListAndSlider (MenuComponentValue const&);

    void onMenuAudioTrack (MenuComponentValue const&, int id);
    void onMenuAudioTrackList (MenuComponentValue const&);
    void onMenuVideoTrack (MenuComponentValue const&, int id);
    void onMenuVideoTrackList (MenuComponentValue const&);
	void onMenuVideoContrast (MenuComponentValue const&);
	void onMenuVideoBrightness (MenuComponentValue const&);
	void onMenuVideoHue (MenuComponentValue const&);
	void onMenuVideoSaturation (MenuComponentValue const&);
	void onMenuVideoGamma (MenuComponentValue const&);
	void onMenuVideoAdjust (MenuComponentValue const&);

    void onMenuFullscreen(MenuComponentValue const&, bool fs);

	void onMenuExit(MenuComponentValue const&);
	void onMenuExitConfirmation(MenuComponentValue const&);
	void onMenuSoundOptions(MenuComponentValue const&);
	void onMenuRatio(MenuComponentValue const&);
	void onMenuVideoAdjustOptions(MenuComponentValue const&);
	void onMenuVideoOptions(MenuComponentValue const&);
	void onPlaylistItem(MenuComponentValue const&, int index);
	void onShowPlaylist(MenuComponentValue const&);
	void onLanguageOptions(MenuComponentValue const&);
	void onLanguageSelect(MenuComponentValue const&, std::string lang);
	void onSetPlayerFonSize(MenuComponentValue const&, int size);
	void onPlayerFonSize(MenuComponentValue const&);
	void onSetVLCOptionInt(MenuComponentValue const&, std::string name, int enable);
	void onSetVLCOption(MenuComponentValue const&, std::string name, bool enable);
	void onPlayerOptions(MenuComponentValue const&);
	void onOptionMenuRoot(MenuComponentValue const&);
	/////////////// VLC EvtListener
	void vlcTimeChanged(int64_t newTime) override;
	void vlcPaused() override;
	void vlcStarted() override;
	void vlcStopped() override;
	void vlcPopupCallback(bool show) override;
	void vlcFullScreenControlCallback() override;
	void vlcMouseMove(int x, int y, int button) override;
	void vlcMouseClick(int x, int y, int button) override;

	void startedSynchronous();
	void stoppedSynchronous();


	/////////////// GUI CALLBACKS
    void paint (juce::Graphics& g) override;

    void resized() override;
	void updateTimeAndSlider(int64_t newTime);

    void sliderValueChanged (juce::Slider* slider) override;
    void buttonClicked (juce::Button* button) override;
	void userTriedToCloseWindow() override;
	using juce::Component::keyPressed;
	bool keyPressed (const juce::KeyPress& key,
								juce::Component* originatingComponent) override;
    void mouseDown (const juce::MouseEvent& e) override;
	void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseWheelMove (const juce::MouseEvent& e,
                                 const juce::MouseWheelDetails& wheel) override;

	//void minimisationStateChanged (bool isNowMinimised){if(!isNowMinimised)resized();}
    void broughtToFront() override;

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

    bool downloadedSubtitleSeekerResult(MenuComponentValue const&, juce::String const& resultSite,
                                                    char* cstr,
                                                     juce::String const& siteTarget,
                                                     std::string const& match,
                                                     std::string const& downloadURLPattern );

};

#endif //VIDEO_COMPONENT
