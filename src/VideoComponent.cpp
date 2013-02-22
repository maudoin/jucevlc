
#include "VideoComponent.h"
#include "Icons.h"
#include "MenuTree.h"
#include "MenuTreeAction.h"
#include <algorithm>

#define DISAPEAR_DELAY_MS 6000
#define DISAPEAR_SPEED_MS 500

#define SETTINGS_FULLSCREEN "SETTINGS_FULLSCREEN"
#define SETTINGS_VOLUME "SETTINGS_VOLUME"
#define SETTINGS_LAST_OPEN_PATH "SETTINGS_LAST_OPEN_PATH"
#define SHORTCUTS_FILE "shortcuts.list"

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

////////////////////////////////////////////////////////////
//
// NATIVE VLC COMPONENT
//
////////////////////////////////////////////////////////////
class VLCNativePopupComponent   : public juce::Component
{
public:
	VLCNativePopupComponent()
		:juce::Component("vlcPopupComponent")
	{
		setOpaque(true);
		addToDesktop(juce::ComponentPeer::windowIsTemporary);  
		setMouseClickGrabsKeyboardFocus(false);
	}
	virtual ~VLCNativePopupComponent(){}
	void paint (juce::Graphics& g)
	{
	}
};
	

////////////////////////////////////////////////////////////
//
// MAIN COMPONENT
//
////////////////////////////////////////////////////////////
VideoComponent::VideoComponent()
#ifdef BUFFER_DISPLAY
	:img(new juce::Image(juce::Image::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
#else
	:juce::Component("MainFrame")
#endif
	,m_settings(juce::File::getCurrentWorkingDirectory().getChildFile("settings.xml"), options())
{    

    itemImage = juce::Drawable::createFromImageData (blue_svg, blue_svgSize);
    folderImage = juce::Drawable::createFromImageData (folder_svg, folder_svgSize);
    folderShortcutImage = juce::Drawable::createFromImageData (folderShortcut_svg, folderShortcut_svgSize);
    audioImage = juce::Drawable::createFromImageData (audio_svg, audio_svgSize);
    displayImage = juce::Drawable::createFromImageData (display_svg, display_svgSize);
    subtitlesImage = juce::Drawable::createFromImageData (sub_svg, sub_svgSize);
    exitImage = juce::Drawable::createFromImageData (exit_svg, exit_svgSize);


	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	controlComponent = new ControlComponent ();
	controlComponent->slider().addListener(this);
	controlComponent->playPauseButton().addListener(this);
	controlComponent->stopButton().addListener(this);

	tree = new MenuTree ();
	tree->setItemImage(getItemImage());
	tree->setFolderImage(getFolderImage());
	tree->setFolderShortcutImage(getFolderShortcutImage());
	
    addChildComponent(controlComponent);
    addAndMakeVisible (tree);

	sliderUpdating = false;
	videoUpdating = false;
		
	
	//after set Size
	vlc = new VLCWrapper();

#ifdef BUFFER_DISPLAY

	vlc->SetDisplayCallback(this);
#else
	vlcNativePopupComponent = new VLCNativePopupComponent();
	vlc->SetOutputWindow(vlcNativePopupComponent->getWindowHandle());

#endif
	
    vlc->SetEventCallBack(this);

	tree->setRootAction(Action::build(*this, &VideoComponent::onMenuRoot));
		
	////////////////
	tree->setScaleComponent(this);
	controlComponent->setScaleComponent(this);

	
    defaultConstrainer.setMinimumSize (100, 100);
    addChildComponent (resizableBorder = new juce::ResizableBorderComponent (this, &defaultConstrainer));

	addKeyListener(this);
		
    // And show it!
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);

	vlc->SetInputCallBack(this);
	mousehookset=  false;

	
	showVolumeSlider();

	addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar);  
	

	initFromSettings();

    setVisible (true);

	invokeLater = new vf::GuiCallQueue();

}

VideoComponent::~VideoComponent()
{    
	invokeLater = nullptr;

	controlComponent->slider().removeListener(this);
	controlComponent->playPauseButton().removeListener(this);
	controlComponent->stopButton().removeListener(this);
	controlComponent = nullptr;
	tree = nullptr;
    vlc->SetEventCallBack(NULL);
#ifdef BUFFER_DISPLAY
	{
		vlc->SetTime(vlc->GetLength());
		vlc->Pause();
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
		vlc = nullptr;
	}
	ptr = nullptr;
	img = nullptr;
#else
	getPeer()->getComponent().removeComponentListener(this);
	vlcNativePopupComponent = nullptr;
#endif

	/////////////////////
	removeKeyListener(this);
	resizableBorder = nullptr;

	juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    // (the content component will be deleted automatically, so no need to do it here)
}

////////////////////////////////////////////////////////////
//
// GUI CALLBACKS
//
////////////////////////////////////////////////////////////

void VideoComponent::userTriedToCloseWindow()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
bool VideoComponent::keyPressed (const juce::KeyPress& key,
                            juce::Component* originatingComponent)
{
	if(key.isKeyCurrentlyDown(juce::KeyPress::returnKey) && key.getModifiers().isAltDown())
	{
		if(invokeLater)invokeLater->queuef(std::bind  (&VideoComponent::switchFullScreen,this));
		return true;
	}
	if(key.isKeyCurrentlyDown(juce::KeyPress::spaceKey))
	{
		switchPlayPause();
		return true;
	}
	return false;
	
}

bool VideoComponent::isFullScreen()const
{
	return juce::Desktop::getInstance().getKioskModeComponent() == getTopLevelComponent();
}

void VideoComponent::switchPlayPause()
{
	if(vlc->isPlaying())
	{
		vlc->Pause();
	}
	else if(vlc->isPaused())
	{
		vlc->Play();
	}
}

void VideoComponent::setFullScreen(bool fs)
{
	juce::Desktop::getInstance().setKioskModeComponent (fs?getTopLevelComponent():nullptr);
	if(!fs)
	{
		resized();
	}
	m_settings.setValue(SETTINGS_FULLSCREEN, fs);
}

void VideoComponent::switchFullScreen()
{
	setFullScreen(juce::Desktop::getInstance().getKioskModeComponent() == nullptr);
}
void VideoComponent::mouseMove (const juce::MouseEvent& e)
{
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();
}
void VideoComponent::mouseDown (const juce::MouseEvent& e)
{
	if(e.mods.isRightButtonDown())
	{
		if(invokeLater)invokeLater->queuef(boost::bind  (&Component::setVisible,tree.get(), true));
	}
	if(e.mods.isLeftButtonDown())
	{
		if(invokeLater)invokeLater->queuef(boost::bind  (&Component::setVisible,tree.get(), false));
	}
	if(!isFullScreen())
	{
		dragger.startDraggingComponent (this, e);
	}
}

void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
	if(!isFullScreen())
	{
		dragger.dragComponent (this, e, 0);
	}
}
void VideoComponent::sliderValueChanged (juce::Slider* slider)
{
	if(!vlc)
	{
		return;
	}
	if(!videoUpdating)
	{
		sliderUpdating = true;
		vlc->SetTime((int64_t)(controlComponent->slider().getValue()*vlc->GetLength()/1000.));
		sliderUpdating =false;
	}
}
void VideoComponent::buttonClicked (juce::Button* button)
{
	if(!vlc)
	{
		return;
	}
	if(button == &controlComponent->playPauseButton())
	{
		if(vlc->isPaused())
		{
			vlc->Play();
		}
		else
		{
			vlc->Pause();
		}
	}
	if(button == &controlComponent->stopButton())
	{
		vlc->Stop();
	}
}

////////////////////////////////////////////////////////////
//
// DISPLAY
//
////////////////////////////////////////////////////////////

void VideoComponent::paint (juce::Graphics& g)
{
#ifdef BUFFER_DISPLAY
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
	{
		g.drawImage(*img, 0, 0, getWidth(), getHeight(), 0, 0, img->getWidth(), img->getHeight());
	}
#else
	if(!vlcNativePopupComponent->isVisible())
	{
		g.fillAll (juce::Colours::black);
	}
	else
	{
		//g.fillAll (juce::Colours::black);
	}
#endif
	
}
	
void VideoComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();
	
	int hMargin = 0.025*w;
	int treeWidth = (browsingFiles?3:1)*w/4;
	int controlHeight = 0.06*w;
	
    tree->setBounds (w-treeWidth, hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2);
	controlComponent->setBounds (hMargin, h-controlHeight, w-2*hMargin, controlHeight);

#ifdef BUFFER_DISPLAY
	if(vlc)
	{
		//rebuild buffer
		bool restart(vlc->isPaused());
		vlc->Pause();

		const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);

		std::ostringstream oss;
		oss << "VLC "<< vlc->getInfo()<<"\n";
		oss << getWidth()<<"x"<< getHeight();
		juce::Graphics g(*img);
		g.fillAll(juce::Colour::fromRGB(0, 0, 0));
		g.setColour(juce::Colour::fromRGB(255, 0, 255));
		g.drawText(oss.str().c_str(), juce::Rectangle<int>(0, 0, img->getWidth(), img->getHeight()/10), juce::Justification::bottomLeft, true);
		if(restart)
		{
			vlc->Play();
		}
	}
#else
	vlcNativePopupComponent->setBounds(getScreenX(), getScreenY(), w, h);

	if(vlcNativePopupComponent->getPeer() && getPeer())
	{
		if(getPeer())getPeer()->toBehind(vlcNativePopupComponent->getPeer());
		toFront(false);
	}
#endif
    if (resizableBorder != nullptr)
    {
        resizableBorder->setVisible (! (isFullScreen() ));

        resizableBorder->setBorderThickness (juce::BorderSize<int> (2));
        resizableBorder->setSize (w, h);
		resizableBorder->toFront(false);
    }
	
}


////////////////////////////////////////////////////////////
//
// MEDIA PLAYER METHODS
//
////////////////////////////////////////////////////////////

void VideoComponent::play(char* path)
{
	if(!vlc)
	{
		return;
	}
	vlc->OpenMedia(path);
#ifdef BUFFER_DISPLAY
	img = new juce::Image(img->rescaled(getWidth(), getHeight()));
	ptr = new juce::Image::BitmapData (*img, juce::Image::BitmapData::readWrite);
	vlc->SetBufferFormat(img->getWidth(), img->getHeight(), ptr->lineStride);
#else
    resized();
#endif

	play();
}

void VideoComponent::play()
{
	if(!vlc)
	{
		return;
	}
	controlComponent->slider().setValue(1000, juce::sendNotificationSync);

	vlc->Play();

	controlComponent->slider().setValue(0);
	
}
	
void VideoComponent::pause()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
}
void VideoComponent::stop()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
	controlComponent->slider().setValue(1000, juce::sendNotificationSync);
	vlc->Stop();
}





#ifdef BUFFER_DISPLAY

void *VideoComponent::vlcLock(void **p_pixels)
{
	imgCriticalSection.enter();
	if(ptr)
	{
		*p_pixels = ptr->getLinePointer(0);
	}
	return NULL; /* picture identifier, not needed here */
}

void VideoComponent::vlcUnlock(void *id, void *const *p_pixels)
{
	imgCriticalSection.exit();

	jassert(id == NULL); /* picture identifier, not needed here */
}

void VideoComponent::vlcDisplay(void *id)
{
	if(invokeLater)invokeLater->queuef(std::bind  (&VideoComponent::repaint,this));
	jassert(id == NULL);
}
#else

void VideoComponent::componentMovedOrResized(Component &  component,bool wasMoved, bool wasResized)
{
    resized();
}
void VideoComponent::componentVisibilityChanged(Component &  component)
{
    resized();
}

#endif

void VideoComponent::showVolumeSlider()
{
	controlComponent->alternateControlComponent().show("Audio Volume: %.f%%",
		boost::bind<void>(&VLCWrapper::setVolume, vlc.get(), _1),
		vlc->getVolume(), 1., 200., .1);
}
void VideoComponent::showPlaybackSpeedSlider ()
{
	controlComponent->alternateControlComponent().show("Speed: %.f%%",
		boost::bind<void>(&VLCWrapper::setRate, vlc.get(), _1),
		vlc->getRate(), 50., 800., .1);
}
void VideoComponent::showZoomSlider ()
{
	controlComponent->alternateControlComponent().show("Zoom: %.f%%",
		boost::bind<void>(&VLCWrapper::setScale, vlc.get(), _1),
		vlc->getScale(), 50., 500., .1);
}
////////////////////////////////////////////////////////////
//
// MENU TREE CALLBACKS
//
////////////////////////////////////////////////////////////
void VideoComponent::setBrowsingFiles(bool newBrowsingFiles)
{
	if(browsingFiles != newBrowsingFiles)
	{
		browsingFiles = newBrowsingFiles;
		resized();
	}
}
void VideoComponent::onMenuListFiles(MenuTreeItem& item, AbstractFileAction* fileMethod)
{
	setBrowsingFiles();

	juce::String path = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
	juce::File f(path);
	if(item.isMenuShortcut() || path.isEmpty() || !f.exists())
	{
		item.focusItemAsMenuShortcut();
		item.addAction( "Favorites", Action::build(*this, &VideoComponent::onMenuListFavorites, fileMethod), getItemImage());
		item.addRootFiles(fileMethod);
	
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, juce::String::empty);
	}
	else
	{
		if(!f.isDirectory())
		{
			f = f.getParentDirectory();
		}
		juce::Array<juce::File> lifo;
		juce::File p = f.getParentDirectory();
		while(p.getFullPathName() != f.getFullPathName())
		{
			lifo.add(f);
			f = p;
			p = f.getParentDirectory();
		}
		lifo.add(f);

		MenuTreeItem* last =&item;
		for(int i=lifo.size()-1;i>=0;--i)
		{
			last = last->addFile(lifo[i], fileMethod->clone());
		}
		if(last)
		{
			last->forceSelection();
		}


	}
}

void VideoComponent::onMenuListFavorites(MenuTreeItem& item, AbstractFileAction* fileMethod)
{
	item.focusItemAsMenuShortcut();
	for(int i=0;i<m_shortcuts.size();++i)
	{
		juce::File path(m_shortcuts[i]);
		item.addFile(path.getVolumeLabel() + "-" + path.getFileName(), path, fileMethod->clone());
	}
}

void VideoComponent::onMenuOpenFiles(MenuTreeItem& item, AbstractFileAction* fileMethod)
{
	onMenuListFiles(item, fileMethod);
}

void VideoComponent::writeFavorites()
{
	juce::File shortcuts(juce::File::getCurrentWorkingDirectory().getChildFile(SHORTCUTS_FILE));
	m_shortcuts.removeDuplicates(true);
	m_shortcuts.removeEmptyStrings();
	shortcuts.replaceWithText(m_shortcuts.joinIntoString("\n"));
}

void VideoComponent::onMenuAddFavorite(MenuTreeItem& item, juce::String path)
{
	m_shortcuts.add(path);
	writeFavorites();

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuRemoveFavorite(MenuTreeItem& item, juce::String path)
{
	m_shortcuts.removeString(path);
	writeFavorites();
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}

void VideoComponent::onMenuOpen (MenuTreeItem& item, juce::File const& file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		item.focusItemAsMenuShortcut();
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onMenuOpen), juce::File::findDirectories|juce::File::ignoreHiddenFiles);
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onMenuOpen), juce::File::findFiles|juce::File::ignoreHiddenFiles);

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			item.addAction("Add to favorites", Action::build(*this, &VideoComponent::onMenuAddFavorite, 
				file.getFullPathName()), getItemImage());
		}
		else
		{
			item.addAction("Remove from favorites", Action::build(*this, &VideoComponent::onMenuRemoveFavorite, 
				file.getFullPathName()), getItemImage());
		}
		
	}
	else
	{
		setBrowsingFiles(false);
		invokeLater;
		play(file.getFullPathName().toUTF8().getAddress());
	}
}
void VideoComponent::onMenuSubtitleMenu(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	int cnt = vlc->getSubtitlesCount();
	int current = vlc->getCurrentSubtitleIndex();
	item.addAction( juce::String::formatted("No subtitles"), Action::build(*this, &VideoComponent::onMenuSubtitleSelect, -1), -1==current?getItemImage():nullptr);
	for(int i = 0;i<cnt;++i)
	{
		item.addAction( juce::String::formatted("Slot %d", i+1), Action::build(*this, &VideoComponent::onMenuSubtitleSelect, i), i==current?getItemImage():nullptr);
	}
	item.addAction( "Add...", Action::build(*this, &VideoComponent::onMenuListFiles, FileAction::build(*this, &VideoComponent::onMenuOpenSubtitle)));
	item.addAction( "Delay", Action::build(*this, &VideoComponent::onMenuShiftSubtitlesSlider));
}
void VideoComponent::onMenuSubtitleSelect(MenuTreeItem& item, int i)
{
	setBrowsingFiles(false);
	vlc->setSubtitleIndex(i);
}
void VideoComponent::onMenuOpenSubtitle (MenuTreeItem& item, juce::File const& file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		item.focusItemAsMenuShortcut();
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onMenuOpenSubtitle), juce::File::findDirectories|juce::File::ignoreHiddenFiles);
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onMenuOpenSubtitle), juce::File::findFiles|juce::File::ignoreHiddenFiles);
	}
	else
	{
		setBrowsingFiles(false);
		vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
	}
}
void VideoComponent::onMenuOpenPlaylist (MenuTreeItem& item, juce::File const& file)
{
}

void VideoComponent::onMenuZoom(MenuTreeItem& item, double ratio)
{
	setBrowsingFiles(false);
	vlc->setScale(ratio);

	showZoomSlider();
}
void VideoComponent::onMenuCrop (MenuTreeItem& item, juce::String ratio)
{
	setBrowsingFiles(false);

	vlc->setAutoCrop(false);
	vlc->setCrop(std::string(ratio.getCharPointer().getAddress()));

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuAutoCrop (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	vlc->setAutoCrop(true);

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuCropList (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	
	item.focusItemAsMenuShortcut();

	//item.addAction( "Auto", Action::build(*this, &VideoComponent::onMenuAutoCrop), vlc->isAutoCrop()?getItemImage():nullptr);
	std::string current = vlc->getCrop();
	std::vector<std::string> list = vlc->getCropList();
	for(std::vector<std::string>::const_iterator it = list.begin();it != list.end();++it)
	{	
		juce::String ratio(it->c_str());
		item.addAction( ratio.isEmpty()?"Original":ratio, Action::build(*this, &VideoComponent::onMenuCrop, ratio), *it==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuRate (MenuTreeItem& item, double rate)
{
	setBrowsingFiles(false);
	vlc->setRate(rate);

	showPlaybackSpeedSlider();

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuRateSlider (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	
	item.focusItemAsMenuShortcut();
	item.addAction( "50%", Action::build(*this, &VideoComponent::onMenuRate, 50.), 50==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "100%", Action::build(*this, &VideoComponent::onMenuRate, 100.), 100==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "125%", Action::build(*this, &VideoComponent::onMenuRate, 125.), 125==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "150%", Action::build(*this, &VideoComponent::onMenuRate, 150.), 150==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "200%", Action::build(*this, &VideoComponent::onMenuRate, 200.), 200==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "300%", Action::build(*this, &VideoComponent::onMenuRate, 300.), 300==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "400%", Action::build(*this, &VideoComponent::onMenuRate, 400.), 400==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "600%", Action::build(*this, &VideoComponent::onMenuRate, 600.), 600==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "800%", Action::build(*this, &VideoComponent::onMenuRate, 800.), 800==(int)(vlc->getRate())?getItemImage():nullptr);

}
void VideoComponent::onMenuShiftAudio(MenuTreeItem& item, double s)
{
	setBrowsingFiles(false);
	vlc->setAudioDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftAudioSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show("Audio offset: %+.3fs",
		boost::bind<void>(&VideoComponent::onMenuShiftAudio, boost::ref(*this), boost::ref(item), _1),
		vlc->getAudioDelay()/1000000., -2., 2., .01, 2.);
}
void VideoComponent::onMenuShiftSubtitles(MenuTreeItem& item, double s)
{
	setBrowsingFiles(false);
	vlc->setSubtitleDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftSubtitlesSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show("Subtitles offset: %+.3fs",
		boost::bind<void>(&VideoComponent::onMenuShiftSubtitles, boost::ref(*this), boost::ref(item), _1),
		vlc->getSubtitleDelay()/1000000., -2., 2., .01, 2.);
}
void VideoComponent::onMenuAudioVolume(MenuTreeItem& item, double volume)
{
	setBrowsingFiles(false);
	vlc->setVolume(volume);

	showVolumeSlider();

	m_settings.setValue(SETTINGS_VOLUME, vlc->getVolume());
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));

}

void VideoComponent::onMenuAudioVolumeSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	showVolumeSlider();
	
	item.focusItemAsMenuShortcut();
	item.addAction( "10%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 10.), 10==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "25%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 25.), 25==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "50%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 50.), 50==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "75%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 75.), 75==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "100%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 100.), 100==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "125%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 125.), 125==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "150%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 150.), 150==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "175%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 175.), 175==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "200%", Action::build(*this, &VideoComponent::onMenuAudioVolume, 200.), 200==(int)(vlc->getVolume())?getItemImage():nullptr);
}

void VideoComponent::onMenuFullscreen(MenuTreeItem& item, bool fs)
{
	setBrowsingFiles(false);
	setFullScreen(fs);

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}

void VideoComponent::onMenuAudioTrack (MenuTreeItem& item, int id)
{
	setBrowsingFiles(false);
	vlc->setAudioTrack(id);
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuAudioTrackList (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();

	int current = vlc->getAudioTrack();
	std::vector<std::pair<int, std::string> > list = vlc->getAudioTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{	
		item.addAction(it->second.c_str(), Action::build(*this, &VideoComponent::onMenuAudioTrack, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuVideoTrack (MenuTreeItem& item, int id)
{
	setBrowsingFiles(false);
	vlc->setVideoTrack(id);
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuVideoTrackList (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();

	int current = vlc->getVideoTrack();
	std::vector<std::pair<int, std::string> > list = vlc->getVideoTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{	
		item.addAction(it->second.c_str(), Action::build(*this, &VideoComponent::onMenuVideoTrack, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuSoundOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "Volume", Action::build(*this, &VideoComponent::onMenuAudioVolumeSlider));
	item.addAction( "Delay", Action::build(*this, &VideoComponent::onMenuShiftAudioSlider));
	item.addAction( "Select Track", Action::build(*this, &VideoComponent::onMenuAudioTrackList));
}

void VideoComponent::onMenuSetAspectRatio(MenuTreeItem& item, juce::String ratio)
{
	setBrowsingFiles(false);
	vlc->setAspect(ratio.getCharPointer().getAddress());
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuRatio(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "original", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("")));
	item.addAction( "1:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("1:1")));
	item.addAction( "4:3", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("4:3")));
	item.addAction( "16:10", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("16:10")));
	item.addAction( "16:9", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("16:9")));
	item.addAction( "2.21:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("2.21:1")));
	item.addAction( "2.35:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("2.35:1")));
	item.addAction( "2.39:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("2.39:1")));
	item.addAction( "5:4", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("5:4")));
	
}
void VideoComponent::onMenuVideoOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "FullScreen", Action::build(*this, &VideoComponent::onMenuFullscreen, true), isFullScreen()?getItemImage():nullptr);
	item.addAction( "Windowed", Action::build(*this, &VideoComponent::onMenuFullscreen, false), isFullScreen()?nullptr:getItemImage());
	item.addAction( "Speed", Action::build(*this, &VideoComponent::onMenuRateSlider));
	item.addAction( "Zoom", Action::build(*this, &VideoComponent::onMenuCropList));
	item.addAction( "Aspect Ratio", Action::build(*this, &VideoComponent::onMenuRatio));
	item.addAction( "Select Track", Action::build(*this, &VideoComponent::onMenuVideoTrackList));
}
void VideoComponent::onMenuExit(MenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
void VideoComponent::onMenuRoot(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "Open", Action::build(*this, &VideoComponent::onMenuOpenFiles, FileAction::build(*this, &VideoComponent::onMenuOpen)), getFolderShortcutImage());
	item.addAction( "Subtitle", Action::build(*this, &VideoComponent::onMenuSubtitleMenu), getSubtitlesImage());
	item.addAction( "Video options", Action::build(*this, &VideoComponent::onMenuVideoOptions), getDisplayImage());
	item.addAction( "Sound options", Action::build(*this, &VideoComponent::onMenuSoundOptions), getAudioImage());
	item.addAction( "Exit", Action::build(*this, &VideoComponent::onMenuExit), getExitImage());

}
////////////////////////////////////////////////////////////
//
// VLC CALLBACKS
//
////////////////////////////////////////////////////////////
void VideoComponent::vlcTimeChanged()
{
	if(!vlc)
	{
		return;
	}
	if(!mousehookset)
	{
		mousehookset = vlc->setMouseInputCallBack(this);
	}
	if(invokeLater)invokeLater->queuef(std::bind  (&VideoComponent::updateTimeAndSlider,this));
}

void VideoComponent::updateTimeAndSlider()
{
	if(!sliderUpdating)
	{
		videoUpdating = true;
		controlComponent->slider().setValue(vlc->GetTime()*1000./vlc->GetLength(), juce::sendNotificationSync);
		controlComponent->setTime(vlc->GetTime(), vlc->GetLength());
		if(invokeLater)invokeLater->queuef(std::bind  (&ControlComponent::repaint,controlComponent.get()));
		videoUpdating =false;
	}
	juce::int64 timeFromLastMouseMove = juce::Time::currentTimeMillis () - lastMouseMoveMovieTime;
	if(timeFromLastMouseMove<(DISAPEAR_DELAY_MS+DISAPEAR_SPEED_MS))
	{
		if(timeFromLastMouseMove<DISAPEAR_DELAY_MS)
		{
			setAlpha(1.f);
		}
		else
		{
			setAlpha(1.f-(float)(timeFromLastMouseMove-DISAPEAR_DELAY_MS)/(float)DISAPEAR_SPEED_MS );
		}
		//DBG ( (long)timeFromLastMouseMove  << "->" << (long)timeFromLastMouseMove-DISAPEAR_DELAY_MS << "/" << DISAPEAR_SPEED_MS << "=" << getAlpha() );
		controlComponent->setVisible(vlc->isPlaying());
	}
	else
	{
		tree->setVisible(false);
		controlComponent->setVisible(false);
	}
	
}
void VideoComponent::vlcPaused()
{
	if(invokeLater)invokeLater->queuef(std::bind  (&ControlComponent::showPausedControls,controlComponent.get()));
}
void VideoComponent::vlcStarted()
{
		
	if(invokeLater)invokeLater->queuef(std::bind  (&ControlComponent::showPlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(std::bind  (&VideoComponent::startedSynchronous,this));
}
void VideoComponent::vlcStopped()
{
	if(invokeLater)invokeLater->queuef(std::bind  (&ControlComponent::hidePlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(std::bind  (&VideoComponent::stoppedSynchronous,this));
}

void VideoComponent::vlcPopupCallback(bool rightClick)
{
	DBG("vlcPopupCallback(" << (rightClick?"rightClick":"leftClick") );

	if(invokeLater)invokeLater->queuef(boost::bind  (&Component::setVisible,tree.get(), rightClick));
	
}
void VideoComponent::vlcFullScreenControlCallback()
{
	DBG("vlcFullScreenControlCallback");
}
void VideoComponent::vlcMouseMove(int x, int y, int button)
{
	bool controlsExpired = (juce::Time::currentTimeMillis () - lastMouseMoveMovieTime) - DISAPEAR_DELAY_MS - DISAPEAR_SPEED_MS > 0;
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();
	if(controlsExpired)
	{
		//reactivateControls
		invokeLater->queuef(std::bind  (&VideoComponent::updateTimeAndSlider,this));
	}
}
void VideoComponent::vlcMouseClick(int x, int y, int button)
{
	vlcMouseMove(x, y, button);
}
void VideoComponent::startedSynchronous()
{
	
	if(!vlcNativePopupComponent->isVisible())
	{		
		initFromMediaDependantSettings();

		setAlpha(1.f);
		setOpaque(false);
		vlcNativePopupComponent->addToDesktop(juce::ComponentPeer::windowIsTemporary); 
		controlComponent->setVisible(true);
		vlcNativePopupComponent->setVisible(true);

		getPeer()->getComponent().removeComponentListener(this);
		getPeer()->getComponent().addComponentListener(this);

		resized();
	}
}
void VideoComponent::stoppedSynchronous()
{
	
	if(vlcNativePopupComponent->isVisible())
	{
		setAlpha(1.f);
		setOpaque(true);
		vlcNativePopupComponent->setVisible(false);
		getPeer()->getComponent().removeComponentListener(this);
	}
}

void VideoComponent::initFromMediaDependantSettings()
{
	vlc->setVolume(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));
}

void VideoComponent::initFromSettings()
{
	setFullScreen(m_settings.getBoolValue(SETTINGS_FULLSCREEN, true));
	juce::File shortcuts(juce::File::getCurrentWorkingDirectory().getChildFile(SHORTCUTS_FILE));
	if(shortcuts.exists())
	{
		shortcuts.readLines(m_shortcuts);
	}
}

