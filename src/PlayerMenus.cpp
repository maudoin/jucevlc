
#include "PlayerMenus.h"
#include "Languages.h"
#include "FileSorter.h"
#include "Extensions.h"
#include "FileSorter.h"
#include <algorithm>
#include <set>
#include <format>
#include <functional>
#include <regex>
#include <variant>


#define MAX_SUBTITLE_ARCHIVE_SIZE 1024*1024
#define SUBTITLE_DOWNLOAD_TIMEOUT_MS 30000

#define SETTINGS_FULLSCREEN "SETTINGS_FULLSCREEN"
#define SETTINGS_VOLUME "SETTINGS_VOLUME"
#define SETTINGS_CROP "SETTINGS_CROP"
#define SETTINGS_FONT_SIZE "SETTINGS_FONT_SIZE"
#define SETTINGS_LAST_OPEN_PATH "SETTINGS_LAST_OPEN_PATH"
#define SETTINGS_LANG "SETTINGS_LANG"
#define SETTINGS_AUTO_SUBTITLES_HEIGHT "SETTINGS_AUTO_SUBTITLES_HEIGHT"
#define SETTINGS_AUDIO_DEVICE "SETTINGS_AUDIO_DEVICE"
#define SETTINGS_AUDIO_OUTPUT "SETTINGS_AUDIO_OUTPUT"
#define SHORTCUTS_FILE "shortcuts.list"
#define MAX_MEDIA_TIME_IN_SETTINGS 30



using namespace std::placeholders;

namespace
{
juce::PropertiesFile::Options options()
{
	juce::PropertiesFile::Options opts;
	opts.applicationName = "JucyVLC";
	opts.folderName = "JucyVLC";
	opts.commonToAllUsers = false;
	opts.filenameSuffix = "xml";
	opts.ignoreCaseOfKeyNames = true;
	opts.millisecondsBeforeSaving = 1000;
	opts.storageFormat = juce::PropertiesFile::storeAsXML;
	return opts;
}


class BackgoundUPNP : public juce::Thread
{
	juce::CriticalSection mutex;
	std::unique_ptr<VLCUPNPMediaList> vlcMediaUPNPList;
public:
	BackgoundUPNP():juce::Thread("BackgoundUPNP")
	{
		startThread();
	}
	~BackgoundUPNP()
	{
		stopThread(2000);
	}
	void run()
	{
		VLCUPNPMediaList* built = new VLCUPNPMediaList();
		juce::CriticalSection::ScopedLockType l(mutex);
		vlcMediaUPNPList.reset(built);
	}

	std::vector<std::pair<std::string, std::string> > getUPNPList(std::vector<std::string> const& path)
	{

		juce::CriticalSection::ScopedLockType l(mutex);
		if(vlcMediaUPNPList)
		{
			return vlcMediaUPNPList->getUPNPList(path);
		}
		return std::vector<std::pair<std::string, std::string> >();
	}
};

}
////////////////////////////////////////////////////////////
//
// MAIN COMPONENT
//
////////////////////////////////////////////////////////////
PlayerMenus::PlayerMenus(std::unique_ptr<VLCWrapper> const& vlc, ViewHandler* viewHandler,
	std::unique_ptr<AbstractMenu>& fileMenu, std::unique_ptr<AbstractMenu>& optionsMenu)
	: vlc(vlc.get())
	, m_viewHandler(*viewHandler)
	, m_fileMenu(fileMenu.get())
	, m_optionsMenu(optionsMenu.get())
	, m_settings(juce::File::getCurrentWorkingDirectory().getChildFile("settings.xml"), options())
	, m_mediaTimes(juce::File::getCurrentWorkingDirectory().getChildFile("mediaTimes.xml"), options())
	, m_autoSubtitlesHeight(true)
{
	Languages::getInstance();
}

PlayerMenus::~PlayerMenus()
{
	Languages::getInstance().clear();

	saveCurrentMediaTime();

}

bool PlayerMenus::isAutoSubtitlesHeight()const
{
	return m_autoSubtitlesHeight;
}

double PlayerMenus::getSavedVolume()const
{
	return m_settings.getDoubleValue(SETTINGS_VOLUME, 100.);
}

void PlayerMenus::saveFullscreenState(bool fs)
{

	m_settings.setValue(SETTINGS_FULLSCREEN, fs);
}

AbstractMenuItem::Icon PlayerMenus::getIcon(juce::String const& e)
{
	if(extensionMatch(Extensions::get().videoExtensions(), e))
	{
		return AbstractMenuItem::Icon::Display;
	}
	if(extensionMatch(Extensions::get().playlistExtensions(), e))
	{
		return AbstractMenuItem::Icon::Folder;
	}
	if(extensionMatch(Extensions::get().subtitlesExtensions(), e))
	{
		return AbstractMenuItem::Icon::Subtitles;
	}
	return AbstractMenuItem::Icon::None;
}
AbstractMenuItem::Icon PlayerMenus::getIcon(juce::File const& f)
{
	if(f.isDirectory())
	{
		return AbstractMenuItem::Icon::Folder;
	}
	return getIcon(f.getFileExtension());
}


////////////////////////////////////////////////////////////
//
// MENU TREE CALLBACKS
//
////////////////////////////////////////////////////////////

namespace{

juce::String name(juce::File const& file)
{
	juce::File p = file.getParentDirectory();
	return p.getFullPathName() == file.getFullPathName() ?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileName();
}

}


void PlayerMenus::onFileMenuRoot(MenuComponentValue const& value, FileMethod const& fileMethod)
{
	juce::String path = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
	juce::File f(path);

	onMenuListFavorites(value, fileMethod);
	m_fileMenu->addMenuItem( TRANS("Settings"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onFileMenuSettings, this, _1), AbstractMenuItem::Icon::Settings);
	m_fileMenu->addMenuItem( TRANS("Exit"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuExit, this, _1), AbstractMenuItem::Icon::Exit);
}
void PlayerMenus::onFileMenuSettings(MenuComponentValue const&)
{
	m_fileMenu->addMenuItem( TRANS("FullScreen"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuFullscreen, this, _1, true), m_viewHandler.isFullScreen()?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_fileMenu->addMenuItem( TRANS("Windowed"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuFullscreen, this, _1, false), m_viewHandler.isFullScreen()?AbstractMenuItem::Icon::None : AbstractMenuItem::Icon::Check);

	m_fileMenu->addMenuItem( TRANS("Language"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onFileMenuLanguageOptions, this, _1), AbstractMenuItem::Icon::Folder);
	m_fileMenu->addMenuItem( TRANS("Menu font size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onFileMenuPlayerFonSize, this, _1));
}
void PlayerMenus::onFileMenuLanguageOptions(MenuComponentValue const&)
{
	std::vector< std::string > list = Languages::getInstance().getLanguages();
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{
		m_fileMenu->addMenuItem(it->c_str(), AbstractMenuItem::REFRESH_MENU,
			std::bind(&PlayerMenus::onLanguageSelect, this, _1, *it),
			(*it==Languages::getInstance().getCurrentLanguage())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}
}
void PlayerMenus::onFileMenuPlayerFonSize(MenuComponentValue const&)
{
	for(int i=50;i<=175;i+=25)
	{
		m_fileMenu->addMenuItem( juce::String::formatted("%d%%", i), AbstractMenuItem::REFRESH_MENU,
			std::bind(&PlayerMenus::onSetPlayerFonSize, this, _1, i),
			i==(AppProportionnalComponent::getItemHeightPercentageRelativeToScreen())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}
}

void PlayerMenus::onMenuLoadSubtitle(MenuComponentValue const& value, FileMethod const& fileMethod)
{
	if(std::get_if<MenuComponentBack>(&value))
	{
		listRootFiles(*m_optionsMenu, value, fileMethod);
	}
	else
	{
		juce::String path = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
		juce::File f(path);
		listRecentPath(*m_optionsMenu, value, fileMethod, f);
	}
}

void PlayerMenus::onMenuListRootFiles(MenuComponentValue const& value, FileMethod const& fileMethod)
{
	listRootFiles(*m_fileMenu, value, fileMethod);
	m_fileMenu->addMenuItem( TRANS("UPNP videos..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuListUPNPFiles, this, _1, std::vector<std::string>()), AbstractMenuItem::Icon::None);
}

//include the dot
std::string getPathExtensionWithoutDot(std::string const& path)
{
	std::string::size_type p = path.find_last_of(".");
	if(p == std::string::npos)
	{
		return "";
	}
	return path.substr(p+1);
}

//include the dot
juce::String getPathExtensionWithoutDot(juce::String const& path)
{
	int p = path.lastIndexOf(".");
	if(p == -1)
	{
		return {};
	}
	return path.substring(p+1);
}

void PlayerMenus::onMenuListUPNPFiles(MenuComponentValue const&, std::vector<std::string> const& path)
{
	std::vector<std::pair<std::string, std::string> > list = vlcMediaUPNPList->getUPNPList(path);
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		if(std::string::npos == std::string(it->second).find("vlc://nop"))
		{
			m_fileMenu->addMenuItem( it->first.c_str(), AbstractMenuItem::EXECUTE_ONLY, std::bind(&PlayerMenus::onMenuOpenUnconditionnal, this, _1, juce::String::fromUTF8(it->second.c_str())), getIcon(getPathExtensionWithoutDot(it->first).c_str()));
		}
		else
		{
			std::vector<std::string> newPath(path);
			newPath.push_back(it->first);
			m_fileMenu->addMenuItem( it->first.c_str(), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuListUPNPFiles, this, _1, newPath), AbstractMenuItem::Icon::Folder);
		}
	}


}
void PlayerMenus::onMenuListFavorites(MenuComponentValue const& value, FileMethod const& fileMethod)
{

	mayPurgeFavorites();

	m_fileMenu->addMenuItem( TRANS("All videos..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuListRootFiles, this, _1, fileMethod), AbstractMenuItem::Icon::Folder);

	listShortcuts(*m_fileMenu, value, fileMethod, m_shortcuts);
}

void PlayerMenus::mayPurgeFavorites()
{
	bool changed = false;
	juce::StringArray newShortcuts;
	for(int i=0;i<m_shortcuts.size();++i)
	{
		juce::File path(m_shortcuts[i]);
		if(path.getVolumeSerialNumber() == 0 || path.exists() )
		{
			//still exists or unkown
			newShortcuts.add(path.getFullPathName());
		}
		else
		{
			changed = true;
		}
	}

	if(changed)
	{
		m_shortcuts = newShortcuts;
		writeFavorites();
	}
}

void PlayerMenus::writeFavorites()
{
	juce::File shortcuts(juce::File::getCurrentWorkingDirectory().getChildFile(SHORTCUTS_FILE));
	m_shortcuts.removeDuplicates(true);
	m_shortcuts.removeEmptyStrings();
	shortcuts.replaceWithText(m_shortcuts.joinIntoString("\n"));
}

void PlayerMenus::onMenuAddFavorite(MenuComponentValue const&, juce::String const& path)
{
	m_shortcuts.add(path);
	writeFavorites();
}
void PlayerMenus::onMenuRemoveFavorite(MenuComponentValue const&, juce::String const& path)
{
	m_shortcuts.removeString(path);
	writeFavorites();
}

void PlayerMenus::onMenuOpenUnconditionnal (MenuComponentValue const&, juce::String const& path)
{
	m_viewHandler.closeFileMenu();
	appendAndPlay(path.toUTF8().getAddress());
}
void PlayerMenus::onMenuQueue (MenuComponentValue const&, juce::String const& path)
{
	m_viewHandler.closeFileMenu();
	vlc->addPlayListItem(path.toUTF8().getAddress());
}

void PlayerMenus::onMenuOpenFolder (MenuComponentValue const& value, juce::File const& file)
{
	if(file.isDirectory())
	{
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		m_fileMenu->addMenuItem(TRANS("Play All"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&PlayerMenus::onMenuOpenUnconditionnal, this, _1,
				file.getFullPathName()), AbstractMenuItem::Icon::PlayAll);
		m_fileMenu->addMenuItem(TRANS("Add All"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&PlayerMenus::onMenuQueue, this, _1,
				file.getFullPathName()), AbstractMenuItem::Icon::AddAll);

		listFiles(*m_fileMenu, value, file, std::bind(&PlayerMenus::onMenuOpenFile, this, _1, _2),
								std::bind(&PlayerMenus::onMenuOpenFolder, this, _1, _2));

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			m_fileMenu->addMenuItem(TRANS("Add to favorites"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuAddFavorite, this, _1,
				file.getFullPathName()), AbstractMenuItem::Icon::FolderShortcutOutline);
		}
		else
		{
			m_fileMenu->addMenuItem(TRANS("Remove from favorites"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuRemoveFavorite, this, _1,
				file.getFullPathName()), AbstractMenuItem::Icon::FolderShortcut);
		}
	}
}

void PlayerMenus::onMenuOpenFile (MenuComponentValue const& value, juce::File const& file)
{
	if(!file.isDirectory())
	{
		if(extensionMatch(Extensions::get().subtitlesExtensions(), file))
		{
			m_viewHandler.closeFileMenu();
			vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
		}
		else
		{
			onMenuOpenUnconditionnal(value, file.getFullPathName());
		}
	}
}
void PlayerMenus::onVLCOptionIntSelect(MenuComponentValue const&, std::string const& name, int v)
{
	vlc->setConfigOptionInt(name.c_str(), v);
	m_settings.setValue(name.c_str(), (int)v);
}
void PlayerMenus::onVLCOptionIntListMenu(MenuComponentValue const&, std::string const& name)
{

	std::pair<int, std::vector<std::pair<int, std::string> > > res = vlc->getConfigOptionInfoInt(name.c_str());
	for(std::vector<std::pair<int, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		int const item = it->first;
		m_optionsMenu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, [this, name, item](MenuComponentValue const&v){this->onVLCOptionIntSelect(v, name, item);}, item==vlc->getConfigOptionInt(name.c_str())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}

}

void PlayerMenus::onVLCOptionStringSelect(MenuComponentValue const&, std::string const& name, std::string const& v)
{
	vlc->setConfigOptionString(name.c_str(), v);
	m_settings.setValue(name.c_str(), juce::String(v.c_str()));
}
void PlayerMenus::onVLCOptionStringMenu (MenuComponentValue const&, std::string const& name)
{

	std::pair<std::string, std::vector<std::pair<std::string, std::string> > > res = vlc->getConfigOptionInfoString(name.c_str());
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		m_optionsMenu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onVLCOptionStringSelect, this, _1, name, it->first), it->first==vlc->getConfigOptionString(name.c_str())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}

}
void setVoutOptionInt(VLCWrapper * vlc, std::string option, double value)
{
	vlc->setVoutOptionInt(option.c_str(), (int)value);

}

void PlayerMenus::onMenuSubtitlePositionAutomaticMode(MenuComponentValue const&)
{

	m_autoSubtitlesHeight = true;
	m_settings.setValue(SETTINGS_AUTO_SUBTITLES_HEIGHT, true);

	m_viewHandler.handleControlRelayout();

}

void PlayerMenus::onMenuSubtitlePositionCustomMode(MenuComponentValue const& value)
{

	if (m_autoSubtitlesHeight)
	{
		m_autoSubtitlesHeight = false;
		m_settings.setValue(SETTINGS_AUTO_SUBTITLES_HEIGHT, m_autoSubtitlesHeight);
	}

	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setVoutOptionInt(CONFIG_INT_OPTION_SUBTITLE_MARGIN, (int)*doublePtr);
	}

}
void PlayerMenus::onMenuSubtitlePosition(MenuComponentValue const&)
{
	m_optionsMenu->addMenuItem( TRANS("Automatic"), AbstractMenuItem::REFRESH_MENU,
		[this](MenuComponentValue const& value){this->onMenuSubtitlePositionAutomaticMode(value);},
		m_autoSubtitlesHeight?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	static constexpr int MAX_SUBTITLE_OFFSET = 4096;
	m_optionsMenu->addMenuItem( TRANS("Custom"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		[this](MenuComponentValue const& value){this->onMenuSubtitlePositionCustomMode(value);},
		(!m_autoSubtitlesHeight)?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None,
		SettingSlider::Params{"", (double)vlc->getVoutOptionInt(CONFIG_INT_OPTION_SUBTITLE_MARGIN), 0., 0., MAX_SUBTITLE_OFFSET});

}
inline juce::Colour RGB2ARGB(int rgb)
{
    uint8 r = (uint8) (rgb >> 16);
    uint8 g = (uint8) (rgb >> 8);
    uint8 b = (uint8) (rgb);
	return juce::Colour::fromRGB(r, g, b);
}

inline int ARGB2RGB(juce::Colour const& c)
{
	juce::PixelARGB argb = c.getPixelARGB();
	return ((argb.getRed() << 16) | (argb.getGreen() << 8) | argb.getBlue())&0xFFFFFF;
}

struct PopupColourSelector final : public Component,
									private ChangeListener
{
	using Listener = std::function<void(juce::Colour const&)>;
	PopupColourSelector (const juce::Colour& colour, int width, int height,
							Listener const& listener)
		: selector(juce::ColourSelector::showColourspace)
		, m_listener (listener)
	{
		selector.setCurrentColour(colour);
		addAndMakeVisible (selector);
		selector.addChangeListener (this);
		setSize (width, height);
	}

	void resized() override
	{
		selector.setBounds (getLocalBounds());
	}

private:
	void changeListenerCallback (ChangeBroadcaster*) override
	{
		m_listener (selector.getCurrentColour());
		if(juce::CallOutBox* parent =dynamic_cast<juce::CallOutBox*>(getParentComponent()))
		{
			parent->dismiss();
		}
	}


	juce::ColourSelector selector;
	Listener m_listener;
};

void PlayerMenus:: onVLCOptionColor(MenuComponentValue const& value, std::string attr)
{
	if ( juce::Colour const* colour = std::get_if<juce::Colour>(&value) )
	{
		int newCol = ARGB2RGB(*colour);
		vlc->setConfigOptionInt(attr.c_str(), newCol);
		m_settings.setValue(attr.c_str(), newCol);
	}
}


void PlayerMenus::onVLCOptionIntRangeMenu(MenuComponentValue const& value, const char* attr)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		this->vlc->setConfigOptionInt(attr, (int)*doublePtr);
		this->m_settings.setValue(attr, (int)*doublePtr);
	}
}

void PlayerMenus::onMenuSubtitleSelectMenu(MenuComponentValue const&)
{
	std::vector<std::pair<int, std::string> > subs = vlc->getSubtitles();
	int current = vlc->getCurrentSubtitleIndex();
	if(!subs.empty())
	{
		for(std::vector<std::pair<int, std::string> >::const_iterator i = subs.begin();i != subs.end();++i)
		{
			m_optionsMenu->addMenuItem( juce::String("> ") + i->second.c_str(),
                     AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSubtitleSelect, this, _1, i->first), i->first==current?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
		}
	}
	else
	{
		m_optionsMenu->addMenuItem( juce::String::formatted(TRANS("No subtitles")), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSubtitleSelect, this, _1, -1), 0==current?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}
	m_optionsMenu->addMenuItem( TRANS("Add..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		std::bind(&PlayerMenus::onMenuLoadSubtitle, this, _1,
				  [this](auto& item, auto const& file){this->onMenuOpenSubtitleFolder(item, file);}), AbstractMenuItem::Icon::Folder);
	m_optionsMenu->addMenuItem( TRANS("opensubtitles.org"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		[this](MenuComponentValue const& value){this->onMenuSearchOpenSubtitlesSelectLanguage(value, vlc->getCurrentPlayListItem().c_str());},
		AbstractMenuItem::Icon::Download);

}

void PlayerMenus::onMenuSubtitleFontMenu(MenuComponentValue const&)
{

	auto addColorItem = [&](juce::String const& label, const char* vlcKey)
	{
		m_optionsMenu->addMenuItem( label,
			AbstractMenuItem::STORE_AND_OPEN_COLOR,
			std::bind(&PlayerMenus::onVLCOptionColor, this, _1, std::string(vlcKey)),
			AbstractMenuItem::Icon::None, RGB2ARGB(vlc->getConfigOptionInt(vlcKey)));
	};
	auto addSliderItem = [&](juce::String const& label, const char* vlcKey, double defaultV, double min, double max)
	{
		m_optionsMenu->addMenuItem( label,
			AbstractMenuItem::STORE_AND_OPEN_SLIDER,
			std::bind(&PlayerMenus::onVLCOptionIntRangeMenu, this, _1, vlcKey),
			AbstractMenuItem::Icon::None, SettingSlider::Params{"%.f", (double)vlc->getConfigOptionInt(vlcKey), defaultV, min, max});
	};

	m_optionsMenu->addMenuItem( TRANS("Size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_SIZE)));
	addSliderItem( TRANS("Opacity"), CONFIG_INT_OPTION_SUBTITLE_OPACITY, 255, 0, 255);
	addColorItem( TRANS("Color"), CONFIG_COLOR_OPTION_SUBTITLE_COLOR);
	m_optionsMenu->addMenuItem( TRANS("Outline"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_THICKNESS)));
	addSliderItem( TRANS("Outline opacity"), CONFIG_INT_OPTION_SUBTITLE_OUTLINE_OPACITY, 255, 0, 255);
	addColorItem( TRANS("Outline Color"), CONFIG_COLOR_OPTION_SUBTITLE_OUTLINE_COLOR);
	addSliderItem( TRANS("Background opacity"), CONFIG_INT_OPTION_SUBTITLE_BACKGROUND_OPACITY, 0, 255, 0);
	addColorItem( TRANS("Background Color"), CONFIG_COLOR_OPTION_SUBTITLE_BACKGROUND_COLOR);
	addSliderItem( TRANS("Shadow opacity"), CONFIG_INT_OPTION_SUBTITLE_SHADOW_OPACITY,0, 255, 0);
	addColorItem( TRANS("Shadow Color"), CONFIG_COLOR_OPTION_SUBTITLE_SHADOW_COLOR);

}

void PlayerMenus::onMenuSubtitleMenu(MenuComponentValue const&)
{
	m_optionsMenu->addMenuItem( TRANS("Select"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuSubtitleSelectMenu, this, _1), AbstractMenuItem::Icon::Folder);

	m_optionsMenu->addMenuItem( TRANS("Delay"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuShiftSubtitlesSlider, this, _1), AbstractMenuItem::Icon::None,
		SettingSlider::Params{"%+.3fs", vlc->getSubtitleDelay()/1000000., 0., -2., 2., .01, 2.});

	m_optionsMenu->addMenuItem( TRANS("Font"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuSubtitleFontMenu, this, _1));

	m_optionsMenu->addMenuItem( TRANS("Position"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuSubtitlePosition, this, _1));


}

template <typename Op>
void applyOnAllSubtitleLanguages(Op const& add)
{
	add("all","en","All");
	add("eng","en","English");
	add("alb","sq","Albanian");
	add("ara","ar","Arabic");
	add("arm","hy","Armenian");
	add("baq","eu","Basque");
	add("ben","bn","Bengali");
	add("bos","bs","Bosnian");
	add("pob","pb","Portuguese-BR");
	add("bre","br","Breton");
	add("bul","bg","Bulgarian");
	add("bur","my","Burmese");
	add("cat","ca","Catalan");
	add("chi","zh","Chinese");
	add("hrv","hr","Croatian");
	add("cze","cs","Czech");
	add("dan","da","Danish");
	add("dut","nl","Dutch");
	add("eng","en","English");
	add("epo","eo","Esperanto");
	add("est","et","Estonian");
	add("fin","fi","Finnish");
	add("fre","fr","French");
	add("glg","gl","Galician");
	add("geo","ka","Georgian");
	add("ger","de","German");
	add("ell","el","Greek");
	add("heb","he","Hebrew");
	add("hin","hi","Hindi");
	add("hun","hu","Hungarian");
	add("ice","is","Icelandic");
	add("ind","id","Indonesian");
	add("ita","it","Italian");
	add("jpn","ja","Japanese");
	add("kaz","kk","Kazakh");
	add("khm","km","Khmer");
	add("kor","ko","Korean");
	add("lav","lv","Latvian");
	add("lit","lt","Lithuanian");
	add("ltz","lb","Luxembourgish");
	add("mac","mk","Macedonian");
	add("may","ms","Malay");
	add("mal","ml","Malayalam");
	add("mon","mn","Mongolian");
	add("nor","no","Norwegian");
	add("oci","oc","Occitan");
	add("per","fa","Farsi");
	add("pol","pl","Polish");
	add("por","pt","Portuguese");
	add("rum","ro","Romanian");
	add("rus","ru","Russian");
	add("scc","sr","Serbian");
	add("sin","si","Sinhalese");
	add("slo","sk","Slovak");
	add("slv","sl","Slovenian");
	add("spa","es","Spanish");
	add("swa","sw","Swahili");
	add("swe","sv","Swedish");
	add("syr","","Syriac");
	add("tgl","tl","Tagalog");
	add("tam","ta","Tamil");
	add("tel","te","Telugu");
	add("tha","th","Thai");
	add("tur","tr","Turkish");
	add("ukr","uk","Ukrainian");
	add("urd","ur","Urdu");
	add("vie","vi","Vietnamese");
}

void PlayerMenus::onMenuSearchOpenSubtitlesSelectLanguage(MenuComponentValue const&, juce::String const& movieNameQuery)
{
	juce::String movieName = movieNameQuery.replace("%", "%37");
	movieName = movieName.replace(" ", "-");
	movieName = movieName.replace("_", "-");
	movieName = movieName.replace(".", "-");
	applyOnAllSubtitleLanguages([&](const char* lang, const char* /*ui*/, const char* label)
	{
		m_optionsMenu->addMenuItem( label, AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		  [this, lang, movieName](MenuComponentValue const& value)
		  {
			this->onMenuListOpenSubtitles(value,
				juce::String(std::format("/en/search2/sublanguageid-{}/moviename-{}/xml",
					lang,std::string(movieName.toUTF8().getAddress()) ).c_str()));
		});
	});
}

void PlayerMenus::onMenuListOpenSubtitles(MenuComponentValue const&, juce::String const& address)
{
	static const juce::String baseUrl = "http://www.opensubtitles.org";
	auto mayCompleteUrl = [](juce::String& url)
	{
		if(url.startsWith("/") || !url.startsWith("http"))
		{
			url = baseUrl + url;
		}
		return url;
	};
	juce::String urlStr = address;
	mayCompleteUrl(urlStr);
	juce::URL url = urlStr;
	if(std::unique_ptr<juce::InputStream> pIStream =
		url.createInputStream(juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).
			withConnectionTimeoutMs (SUBTITLE_DOWNLOAD_TIMEOUT_MS)))
	{
		if(std::unique_ptr<juce::XmlElement> root = juce::XmlDocument::parse(pIStream->readEntireStreamAsString()))
		{
			if(juce::XmlElement* search = root->getChildByName("search"))
			{
				if(juce::XmlElement* results = search->getChildByName("results"))
				{
					if(juce::XmlElement* sub = results->getChildByName("subtitle"))
					{
						do
						{
							if(juce::XmlElement* id = sub->getChildByName("MovieID"))
							{
								juce::String downloadURL = id->getStringAttribute("Link");
								juce::String name = sub->getChildElementAllSubText("MovieName", {});
								if(!name.isEmpty() && !downloadURL.isEmpty())
								{
									juce::String count = sub->getChildElementAllSubText("TotalSubs", {});
									if(!count.isEmpty())
									{
										name += juce::String(" (") + count + juce::String(")");
									}
									m_optionsMenu->addMenuItem(name, AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
										std::bind(&PlayerMenus::onMenuListOpenSubtitles, this, _1, downloadURL));
								}
							}
							if(juce::XmlElement* id = sub->getChildByName("SeriesDownloadsCnt"))
							{
								juce::String downloadURL = id->getStringAttribute("LinkDownload");
								juce::String name = sub->getChildElementAllSubText("MovieName", {});
								if(!name.isEmpty() && !downloadURL.isEmpty())
								{
									juce::String s = sub->getChildElementAllSubText("SeriesSeason", {});
									if(!s.isEmpty())
									{
										name += juce::String(" S") + s;
									}
									juce::String e = sub->getChildElementAllSubText("SeriesEpisode", {});
									if(!e.isEmpty())
									{
										name += juce::String(" E") + e;
									}
									m_optionsMenu->addMenuItem(name, AbstractMenuItem::EXECUTE_ONLY,
										std::bind(&PlayerMenus::onMenuDowloadOpenSubtitle, this, _1,  mayCompleteUrl(downloadURL)));
								}
							}
							if(juce::XmlElement* id = sub->getChildByName("IDSubtitle"))
							{
								juce::String downloadURL = id->getStringAttribute("LinkDownload");
								juce::String name = sub->getChildElementAllSubText("MovieName", {});
								if(!name.isEmpty() && !downloadURL.isEmpty())
								{
									m_optionsMenu->addMenuItem(name, AbstractMenuItem::EXECUTE_ONLY,
										std::bind(&PlayerMenus::onMenuDowloadOpenSubtitle, this, _1,  mayCompleteUrl(downloadURL)));
								}
							}

							sub = sub->getNextElement();
						}
						while(sub);
					}
				}
			}
		}
		//m_optionsMenu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuSearchSubtitlesManually, this, _1, lang), AbstractMenuItem::Icon::Item);
		m_optionsMenu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU,
			[this, urlStr](MenuComponentValue const& value){this->onMenuListOpenSubtitles(value, urlStr);});

	}
	else
	{
		m_optionsMenu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU,
			[this, urlStr](MenuComponentValue const& value){this->onMenuListOpenSubtitles(value, urlStr);});
	}


}

struct ZipEntrySorter
{
	std::vector< std::set<juce::String> > priorityExtensions;
	ZipEntrySorter(std::set<juce::String> const& priorityExtensions_){priorityExtensions.push_back(priorityExtensions_);}
	ZipEntrySorter(std::vector< std::set<juce::String> > const& priorityExtensions_):priorityExtensions(priorityExtensions_) {}
	int rank(const juce::ZipFile::ZipEntry* f)
	{
		for(std::vector< std::set<juce::String> >::const_iterator it = priorityExtensions.begin();it != priorityExtensions.end();++it)
		{
			if(extensionMatch(*it, getPathExtensionWithoutDot(f->filename)))
			{
				return (it-priorityExtensions.begin());
			}
		}
		return priorityExtensions.size();
	}
	int compareElements(const juce::ZipFile::ZipEntry* some, const juce::ZipFile::ZipEntry* other)
	{
		int r1 = rank(some);
		int r2 = rank(other);
		if(r1 == r2)
		{
			return some->uncompressedSize - other->uncompressedSize;
		}
		return r1 - r2;
	}
};

void PlayerMenus::onMenuDowloadOpenSubtitle(MenuComponentValue const& value, juce::String const& downloadUrl)
{
	juce::URL url(downloadUrl);


	juce::StringPairArray response;
	std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(
			juce::URL::InputStreamOptions(
					juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs (SUBTITLE_DOWNLOAD_TIMEOUT_MS).withResponseHeaders(&response)));
	juce::StringArray const& headers = response.getAllKeys();
	juce::String fileName = downloadUrl.fromLastOccurrenceOf("/", false, false);
	for(int i=0;i<headers.size();++i)
	{
		juce::String const& h = headers[i];
		juce::String const& v = response[h];
		if(h.equalsIgnoreCase("Content-Disposition") && v.startsWith("attachment"))
		{
			fileName = v.fromLastOccurrenceOf("=", false, false);
			fileName = fileName.trimCharactersAtStart("\" ");
			fileName = fileName.trimCharactersAtEnd("\" ");
		}
	}
	if(pIStream.get())
	{
		juce::MemoryOutputStream memStream(10000);//10ko at least
		int written = memStream.writeFromInputStream(*pIStream, MAX_SUBTITLE_ARCHIVE_SIZE);
		if(written>0)
		{
			juce::String outPath = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);


			juce::MemoryInputStream memInput(memStream.getData(), written, false);

			juce::ZipFile zip(memInput);
			if(zip.getNumEntries()==0)
			{
				juce::File out(outPath);
				out = out.getChildFile(fileName);
				juce::FileOutputStream outStream(out);
				if(outStream.openedOk())
				{
					memInput.setPosition(0);
					if(outStream.writeFromInputStream(memInput, written)>0)
					{
						onMenuOpenSubtitleFile(value,out);
					}
				}
				return;
			}
			juce::Array<const juce::ZipFile::ZipEntry*> entries;
			for(int i=0;i<zip.getNumEntries();++i)
			{
				entries.add(zip.getEntry(i));
			}

			std::set<juce::String> subExtensions;
			EXTENSIONS_SUBTITLE([&subExtensions](const char* item){subExtensions.insert(item);});
			subExtensions.erase("txt");

			//txt is ambiguous ,lesser priority
			std::set<juce::String> txtExtension;
			txtExtension.insert("txt");

			std::vector< std::set<juce::String> > priorityExtensions;
			priorityExtensions.push_back(subExtensions);
			priorityExtensions.push_back(txtExtension);

			//find the subtitles based on extension/size
			ZipEntrySorter sorter(priorityExtensions);
			entries.sort(sorter);



			juce::Array<const juce::ZipFile::ZipEntry*> entriesToExtract;

			const juce::ZipFile::ZipEntry* supposedSubtitle = entries.getFirst();
			if(extensionMatch(subExtensions,juce::File(supposedSubtitle->filename)) || extensionMatch(txtExtension,juce::File(supposedSubtitle->filename)) )
            {
                entriesToExtract.add(supposedSubtitle);
            }
            else
            {
                //extract all
                for(int i=0;i<zip.getNumEntries();++i)
                {
                    entriesToExtract.add(zip.getEntry(i));
                }
            }

            juce::File out(outPath);
            for(int i=0;i<entriesToExtract.size();++i)
            {
                zip.uncompressEntry(zip.getIndexOfFileName(entriesToExtract[i]->filename), out, false);
            }

            if(entriesToExtract.size()==1)
            {
                onMenuOpenSubtitleFile(value,out.getChildFile(entriesToExtract.getFirst()->filename));

            }
		}
	}
}
void PlayerMenus::onMenuSubtitleSelect(MenuComponentValue const&, int i)
{
	vlc->setSubtitleIndex(i);
}
void PlayerMenus::onMenuOpenSubtitleFolder (MenuComponentValue const&, juce::File const& file)
{
	if(file.isDirectory())
	{
		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false, "*");
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			m_optionsMenu->addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuOpenSubtitleFolder, this, _1, file), AbstractMenuItem::Icon::Folder);
		}


		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		FileSorter sorter(Extensions::get().subtitlesExtensions());
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			m_optionsMenu->addMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY, std::bind(&PlayerMenus::onMenuOpenSubtitleFile, this, _1, file),
										extensionMatch(Extensions::get().subtitlesExtensions(), destArray[i])?AbstractMenuItem::Icon::Subtitles:AbstractMenuItem::Icon::None);
		}
	}
}
void PlayerMenus::onMenuOpenSubtitleFile (MenuComponentValue const&, juce::File const& file)
{
	if(!file.isDirectory())
	{
		vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
	}
}
void PlayerMenus::onMenuOpenPlaylist (MenuComponentValue const&, juce::File const&)
{
}

void PlayerMenus::onMenuCrop (MenuComponentValue const&, juce::String const& ratio)
{
	vlc->setAutoCrop(false);
	vlc->setCrop(std::string(ratio.getCharPointer().getAddress()));

	m_settings.setValue(SETTINGS_CROP, ratio);

}
void PlayerMenus::onMenuAutoCrop (MenuComponentValue const&)
{
	vlc->setAutoCrop(true);
}
void PlayerMenus::onMenuCropList (MenuComponentValue const&)
{
	//m_optionsMenu->addMenuItem( TRANS("Auto"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuAutoCrop, this, _1), vlc->isAutoCrop()?AbstractMenuItem::Icon::Item : AbstractMenuItem::Icon::None);
	std::string current = vlc->getCrop();
	std::vector<std::string> list = vlc->getCropList();
	for(std::vector<std::string>::const_iterator it = list.begin();it != list.end();++it)
	{
		juce::String ratio(it->c_str());
		m_optionsMenu->addMenuItem( ratio.isEmpty()?TRANS("Original"):ratio, AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuCrop, this, _1, ratio), *it==current?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}

}
void PlayerMenus::onMenuRate (MenuComponentValue const&, double rate)
{
	vlc->setRate(rate);

}
void PlayerMenus::onMenuCustomRate (MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setRate(*doublePtr);
	}
}
void PlayerMenus::onMenuRateListAndSlider (MenuComponentValue const&)
{
	m_optionsMenu->addMenuItem( "25%", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuRate, this, _1, 25.), 25==(int)(vlc->getRate())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "50%", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuRate, this, _1, 50.), 50==(int)(vlc->getRate())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "100%", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuRate, this, _1, 100.), 100==(int)(vlc->getRate())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "125%", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuRate, this, _1, 125.), 125==(int)(vlc->getRate())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "150%", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuRate, this, _1, 150.), 150==(int)(vlc->getRate())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "200%", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuRate, this, _1, 200.), 200==(int)(vlc->getRate())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "Custom", AbstractMenuItem::STORE_AND_OPEN_SLIDER, std::bind(&PlayerMenus::onMenuCustomRate, this, _1),
		AbstractMenuItem::Icon::None, SettingSlider::Params{"%.f%%", vlc->getRate(), 100., 25., 400., 25.});


}
void PlayerMenus::onMenuShiftAudio(double s)
{
	vlc->setAudioDelay((int64_t)(s*1000000.));
}
void PlayerMenus::onMenuShiftAudioSlider(MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setAudioDelay((int64_t)(*doublePtr*1000000.));
	}
}
void PlayerMenus::onMenuShiftSubtitles(double s)
{
	vlc->setSubtitleDelay((int64_t)(s*1000000.));
}
void PlayerMenus::onMenuShiftSubtitlesSlider(MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setSubtitleDelay((int64_t)(*doublePtr*1000000.));
	}
}

void PlayerMenus::onVLCAoutStringSelect(MenuComponentValue const&, std::string const& filter, std::string const& name, std::string const& v)
{
	vlc->setAoutFilterOptionString(name.c_str(), filter, v);
	m_settings.setValue(name.c_str(), v.c_str());
}
void PlayerMenus::onVLCAoutStringSelectListMenu(MenuComponentValue const&, std::string const& filter, std::string const& name)
{
	std::string current = vlc->getAoutFilterOptionString(name.c_str());
	m_optionsMenu->addMenuItem( TRANS("Disable"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onVLCAoutStringSelect, this, _1, filter, name, std::string("")), current.empty()?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);

	std::pair<std::string, std::vector<std::pair<std::string, std::string> > > res = vlc->getConfigOptionInfoString(name.c_str());
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		m_optionsMenu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onVLCAoutStringSelect, this, _1, filter, name, it->first), it->first==current?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}

}

void PlayerMenus::onMenuFullscreen(MenuComponentValue const&, bool fs)
{
	m_viewHandler.setFullScreen(fs);

}

void PlayerMenus::onMenuAudioTrack (MenuComponentValue const&, int id)
{
	vlc->setAudioTrack(id);

}
void PlayerMenus::onMenuAudioTrackList (MenuComponentValue const&)
{
	int current = vlc->getAudioTrack();
	std::vector<std::pair<int, std::string> > list = vlc->getAudioTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		m_optionsMenu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuAudioTrack, this, _1, it->first), it->first==current?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}

}

void PlayerMenus::onMenuVideoTrack (MenuComponentValue const&, int id)
{
	vlc->setVideoTrack(id);
}
void PlayerMenus::onMenuVideoTrackList (MenuComponentValue const&)
{
	int current = vlc->getVideoTrack();
	std::vector<std::pair<int, std::string> > list = vlc->getVideoTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		m_optionsMenu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuVideoTrack, this, _1, it->first), it->first==current?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}
}
void PlayerMenus::onVLCAudioChannelSelect(MenuComponentValue const&)
{
	VLCWrapper::AudioChannel c = vlc->getAudioChannel();

	m_optionsMenu->addMenuItem(TRANS("Stereo"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc, VLCWrapper::VLCWrapperAudioChannel_Stereo), c==VLCWrapper::VLCWrapperAudioChannel_Stereo?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem(TRANS("Reverse"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc, VLCWrapper::VLCWrapperAudioChannel_RStereo), c==VLCWrapper::VLCWrapperAudioChannel_RStereo?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem(TRANS("Left"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc, VLCWrapper::VLCWrapperAudioChannel_Left), c==VLCWrapper::VLCWrapperAudioChannel_Left?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem(TRANS("Right"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc, VLCWrapper::VLCWrapperAudioChannel_Right), c==VLCWrapper::VLCWrapperAudioChannel_Right?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem(TRANS("Dolby"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc, VLCWrapper::VLCWrapperAudioChannel_Dolbys), c==VLCWrapper::VLCWrapperAudioChannel_Dolbys?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);

}

void PlayerMenus::onVLCAudioOutputDeviceSelect(MenuComponentValue const&, std::string const& output, std::string const& device)
{
	vlc->setAudioOutputDevice(output, device);
	m_settings.setValue(SETTINGS_AUDIO_OUTPUT, juce::String(output.c_str()));
	m_settings.setValue(SETTINGS_AUDIO_DEVICE, juce::String(device.c_str()));

}
void PlayerMenus::onVLCAudioOutputSelect(MenuComponentValue const&, std::string const& output, std::vector< std::pair<std::string, std::string> > const& list)
{
	for(std::vector< std::pair<std::string, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		bool selected = m_settings.getValue(SETTINGS_AUDIO_DEVICE)==juce::String(it->second.c_str());
		m_optionsMenu->addMenuItem(it->first, AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onVLCAudioOutputDeviceSelect, this, _1, output, it->second), selected?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}

}
void PlayerMenus::onVLCAudioOutputList(MenuComponentValue const& value)
{
	std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > > list = vlc->getAudioOutputList();

	if(list.size() == 1)
	{
		onVLCAudioOutputSelect(value, list.front().first.second, list.front().second);
	}
	else
	{
		for(std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > >::const_iterator it = list.begin();it != list.end();++it)
		{
			bool selected = m_settings.getValue(SETTINGS_AUDIO_OUTPUT)==juce::String(it->first.second.c_str());
			m_optionsMenu->addMenuItem(it->first.first, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCAudioOutputSelect, this, _1, it->first.second, it->second), selected?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
		}
	}
}
void PlayerMenus::onMenuSoundOptions(MenuComponentValue const&)
{
	m_optionsMenu->addMenuItem( TRANS("Delay"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuShiftAudioSlider, this, _1),
		AbstractMenuItem::Icon::None, SettingSlider::Params{"%+.3fs", vlc->getSubtitleDelay()/1000000., 0., -2., 2., .01, 2.});
	m_optionsMenu->addMenuItem( TRANS("Equalizer"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		std::bind(&PlayerMenus::onVLCAoutStringSelectListMenu, this, _1, std::string(AOUT_FILTER_EQUALIZER), std::string(CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET)));
	m_optionsMenu->addMenuItem( TRANS("Select Track"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		std::bind(&PlayerMenus::onMenuAudioTrackList, this, _1), AbstractMenuItem::Icon::Folder);
	bool currentStatus=vlc->getConfigOptionBool(CONFIG_STRING_OPTION_AUDIO_OUT);
	m_optionsMenu->addMenuItem( currentStatus?TRANS("Disable"):TRANS("Enable"), AbstractMenuItem::REFRESH_MENU,
		std::bind(&VLCWrapper::setConfigOptionBool, vlc, CONFIG_STRING_OPTION_AUDIO_OUT, !currentStatus));
	m_optionsMenu->addMenuItem( TRANS("Channel"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCAudioChannelSelect, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Output"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCAudioOutputList, this, _1));
//	m_optionsMenu->addMenuItem( TRANS("Audio visu."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCOptionStringMenu, this, _1, std::string(CONFIG_STRING_OPTION_AUDIO_VISUAL)));

}

void PlayerMenus::onMenuSetAspectRatio(MenuComponentValue const&, juce::String const& ratio)
{
		vlc->setAspect(ratio.getCharPointer().getAddress());

}
void PlayerMenus::onMenuRatio(MenuComponentValue const&)
{
		std::string current = vlc->getAspect();

	m_optionsMenu->addMenuItem( TRANS("Original"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("")), current==""?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "1:1", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("1:1")), current=="1:1"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "4:3", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("4:3")), current=="4:3"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "16:10", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("16:10")), current=="16:10"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "16:9", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("16:9")), current=="16:9"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "2.21:1", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("2.21:1")), current=="2.21:1"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "2.35:1", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("2.35:1")), current=="2.35:1"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "2.39:1", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("2.39:1")), current=="2.39:1"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( "5:4", AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuSetAspectRatio, this, _1, juce::String("5:4")), current=="5:4"?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);


}

void PlayerMenus::onMenuVideoAdjustOptions(MenuComponentValue const&)
{
	m_optionsMenu->addMenuItem( TRANS("Enable"), AbstractMenuItem::REFRESH_MENU,
		std::bind(&PlayerMenus::onMenuVideoAdjust, this, _1),
		vlc->getVideoAdjust()?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( TRANS("Contrast"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuVideoContrast, this, _1), AbstractMenuItem::Icon::None,
		SettingSlider::Params{"%.2f", vlc->getVideoContrast(), 1., 0., 2., .01});
	m_optionsMenu->addMenuItem( TRANS("Brightness"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuVideoBrightness, this, _1), AbstractMenuItem::Icon::None,
		SettingSlider::Params{"%.2f", vlc->getVideoBrightness(), 1., 0., 2., .01});
	m_optionsMenu->addMenuItem( TRANS("Saturation"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuVideoSaturation, this, _1), AbstractMenuItem::Icon::None,
		SettingSlider::Params{"%.2f", vlc->getVideoSaturation(), 1., 0., 2., .01});
	m_optionsMenu->addMenuItem( TRANS("Hue"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuVideoHue, this, _1), AbstractMenuItem::Icon::None,
		SettingSlider::Params{"", vlc->getVideoHue(), 0., 0., 256.});
	m_optionsMenu->addMenuItem( TRANS("Gamma"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuVideoGamma, this, _1), AbstractMenuItem::Icon::None,
		SettingSlider::Params{"%.2f", vlc->getVideoGamma(), 1., 0., 2., .01});

}

void PlayerMenus::onMenuVideoAdjust (MenuComponentValue const&)
{
	vlc->setVideoAdjust(!vlc->getVideoAdjust());

}

void PlayerMenus::onMenuVideoContrast (MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setVideoAdjust(true);
		vlc->setVideoContrast(*doublePtr);
	}
}
void  PlayerMenus::onMenuVideoBrightness (MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setVideoAdjust(true);
		vlc->setVideoBrightness(*doublePtr);
	}
}
void  PlayerMenus::onMenuVideoHue (MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setVideoAdjust(true);
		vlc->setVideoHue(*doublePtr);
	}
}
void  PlayerMenus::onMenuVideoSaturation (MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setVideoAdjust(true);
		vlc->setVideoSaturation(*doublePtr);
	}
}
void  PlayerMenus::onMenuVideoGamma (MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setVideoAdjust(true);
		vlc->setVideoGamma(*doublePtr);
	}
}

void  PlayerMenus::onMenuZoomSlider (MenuComponentValue const& value)
{
	if ( double const* doublePtr = std::get_if<double>(&value) )
	{
		vlc->setScale(*doublePtr);
	}
}

void PlayerMenus::onMenuVideoOptions(MenuComponentValue const&)
{
	m_optionsMenu->addMenuItem( TRANS("Speed"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuRateListAndSlider, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Crop"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuCropList, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Scale"), AbstractMenuItem::STORE_AND_OPEN_SLIDER,
		std::bind(&PlayerMenus::onMenuZoomSlider, this, _1), AbstractMenuItem::Icon::None,
		SettingSlider::Params{"%.f%%", vlc->getScale(), 100., 25., 400.});
	m_optionsMenu->addMenuItem( TRANS("Aspect Ratio"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuRatio, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Select Track"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuVideoTrackList, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Adjust"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuVideoAdjustOptions, this, _1), AbstractMenuItem::Icon::Sliders);
//m_optionsMenu->addMenuItem( TRANS("Quality"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_QUALITY)));
	m_optionsMenu->addMenuItem( TRANS("Deinterlace"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_DEINTERLACE)));
	m_optionsMenu->addMenuItem( TRANS("Deint. mode"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onVLCOptionStringMenu, this, _1, std::string(CONFIG_STRING_OPTION_VIDEO_DEINTERLACE_MODE)));

}
void PlayerMenus::onMenuExit(MenuComponentValue const&)
{
	m_fileMenu->addMenuItem( TRANS("Confirm Exit"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&PlayerMenus::onMenuExitConfirmation, this, _1), AbstractMenuItem::Icon::Exit);
}

void PlayerMenus::onOptionMenuExit(MenuComponentValue const&)
{
	//m_optionsMenu->addMenuItem( TRANS(""), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, [](MenuComponentValue const&){});//dirty workaround, shame, shame
	m_optionsMenu->addMenuItem( TRANS("Confirm Exit"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&PlayerMenus::onMenuExitConfirmation, this, _1), AbstractMenuItem::Icon::Exit);
	//m_optionsMenu->addMenuItem( TRANS(""), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, [](MenuComponentValue const&){});//dirty workaround, shame, shame
}
void PlayerMenus::onMenuExitConfirmation(MenuComponentValue const&)
{
	saveCurrentMediaTime();
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void PlayerMenus::onPlaylistItem(MenuComponentValue const&, int index)
{
	saveCurrentMediaTime();

	std::string name;
	try
	{
		std::vector<std::string > list = vlc->getCurrentPlayList();
		name = list.at(index);
	}
	catch(std::exception const& )
	{
	}

	m_viewHandler.playPlayListItem(index, name);
}

void PlayerMenus::appendAndPlay(std::string const& path)
{
	saveCurrentMediaTime();

	if(!vlc)
	{
		return;
	}
	std::string::size_type i = path.find_last_of("/\\");
	std::string name =  i == std::string::npos ? path : path.substr(i+1);

	int index = vlc->addPlayListItem(path);
	m_viewHandler.playPlayListItem(index, name);
}

void PlayerMenus::onShowPlaylist(MenuComponentValue const&)
{
	int current = vlc->getCurrentPlayListItemIndex ();
	std::vector<std::string > list = vlc->getCurrentPlayList();
	int i=0;
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{
		m_optionsMenu->addMenuItem(juce::CharPointer_UTF8(it->c_str()), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onPlaylistItem, this, _1, i), i==current?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
		++i;
	}
}
void PlayerMenus::onLanguageSelect(MenuComponentValue const&, std::string const& lang)
{
	Languages::getInstance().setCurrentLanguage(lang);

	m_settings.setValue(SETTINGS_LANG, lang.c_str());
}
void PlayerMenus::onLanguageOptions(MenuComponentValue const&)
{
	std::vector< std::string > list = Languages::getInstance().getLanguages();
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{
		m_optionsMenu->addMenuItem(it->c_str(), AbstractMenuItem::REFRESH_MENU,
			std::bind(&PlayerMenus::onLanguageSelect, this, _1, *it),
			(*it==Languages::getInstance().getCurrentLanguage())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}
}
void PlayerMenus::onSetPlayerFonSize(MenuComponentValue const&, int size)
{
	AppProportionnalComponent::setItemHeightPercentageRelativeToScreen(size);

	m_settings.setValue(SETTINGS_FONT_SIZE, size);

	m_viewHandler.handleFullRelayout();
}
void PlayerMenus::onPlayerFonSize(MenuComponentValue const&)
{
	for(int i=50;i<=175;i+=25)
	{
		m_optionsMenu->addMenuItem( juce::String::formatted("%d%%", i), AbstractMenuItem::REFRESH_MENU,
			std::bind(&PlayerMenus::onSetPlayerFonSize, this, _1, i),
			i==(AppProportionnalComponent::getItemHeightPercentageRelativeToScreen())?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	}

}

void PlayerMenus::onSetVLCOptionInt(MenuComponentValue const&, std::string const& name, int enable)
{
	vlc->setConfigOptionInt(name.c_str(), enable);

	m_settings.setValue(name.c_str(), enable);

}
void PlayerMenus::onSetVLCOption(MenuComponentValue const&, std::string const& name, bool enable)
{
	vlc->setConfigOptionBool(name.c_str(), enable);

	m_settings.setValue(name.c_str(), enable);

}
void PlayerMenus::onPlayerOptions(MenuComponentValue const&)
{
	m_optionsMenu->addMenuItem( TRANS("FullScreen"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuFullscreen, this, _1, true), m_viewHandler.isFullScreen()?AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( TRANS("Windowed"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onMenuFullscreen, this, _1, false), m_viewHandler.isFullScreen()?AbstractMenuItem::Icon::None : AbstractMenuItem::Icon::Check);

	m_optionsMenu->addMenuItem( TRANS("Language"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onLanguageOptions, this, _1), AbstractMenuItem::Icon::Folder);
	m_optionsMenu->addMenuItem( TRANS("Menu font size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onPlayerFonSize, this, _1));

	m_optionsMenu->addMenuItem( TRANS("Hardware"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), true), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)? AbstractMenuItem::Icon::Check : AbstractMenuItem::Icon::None);
	m_optionsMenu->addMenuItem( TRANS("No hardware"), AbstractMenuItem::REFRESH_MENU, std::bind(&PlayerMenus::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), false), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)? AbstractMenuItem::Icon::None :AbstractMenuItem::Icon::Check);

	m_optionsMenu->addMenuItem( TRANS("Exit"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onOptionMenuExit, this, _1), AbstractMenuItem::Icon::Exit);

}
void PlayerMenus::onOptionMenuRoot(MenuComponentValue const&)
{
	// TODO is it usefull for ninge watching? m_optionsMenu->addMenuItem( TRANS("Now playing"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onShowPlaylist, this, _1), AbstractMenuItem::Icon::Folder);
	m_optionsMenu->addMenuItem( TRANS("Subtitles"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuSubtitleMenu, this, _1), AbstractMenuItem::Icon::Subtitles);
	m_optionsMenu->addMenuItem( TRANS("Sound"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuSoundOptions, this, _1), AbstractMenuItem::Icon::Audio);
	m_optionsMenu->addMenuItem( TRANS("Video"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onMenuVideoOptions, this, _1), AbstractMenuItem::Icon::Display);
	m_optionsMenu->addMenuItem( TRANS("Player"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onPlayerOptions, this, _1), AbstractMenuItem::Icon::Settings);

}
void PlayerMenus::initFromMediaDependantSettings()
{
	vlc->setVolume(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));

	vlc->setCrop(m_settings.getValue(SETTINGS_CROP, "").toUTF8().getAddress());

	vlc->setAudioOutputDevice(m_settings.getValue(SETTINGS_AUDIO_OUTPUT).toUTF8().getAddress(), m_settings.getValue(SETTINGS_AUDIO_DEVICE).toUTF8().getAddress());
}
void PlayerMenus::initBoolSetting(const char* name)
{
	vlc->setConfigOptionBool(name, m_settings.getBoolValue(name, vlc->getConfigOptionBool(name)));
}
void PlayerMenus::initIntSetting(const char* name)
{
	initIntSetting(name, vlc->getConfigOptionInt(name));
}
void PlayerMenus::initIntSetting(const char* name, int defaultVal)
{
	vlc->setConfigOptionInt(name, m_settings.getIntValue(name, defaultVal));
}
void PlayerMenus::initStrSetting(const char* name)
{
	vlc->setConfigOptionString(name, m_settings.getValue(name, vlc->getConfigOptionString(name).c_str()).toUTF8().getAddress());
}

void PlayerMenus::initFromSettings()
{
	m_viewHandler.setFullScreen(m_settings.getBoolValue(SETTINGS_FULLSCREEN, true));
	juce::File shortcuts(juce::File::getCurrentWorkingDirectory().getChildFile(SHORTCUTS_FILE));
	Languages::getInstance().setCurrentLanguage(m_settings.getValue(SETTINGS_LANG, "").toUTF8().getAddress());
	AppProportionnalComponent::setItemHeightPercentageRelativeToScreen(m_settings.getIntValue(SETTINGS_FONT_SIZE, 100));
	if(shortcuts.exists())
	{
		shortcuts.readLines(m_shortcuts);
	}
	m_autoSubtitlesHeight=m_settings.getBoolValue(SETTINGS_AUTO_SUBTITLES_HEIGHT, m_autoSubtitlesHeight);

	initBoolSetting(CONFIG_BOOL_OPTION_HARDWARE);

	initIntSetting(CONFIG_INT_OPTION_SUBTITLE_SIZE);
	initIntSetting(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_THICKNESS);
	initIntSetting(CONFIG_INT_OPTION_SUBTITLE_MARGIN);
	initIntSetting(CONFIG_INT_OPTION_SUBTITLE_OPACITY);

	//initIntSetting(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_OPACITY);
	//initIntSetting(CONFIG_INT_OPTION_SUBTITLE_SHADOW_OPACITY);
	initIntSetting(CONFIG_INT_OPTION_SUBTITLE_BACKGROUND_OPACITY);

	initIntSetting(CONFIG_COLOR_OPTION_SUBTITLE_COLOR);
	initIntSetting(CONFIG_COLOR_OPTION_SUBTITLE_BACKGROUND_COLOR);
	initIntSetting(CONFIG_COLOR_OPTION_SUBTITLE_OUTLINE_COLOR);
	initIntSetting(CONFIG_COLOR_OPTION_SUBTITLE_SHADOW_COLOR);

	initIntSetting(CONFIG_INT_OPTION_VIDEO_QUALITY);
	initIntSetting(CONFIG_INT_OPTION_VIDEO_DEINTERLACE);
	initStrSetting(CONFIG_STRING_OPTION_VIDEO_DEINTERLACE_MODE);

	std::string audioFilters;
	juce::String preset = m_settings.getValue(CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET, juce::String(vlc->getConfigOptionString(CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET).c_str()));
	if(!preset.isEmpty())
	{
		if(!audioFilters.empty())
		{
			audioFilters += ":";
		}
		audioFilters += AOUT_FILTER_EQUALIZER;
		vlc->setConfigOptionString(CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET, preset.toUTF8().getAddress());
	}

	if(!audioFilters.empty())
	{
		vlc->setConfigOptionString("audio-filter", AOUT_FILTER_EQUALIZER);
	}

}


struct MediaTimeSorter
{
	juce::PropertySet const& propertySet;
	MediaTimeSorter(juce::PropertySet const& propertySet_):propertySet(propertySet_) {}
	int compareElements(juce::String const& some, juce::String const& other)
	{
		return propertySet.getIntValue(other, 0) - propertySet.getIntValue(some, 0);
	}
};

void PlayerMenus::saveCurrentMediaTime()
{
	if(!vlc || !vlc->isPlaying())
	{
		return;
	}
	std::string media = vlc->getCurrentPlayListItem();
	if(media.empty())
	{

		return;
	}
	int64_t time = vlc->GetTime();
	m_mediaTimes.setValue(media.c_str(), std::floor(time / 1000.));

	//clear old times
	juce::StringArray props = m_mediaTimes.getAllProperties().getAllKeys();
	while(m_mediaTimes.getAllProperties().getAllKeys().size()>MAX_MEDIA_TIME_IN_SETTINGS)
	{
		m_mediaTimes.removeValue(props[0]);
	}
	/*
	MediaTimeSorter sorter(m_mediaTimes);
	props.sort(sorter);
	for(int i=MAX_MEDIA_TIME_IN_SETTINGS;i<props.size();++i)
	{
		m_mediaTimes.removeValue(props[i]);
	}*/
}

int PlayerMenus::getMediaSavedTime(std::string const& name) const
{
	return m_mediaTimes.getIntValue(name.c_str(), 0);
}

void PlayerMenus::listShortcuts(AbstractMenu& menu, MenuComponentValue const&, FileMethod const& fileMethod, juce::StringArray const& shortcuts)
{
	for(int i=0;i<shortcuts.size();++i)
	{
		juce::File path(shortcuts[i]);
		juce::String driveRoot = path.getFullPathName().upToFirstOccurrenceOf(juce::File::getSeparatorString(), false, false);
		juce::String drive = path.getVolumeLabel().isEmpty() ? driveRoot : (path.getVolumeLabel()+"("+driveRoot + ")" );
		menu.addMenuItem(path.getFileName() + "-" + drive, AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
			[fileMethod, path](MenuComponentValue const& v){return fileMethod(v, path);}, AbstractMenuItem::Icon::FolderShortcut);
	}
}


void PlayerMenus::listRootFiles(AbstractMenu& menu, MenuComponentValue const&, FileMethod const& fileMethod)
{
	juce::Array<juce::File> destArray;
	juce::File::findFileSystemRoots(destArray);

	for(int i=0;i<destArray.size();++i)
	{
		juce::File const& file(destArray[i]);
		menu.addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
			[file, fileMethod](MenuComponentValue const& v){return fileMethod(v, file);}, getIcon(file));
	}

}

void PlayerMenus::listRecentPath(AbstractMenu& menu, MenuComponentValue const&, FileMethod const&  fileMethod, juce::File const& path)
{
	if(path.exists())
	{
		juce::File f(path);
		//try to re-build file folder hierarchy
		if(!f.isDirectory())
		{
			f = f.getParentDirectory();
		}
		juce::Array<juce::File> parentFolders;
		juce::File p = f.getParentDirectory();
		while(p.getFullPathName() != f.getFullPathName())
		{
			parentFolders.add(f);
			f = p;
			p = f.getParentDirectory();
		}
		parentFolders.add(f);

		//re-create shortcuts as if the user browsed to the last used folder
		for(int i=parentFolders.size()-1;i>=0;--i)
		{
			juce::File const& file(parentFolders[i]);
			menu.addRecentMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY,
				[file, fileMethod](MenuComponentValue const& v){return fileMethod(v, file);}, AbstractMenuItem::Icon::Back);
		}
		//select the last item
		menu.forceMenuRefresh();
	}
}


void PlayerMenus::mayOpen(juce::String const& str)
{
	juce::File path = (str.startsWith("")&&str.endsWith(""))?str.substring(1, str.length()-2):str;
	std::deque<juce::File> q;
	if(path.isDirectory())
	{
		// path is folder
		q.emplace_back(path);
	}
	// parents
	for(juce::File folder = path.getParentDirectory();
		folder.isDirectory() && (q.empty() || folder!=q.front());
		folder = q.front().getParentDirectory())
	{
		q.emplace_front(folder);
	}
	// unwind
	for(juce::File const& folder:q)
	{
		m_fileMenu->addRecentMenuItem( name(folder), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
			[this, folder](MenuComponentValue const& v){return this->onMenuOpenFolder(v, folder);}, AbstractMenuItem::Icon::Back);
	}
	m_fileMenu->forceMenuRefresh();
	if(path.existsAsFile() && Extensions::get().videoExtensions().find(path.getFileExtension().substring(1))!=Extensions::get().videoExtensions().end())
	{
		//open
		onMenuOpenFile({}, path);
	}
}
void PlayerMenus::listFiles(AbstractMenu& menu, MenuComponentValue const&, juce::File const& file, FileMethod const& fileMethod, FileMethod const& folderMethod)
{
	if(file.isDirectory())
	{
		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false);
		FileSorter sorter(Extensions::get().supportedExtensions());
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& f(destArray[i]);
			menu.addMenuItem( name(f), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, [=](MenuComponentValue const& v){folderMethod(v, f);}, AbstractMenuItem::Icon::Folder);

		}
		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& f(destArray[i]);
			menu.addMenuItem( name(f), AbstractMenuItem::EXECUTE_ONLY, [=](MenuComponentValue const& v){return fileMethod(v, f);}, getIcon(f));

		}
	}
}