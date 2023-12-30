
#ifndef PLAYER_MENUS_H
#define PLAYER_MENUS_H

#include <JuceHeader.h>
#include "VLCWrapper.h"
#include "AbstractMenu.h"
#include <sstream>
#include <set>


namespace
{
class BackgoundUPNP;
}


class PlayerMenus
{
public:
	struct ViewHandler
	{
		virtual ~ViewHandler() = default;
		virtual void handleFullRelayout() = 0;
		virtual void handleControlRelayout() = 0;
		virtual void closeFileMenu() = 0;
		virtual void playPlayListItem(int index, std::string const& name) = 0;
		virtual bool isFullScreen()const = 0;
		virtual void setFullScreen(bool fs = true) = 0;
		virtual void setBrowsingFiles(bool newBrowsingFiles = true) = 0;
	};
private:
	//all thos pointers are fine, they are owned by the instance that owns this
	VLCWrapper* vlc;
	ViewHandler& m_viewHandler;
	AbstractMenu* m_fileMenu;
    AbstractMenu* m_optionsMenu;

	std::unique_ptr<BackgoundUPNP> vlcMediaUPNPList;

	juce::PropertiesFile m_settings;
	juce::PropertiesFile m_mediaTimes;
	juce::StringArray m_shortcuts;
	bool m_autoSubtitlesHeight;

public:
    PlayerMenus(std::unique_ptr<VLCWrapper> const& vlc, ViewHandler* viewHandler, std::unique_ptr<AbstractMenu>& fileMenu, std::unique_ptr<AbstractMenu>& optionsMenu);
    virtual ~PlayerMenus();

	VLCWrapper& player();

	bool isAutoSubtitlesHeight()const;

	double getSavedVolume()const;
	int getMediaSavedTime(std::string const& name) const;
	void saveFullscreenState(bool);
	void saveCurrentMediaTime();
	void initFromSettings();
	void initFromMediaDependantSettings();

	void appendAndPlay(std::string const&path);

	using FileMethod = AbstractMenu::FileMethod;

	/////////////// MenuTree
	void onFileMenuRoot(MenuComponentValue const&, FileMethod const& fileMethod);
	void onMenuLoadSubtitle(MenuComponentValue const&, FileMethod const& fileMethod);
	void onMenuListRootFiles(MenuComponentValue const&, FileMethod const& fileMethod);
	void onMenuListUPNPFiles(MenuComponentValue const&, std::vector<std::string> const& path);
	void onMenuListFavorites(MenuComponentValue const&, FileMethod const& fileMethod);

	void onMenuAddFavorite (MenuComponentValue const&, juce::String const& path);
	void onMenuRemoveFavorite (MenuComponentValue const&, juce::String const& path);
	void mayPurgeFavorites();
	void writeFavorites();
    void onMenuOpenFile (MenuComponentValue const&, juce::File const& file);
    void onMenuOpenFolder (MenuComponentValue const&, juce::File const& file);
    void onMenuOpenUnconditionnal (MenuComponentValue const&,  juce::String const& path);
    void onMenuQueue (MenuComponentValue const&,  juce::String const& path);
    void onMenuOpenSubtitleFolder (MenuComponentValue const&, juce::File const& file);
    void onMenuOpenSubtitleFile (MenuComponentValue const&, juce::File const& file);
    void onMenuOpenPlaylist (MenuComponentValue const&, juce::File const& file);

	void onVLCOptionIntSelect(MenuComponentValue const&, std::string const&, int i);
    void onVLCOptionIntListMenu (MenuComponentValue const&, std::string const&);
    void onVLCOptionIntRangeMenu (MenuComponentValue const&, const char* );
	void onVLCOptionStringSelect(MenuComponentValue const&, std::string const&, std::string const& i);
    void onVLCOptionStringMenu (MenuComponentValue const&, std::string const&);
	void onVLCAudioChannelSelect(MenuComponentValue const&);
	void onVLCAudioOutputDeviceSelect(MenuComponentValue const&, std::string const& output, std::string const& device);
    void onVLCAudioOutputSelect(MenuComponentValue const&, std::string const&, std::vector< std::pair<std::string, std::string> >const&);
    void onVLCAudioOutputList(MenuComponentValue const&);
	void onMenuSearchOpenSubtitlesSelectLanguage(MenuComponentValue const&, juce::String const& name);
	void onMenuListOpenSubtitles(MenuComponentValue const&, juce::String const& url);
	void onMenuDowloadOpenSubtitle(MenuComponentValue const&, juce::String const& url);
	void onMenuSubtitlePositionAutomaticMode(MenuComponentValue const&);
	void onMenuSubtitlePositionCustomMode(MenuComponentValue const&);
	void onMenuSubtitlePositionMode(MenuComponentValue const&);
	void onMenuSubtitleSelect(MenuComponentValue const&, int i);
    void onMenuSubtitlePosition (MenuComponentValue const&);
	void onVLCOptionColor(MenuComponentValue const&, std::string);
    void onMenuSubtitleSelectMenu (MenuComponentValue const&);
    void onMenuSubtitleFontMenu (MenuComponentValue const&);
    void onMenuSubtitleMenu (MenuComponentValue const&);
	void onMenuZoomSlider (MenuComponentValue const& value);
    void onMenuCrop (MenuComponentValue const&, juce::String const& crop);
    void onMenuAutoCrop (MenuComponentValue const&);
    void onMenuCropList (MenuComponentValue const&);
    void onMenuRate (MenuComponentValue const&, double rate);
    void onMenuCustomRate (MenuComponentValue const&);
    void onMenuRateListAndSlider (MenuComponentValue const&);
    void onMenuSetAspectRatio(MenuComponentValue const&, juce::String const& ratio);
    void onMenuShiftAudio(double s);
    void onMenuShiftAudioSlider(MenuComponentValue const&);
    void onMenuShiftSubtitles(double s);
    void onMenuShiftSubtitlesSlider(MenuComponentValue const&);
	void onVLCAoutStringSelect(MenuComponentValue const&, std::string const&, std::string const&, std::string const&i);
    void onVLCAoutStringSelectListMenu (MenuComponentValue const&, std::string const&, std::string const&);

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
	void onOptionMenuExit(MenuComponentValue const&);
	void onMenuExitConfirmation(MenuComponentValue const&);
	void onMenuSoundOptions(MenuComponentValue const&);
	void onMenuRatio(MenuComponentValue const&);
	void onMenuVideoAdjustOptions(MenuComponentValue const&);
	void onMenuVideoOptions(MenuComponentValue const&);
	void onPlaylistItem(MenuComponentValue const&, int index);
	void onShowPlaylist(MenuComponentValue const&);
	void onLanguageOptions(MenuComponentValue const&);
	void onLanguageSelect(MenuComponentValue const&, std::string const& lang);
	void onSetPlayerFonSize(MenuComponentValue const&, int size);
	void onPlayerFonSize(MenuComponentValue const&);
	void onSetVLCOptionInt(MenuComponentValue const&, std::string const& name, int enable);
	void onSetVLCOption(MenuComponentValue const&, std::string const& name, bool enable);
	void onPlayerOptions(MenuComponentValue const&);
	void onOptionMenuRoot(MenuComponentValue const&);


private:

	static AbstractMenuItem::Icon getIcon(juce::String const& e);
	static AbstractMenuItem::Icon getIcon(juce::File const& f);


	static void listRecentPath(AbstractMenu& menu, MenuComponentValue const&, FileMethod const& fileMethod, juce::File const& path);
	static void listFiles(AbstractMenu& menu, MenuComponentValue const&, juce::File const& file, FileMethod const& fileMethod, FileMethod const& folderMethod);
	static void listShortcuts(AbstractMenu& menu, MenuComponentValue const&, FileMethod const& fileMethod, juce::StringArray const& shortcuts);
	static void listRootFiles(AbstractMenu& menu, MenuComponentValue const&, FileMethod const& fileMethod);

	void initBoolSetting(const char* name);
	void initIntSetting(const char* name);
	void initIntSetting(const char* name, int defaultVal);
	void initStrSetting(const char* name);

    bool downloadedSubtitleSeekerResult(MenuComponentValue const&, juce::String const& resultSite,
                                                    char* cstr,
                                                     juce::String const& siteTarget,
                                                     std::string const& match,
                                                     std::string const& downloadURLPattern );

};

#endif //PLAYER_MENUS_H
