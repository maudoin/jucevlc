
#include "VideoComponent.h"
#include "Icons.h"
#include "MenuTree.h"
#include "MenuTreeAction.h"
#include "Languages.h"
#include <algorithm>

#define DISAPEAR_DELAY_MS 6000
#define DISAPEAR_SPEED_MS 500

#define SETTINGS_FULLSCREEN "SETTINGS_FULLSCREEN"
#define SETTINGS_VOLUME "SETTINGS_VOLUME"
#define SETTINGS_LAST_OPEN_PATH "SETTINGS_LAST_OPEN_PATH"
#define SETTINGS_LANG "SETTINGS_LANG"
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
    hideFolderShortcutImage = juce::Drawable::createFromImageData (hideFolderShortcut_svg, hideFolderShortcut_svgSize);
    audioImage = juce::Drawable::createFromImageData (audio_svg, audio_svgSize);
    displayImage = juce::Drawable::createFromImageData (display_svg, display_svgSize);
    subtitlesImage = juce::Drawable::createFromImageData (sub_svg, sub_svgSize);
    exitImage = juce::Drawable::createFromImageData (exit_svg, exit_svgSize);


	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	controlComponent = new ControlComponent ();
	controlComponent->slider().addListener(this);
	controlComponent->playPauseButton().addListener(this);
	controlComponent->stopButton().addListener(this);
	controlComponent->menuButton().addListener(this);
	controlComponent->alternateSliderModeButton().addListener(this);
	controlComponent->addMouseListener(this, true);

	tree = new MenuTree ();
	tree->setItemImage(getItemImage());
	tree->setFolderImage(getFolderImage());
	tree->setFolderShortcutImage(getFolderShortcutImage());
	tree->addMouseListener(this, true);
	
    addChildComponent(controlComponent);
    addChildComponent (tree);
	setMenuTreeVisibleAndUpdateMenuButtonIcon(true);

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
	controlComponent->slider().setScaleComponent(this);

	
    defaultConstrainer.setMinimumSize (100, 100);
    addChildComponent (resizableBorder = new juce::ResizableBorderComponent (this, &defaultConstrainer));

	addKeyListener(this);
		
    // And show it!
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);

	vlc->SetInputCallBack(this);
	mousehookset=  false;

	
	showVolumeSlider();

	addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar);  
	
	setSize(800, 600);

	initFromSettings();

    setVisible (true);
	
	if(!isFullScreen())
	{
		//default window size if windowed
		centreWithSize(800, 600);
	}

	invokeLater = new vf::GuiCallQueue();

}

VideoComponent::~VideoComponent()
{    
	invokeLater->close();
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

	Languages::getInstance().reset();
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

	if(e.eventComponent == &controlComponent->slider())
	{
		float min = controlComponent->slider().getPositionOfValue(controlComponent->slider().getMinimum());
		float w = controlComponent->slider().getPositionOfValue(controlComponent->slider().getMaximum()) - min;
		double mouseMoveValue = (e.x - min)/w;
		controlComponent->slider().setMouseOverTime(e.x, mouseMoveValue*vlc->GetLength());
		//if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,boost::ref(controlComponent->slider())));
	}
}

void VideoComponent::mouseExit (const juce::MouseEvent& e)
{
	if(e.eventComponent == &controlComponent->slider())
	{
		controlComponent->slider().resetMouseOverTime();
		//if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,boost::ref(controlComponent->slider())));
	}
}

void VideoComponent::mouseDown (const juce::MouseEvent& e)
{
	if(e.eventComponent == this)
	{
		if(e.mods.isRightButtonDown())
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, true));
		}
		if(e.mods.isLeftButtonDown())
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, vlc->isStopped()));
		}
		if(!isFullScreen())
		{
			dragger.startDraggingComponent (this, e.getEventRelativeTo(this));
		}
	}
}

void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

	if(!isFullScreen())
	{
		dragger.dragComponent (this, e.getEventRelativeTo(this), nullptr);
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
		vlc->SetTime((int64_t)(controlComponent->slider().getValue()*vlc->GetLength()/10000.));
		sliderUpdating =false;
	}
}

void VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon(bool visible)
{
	tree->setVisible(visible);
	controlComponent->menuButton().setImages(tree->isVisible()?hideFolderShortcutImage:folderShortcutImage);
}

//==============================================================================
class DrawableMenuComponent  : public juce::PopupMenu::CustomComponent
{
	juce::Drawable* m_drawable;
	int m_size;
public:
    DrawableMenuComponent(juce::Drawable* drawable, int size)
        : m_drawable (drawable), m_size(size)
    {
    }

    ~DrawableMenuComponent()
    {
    }

    void getIdealSize (int& idealWidth,
                       int& idealHeight)
    {
        idealHeight = idealWidth = m_size;
    }

    void paint (juce::Graphics& g)
    {
		g.fillAll(juce::Colours::black);
		m_drawable->drawWithin(g, getLocalBounds().toFloat(), juce::RectanglePlacement(juce::RectanglePlacement::stretchToFit), 1.f );
    }

};
static void alternateSliderModeButtonCallback (int result, VideoComponent* videoComponent)
{
    if (result != 0 && videoComponent != 0)
        videoComponent->alternateSliderModeButton(result);
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
	else if(button == &controlComponent->stopButton())
	{
		vlc->Stop();
	}
	else if(button == &controlComponent->menuButton())
	{
		setMenuTreeVisibleAndUpdateMenuButtonIcon(!tree->isVisible());
	}
	else if(button == &controlComponent->alternateSliderModeButton())
	{
		
		int buttonWidth = 0.03*controlComponent->getWidth();

        juce::PopupMenu m;
		m.addCustomItem (E_POPUP_ITEM_VOLUME_SLIDER, new DrawableMenuComponent(audioImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_SUBTITLES_DELAY_SLIDER, new DrawableMenuComponent(subtitlesImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_VOLUME_DELAY_SLIDER,new DrawableMenuComponent(audioImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_PLAY_SPEED_SLIDER, new DrawableMenuComponent(displayImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_SHOW_CURRENT_TIME, new DrawableMenuComponent(itemImage.get(), buttonWidth));

        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (button),
                             juce::ModalCallbackFunction::forComponent (alternateSliderModeButtonCallback, this));

		//todo update icon/checked item
	}
}

void VideoComponent::alternateSliderModeButton(int result)
{
	switch(result)
	{
	case E_POPUP_ITEM_VOLUME_SLIDER:
		showVolumeSlider();
		break;
	case E_POPUP_ITEM_SUBTITLES_DELAY_SLIDER:
		showSubtitlesOffsetSlider();
		break;
	case E_POPUP_ITEM_VOLUME_DELAY_SLIDER:
		showAudioOffsetSlider ();
		break;
	case E_POPUP_ITEM_PLAY_SPEED_SLIDER:
		showPlaybackSpeedSlider();
		break;
	case E_POPUP_ITEM_SHOW_CURRENT_TIME:
		controlComponent->alternateControlComponent().setVisible(false);
		break;
	}
}
void VideoComponent::broughtToFront()
{
	juce::Component::broughtToFront();
#ifndef BUFFER_DISPLAY
	if(isVisible() && !isFullScreen()
		&& vlcNativePopupComponent && vlcNativePopupComponent->getPeer() && getPeer())
	{
		vlcNativePopupComponent->getPeer()->toBehind(getPeer());
	}
#endif//BUFFER_DISPLAY

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
#endif//BUFFER_DISPLAY
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
	controlComponent->slider().setValue(10000, juce::sendNotificationSync);

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
	controlComponent->slider().setValue(10000, juce::sendNotificationSync);
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
	controlComponent->alternateControlComponent().show(TRANS("Audio Volume: %.f%%"),
		boost::bind<void>(&VLCWrapper::setVolume, vlc.get(), _1),
		vlc->getVolume(), 1., 200., .1);
}
void VideoComponent::showPlaybackSpeedSlider ()
{
	controlComponent->alternateControlComponent().show(TRANS("Speed: %.f%%"),
		boost::bind<void>(&VLCWrapper::setRate, vlc.get(), _1),
		vlc->getRate(), 50., 800., .1);
}
void VideoComponent::showZoomSlider ()
{
	controlComponent->alternateControlComponent().show(TRANS("Zoom: %.f%%"),
		boost::bind<void>(&VLCWrapper::setScale, vlc.get(), _1),
		vlc->getScale(), 50., 500., .1);
}
void VideoComponent::showAudioOffsetSlider ()
{
	controlComponent->alternateControlComponent().show(TRANS("Audio offset: %+.3fs"),
		boost::bind<void>(&VideoComponent::onMenuShiftAudio, boost::ref(*this), _1),
		vlc->getAudioDelay()/1000000., -2., 2., .01, 2.);
}
void VideoComponent::showSubtitlesOffsetSlider ()
{
	controlComponent->alternateControlComponent().show(TRANS("Subtitles offset: %+.3fs"),
		boost::bind<void>(&VideoComponent::onMenuShiftSubtitles, boost::ref(*this), _1),
		vlc->getSubtitleDelay()/1000000., -2., 2., .01, 2.);
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

void VideoComponent::onMenuOpenUnconditionnal (MenuTreeItem& item, juce::String path)
{
	setBrowsingFiles(false);
	play(path.toUTF8().getAddress());
}
void VideoComponent::onMenuQueue (MenuTreeItem& item, juce::String path)
{
	setBrowsingFiles(false);
	vlc->addPlayListItem(path.toUTF8().getAddress());
}

void VideoComponent::onMenuOpen (MenuTreeItem& item, juce::File const& file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		item.focusItemAsMenuShortcut();
		
		item.addAction(TRANS("Play All"), Action::build(*this, &VideoComponent::onMenuOpenUnconditionnal, 
				file.getFullPathName()), getItemImage());
		item.addAction(TRANS("Add All"), Action::build(*this, &VideoComponent::onMenuQueue, 
				file.getFullPathName()), getItemImage());

		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onMenuOpen), juce::File::findDirectories|juce::File::ignoreHiddenFiles);
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onMenuOpen), juce::File::findFiles|juce::File::ignoreHiddenFiles);

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			item.addAction(TRANS("Add to favorites"), Action::build(*this, &VideoComponent::onMenuAddFavorite, 
				file.getFullPathName()), getItemImage());
		}
		else
		{
			item.addAction(TRANS("Remove from favorites"), Action::build(*this, &VideoComponent::onMenuRemoveFavorite, 
				file.getFullPathName()), getItemImage());
		}
		
	}
	else
	{
		onMenuOpenUnconditionnal(item, file.getFullPathName());
	}
}
void VideoComponent::onMenuSubtitleMenu(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	int cnt = vlc->getSubtitlesCount();
	int current = vlc->getCurrentSubtitleIndex();
	if(cnt)
	{
		item.addAction( juce::String::formatted(TRANS("Disable")), Action::build(*this, &VideoComponent::onMenuSubtitleSelect, 0), (0==current||-1==current)?getItemImage():nullptr);
		for(int i = 1;i<cnt;++i)
		{
			item.addAction( juce::String::formatted(TRANS("Slot %d"), i), Action::build(*this, &VideoComponent::onMenuSubtitleSelect, i), i==current?getItemImage():nullptr);
		}
	}
	else
	{
		item.addAction( juce::String::formatted(TRANS("No subtitles")), Action::build(*this, &VideoComponent::onMenuSubtitleSelect, -1), -1==current?getItemImage():nullptr);
	}
	item.addAction( TRANS("Add..."), Action::build(*this, &VideoComponent::onMenuListFiles, FileAction::build(*this, &VideoComponent::onMenuOpenSubtitle)));
	item.addAction( TRANS("Delay"), Action::build(*this, &VideoComponent::onMenuShiftSubtitlesSlider));
}
void VideoComponent::onMenuSubtitleSelect(MenuTreeItem& item, int i)
{
	setBrowsingFiles(false);
	vlc->setSubtitleIndex(i);

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
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

	//item.addAction( TRANS("Auto"), Action::build(*this, &VideoComponent::onMenuAutoCrop), vlc->isAutoCrop()?getItemImage():nullptr);
	std::string current = vlc->getCrop();
	std::vector<std::string> list = vlc->getCropList();
	for(std::vector<std::string>::const_iterator it = list.begin();it != list.end();++it)
	{	
		juce::String ratio(it->c_str());
		item.addAction( ratio.isEmpty()?TRANS("Original"):ratio, Action::build(*this, &VideoComponent::onMenuCrop, ratio), *it==current?getItemImage():nullptr);
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

	showPlaybackSpeedSlider();
	
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
void VideoComponent::onMenuShiftAudio(double s)
{
	setBrowsingFiles(false);
	vlc->setAudioDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftAudioSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	showAudioOffsetSlider();
}
void VideoComponent::onMenuShiftSubtitles(double s)
{
	setBrowsingFiles(false);
	vlc->setSubtitleDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftSubtitlesSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	showSubtitlesOffsetSlider();
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
void VideoComponent::onMenuVideoAdjust (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	vlc->setVideoAdjust(!vlc->getVideoAdjust());
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuVideoContrast (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show(TRANS("Contrast: %+.3fs"),
		boost::bind<void>(&VLCWrapper::setVideoContrast, vlc.get(), _1),
		vlc->getVideoContrast(), 0., 2., .01);
}
void  VideoComponent::onMenuVideoBrightness (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show(TRANS("Brightness: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoBrightness, vlc.get(), _1),
		vlc->getVideoBrightness(), 0., 2., .01);
}
void  VideoComponent::onMenuVideoHue (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show(TRANS("Hue"),
		boost::bind<void>(&VLCWrapper::setVideoHue, vlc.get(), _1),
		vlc->getVideoHue(), 0, 256., .1);
}
void  VideoComponent::onMenuVideoSaturation (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show(TRANS("Saturation: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoSaturation, vlc.get(), _1),
		vlc->getVideoSaturation(), 0., 2., .01);
}
void  VideoComponent::onMenuVideoGamma (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show(TRANS("Gamma: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoGamma, vlc.get(), _1),
		vlc->getVideoGamma(), 0., 2., .01);
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
	item.addAction( TRANS("Volume"), Action::build(*this, &VideoComponent::onMenuAudioVolumeSlider));
	item.addAction( TRANS("Delay"), Action::build(*this, &VideoComponent::onMenuShiftAudioSlider));
	item.addAction( TRANS("Select Track"), Action::build(*this, &VideoComponent::onMenuAudioTrackList));
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
	std::string current = vlc->getAspect();
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Original"), Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("")), current==""?getItemImage():nullptr);
	item.addAction( "1:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("1:1")), current=="1:1"?getItemImage():nullptr);
	item.addAction( "4:3", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("4:3")), current=="4:3"?getItemImage():nullptr);
	item.addAction( "16:10", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("16:10")), current=="16:10"?getItemImage():nullptr);
	item.addAction( "16:9", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("16:9")), current=="16:9"?getItemImage():nullptr);
	item.addAction( "2.21:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("2.21:1")), current=="2.21:1"?getItemImage():nullptr);
	item.addAction( "2.35:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("2.35:1")), current=="2.35:1"?getItemImage():nullptr);
	item.addAction( "2.39:1", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("2.39:1")), current=="2.39:1"?getItemImage():nullptr);
	item.addAction( "5:4", Action::build(*this, &VideoComponent::onMenuSetAspectRatio, juce::String("5:4")), current=="5:4"?getItemImage():nullptr);
	
}

void VideoComponent::onMenuVideoAdjustOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Enable"), Action::build(*this, &VideoComponent::onMenuVideoAdjust), vlc->getVideoAdjust()?getItemImage():nullptr);
	item.addAction( TRANS("Contrast"), Action::build(*this, &VideoComponent::onMenuVideoContrast));
	item.addAction( TRANS("Brightness"), Action::build(*this, &VideoComponent::onMenuVideoBrightness));
	item.addAction( TRANS("Saturation"), Action::build(*this, &VideoComponent::onMenuVideoSaturation));
	item.addAction( TRANS("Hue"), Action::build(*this, &VideoComponent::onMenuVideoHue));
	item.addAction( TRANS("Gamma"), Action::build(*this, &VideoComponent::onMenuVideoGamma));
}

void VideoComponent::onMenuVideoOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Speed"), Action::build(*this, &VideoComponent::onMenuRateSlider));
	item.addAction( TRANS("Zoom"), Action::build(*this, &VideoComponent::onMenuCropList));
	item.addAction( TRANS("Aspect Ratio"), Action::build(*this, &VideoComponent::onMenuRatio));
	item.addAction( TRANS("Select Track"), Action::build(*this, &VideoComponent::onMenuVideoTrackList));
	item.addAction( TRANS("Adjust"), Action::build(*this, &VideoComponent::onMenuVideoAdjustOptions));
}
void VideoComponent::onMenuExit(MenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void VideoComponent::onPlaylistItem(MenuTreeItem& item, int index)
{
	setBrowsingFiles(false);
	vlc->playPlayListItem(index);
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onShowPlaylist(MenuTreeItem& item)
{
	setBrowsingFiles(true);
	item.focusItemAsMenuShortcut();

	int current = vlc->getCurrentPlayListItemIndex ();
	std::vector<std::string > list = vlc->getCurrentPlayList();
	int i=0;
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{	
		item.addAction(it->c_str(), Action::build(*this, &VideoComponent::onPlaylistItem, i), i==current?getItemImage():nullptr);
		++i;
	}
	
}
void VideoComponent::onLanguageSelect(MenuTreeItem& item, std::string lang)
{
	setBrowsingFiles(false);
	Languages::getInstance().setCurrentLanguage(lang);
	
	m_settings.setValue(SETTINGS_LANG, lang.c_str());
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onLanguageOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();

	std::vector< std::string > list = Languages::getInstance().getLanguages();
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{	
		item.addAction(it->c_str(), Action::build(*this, &VideoComponent::onLanguageSelect, *it), (*it==Languages::getInstance().getCurrentLanguage())?getItemImage():nullptr);
	}
}
void VideoComponent::onPlayerOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("FullScreen"), Action::build(*this, &VideoComponent::onMenuFullscreen, true), isFullScreen()?getItemImage():nullptr);
	item.addAction( TRANS("Windowed"), Action::build(*this, &VideoComponent::onMenuFullscreen, false), isFullScreen()?nullptr:getItemImage());
	//item.addAction( TRANS("Language"), Action::build(*this, &VideoComponent::onLanguageOptions));
}
void VideoComponent::onMenuRoot(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Open"), Action::build(*this, &VideoComponent::onMenuOpenFiles, FileAction::build(*this, &VideoComponent::onMenuOpen)), getFolderShortcutImage());
	item.addAction( TRANS("Now playing"), Action::build(*this, &VideoComponent::onShowPlaylist), getItemImage());
	item.addAction( TRANS("Subtitle"), Action::build(*this, &VideoComponent::onMenuSubtitleMenu), getSubtitlesImage());
	item.addAction( TRANS("Video options"), Action::build(*this, &VideoComponent::onMenuVideoOptions), getDisplayImage());
	item.addAction( TRANS("Sound options"), Action::build(*this, &VideoComponent::onMenuSoundOptions), getAudioImage());
	item.addAction( TRANS("Player options"), Action::build(*this, &VideoComponent::onPlayerOptions), getItemImage());
	item.addAction( TRANS("Exit"), Action::build(*this, &VideoComponent::onMenuExit), getExitImage());

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
		controlComponent->slider().setValue(vlc->GetTime()*10000./vlc->GetLength(), juce::sendNotificationSync);
		controlComponent->setTime(vlc->GetTime(), vlc->GetLength());
		if(invokeLater)invokeLater->queuef(std::bind  (&ControlComponent::repaint,controlComponent.get()));
		videoUpdating =false;
	}
	handleIdleTimeAndControlsVisibility();
}

void VideoComponent::handleIdleTimeAndControlsVisibility()
{
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
		controlComponent->setVisible(vlc->isPlaying() || vlc->isPaused());
	}
	else
	{
		setMenuTreeVisibleAndUpdateMenuButtonIcon(false);
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

	bool showMenu = rightClick || vlc->isStopped();
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, showMenu));
	if(invokeLater)invokeLater->queuef(boost::bind  (&Component::toFront,this, true));
	
}
void VideoComponent::vlcFullScreenControlCallback()
{
	DBG("vlcFullScreenControlCallback");
}
void VideoComponent::vlcMouseMove(int x, int y, int button)
{
	bool controlsExpired = (juce::Time::currentTimeMillis () - lastMouseMoveMovieTime) - DISAPEAR_DELAY_MS - DISAPEAR_SPEED_MS > 0;
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();
	if(controlsExpired || vlc->isPaused())
	{
		//reactivateControls
		invokeLater->queuef(std::bind  (&VideoComponent::handleIdleTimeAndControlsVisibility,this));
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
		setMenuTreeVisibleAndUpdateMenuButtonIcon(true);
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
	Languages::getInstance().setCurrentLanguage(m_settings.getValue(SETTINGS_LANG, "").toUTF8().getAddress());
	if(shortcuts.exists())
	{
		shortcuts.readLines(m_shortcuts);
	}
}

