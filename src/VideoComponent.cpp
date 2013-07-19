
#include "VideoComponent.h"
#include "Icons.h"
#include "MenuTree.h"
#include "Languages.h"
#include <algorithm>
#include <set>
#include <boost/bind.hpp>

#define DISAPEAR_DELAY_MS 500
#define DISAPEAR_SPEED_MS 500

#define SETTINGS_FULLSCREEN "SETTINGS_FULLSCREEN"
#define SETTINGS_VOLUME "SETTINGS_VOLUME"
#define SETTINGS_CROP "SETTINGS_CROP"
#define SETTINGS_FONT_SIZE "SETTINGS_FONT_SIZE"
#define SETTINGS_LAST_OPEN_PATH "SETTINGS_LAST_OPEN_PATH"
#define SETTINGS_LANG "SETTINGS_LANG"
#define SETTINGS_AUTO_SUBTITLES_HEIGHT "SETTINGS_AUTO_SUBTITLES_HEIGHT"
#define SHORTCUTS_FILE "shortcuts.list"
#define MAX_MEDIA_TIME_IN_SETTINGS 30



#define EXTENSIONS_VIDEO(add) add("3g2");add("3gp");add("3gp2");add("3gpp");add("amv");add("asf");add("avi");add("bin");add("divx");add("drc");add("dv");add("f4v");add("flv");add("gxf");add("iso");add("m1v");add("m2v");\
                         add("m2t");add("m2ts");add("m4v");add("mkv");add("mov");add("mp2");add("mp2v");add("mp4");add("mp4v");add("mpa");add("mpe");add("mpeg");add("mpeg1");\
                         add("mpeg2");add("mpeg4");add("mpg");add("mpv2");add("mts");add("mtv");add("mxf");add("mxg");add("nsv");add("nuv");\
                         add("ogg");add("ogm");add("ogv");add("ogx");add("ps");\
                         add("rec");add("rm");add("rmvb");add("tod");add("ts");add("tts");add("vob");add("vro");add("webm");add("wm");add("wmv");add("flv");

#define EXTENSIONS_PLAYLIST(add) add("asx");add("b4s");add("cue");add("ifo");add("m3u");add("m3u8");add("pls");add("ram");add("rar");add("sdp");add("vlc");add("xspf");add("wvx");add("zip");add("conf");

#define EXTENSIONS_SUBTITLE(add) add("cdg");add("idx");add("srt"); \
                            add("sub");add("utf");add("ass"); \
                            add("ssa");add("aqt"); \
                            add("jss");add("psb"); \
                            add("rt");add("smi");add("txt"); \
							add("smil");add("stl");add("usf"); \
                            add("dks");add("pjs");add("mpl2");


#define EXTENSIONS_MEDIA(add) EXTENSIONS_VIDEO(add) EXTENSIONS_SUBTITLE(add) EXTENSIONS_PLAYLIST(add)

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
// TITLE COMPONENT
//
////////////////////////////////////////////////////////////
class TitleComponent   : public juce::Component
{
	juce::Component* m_componentToMove;
	juce::ComponentDragger dragger;
public:
	TitleComponent(juce::Component* componentToMove)
		:juce::Component("Title")
		,m_componentToMove(componentToMove)
	{
		setOpaque(false);
	}
	virtual ~TitleComponent(){}
	void paint (juce::Graphics& g)
	{
		char* title = "JuceVLC player";

		juce::Font f = g.getCurrentFont().withHeight((float)getHeight());
		f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);
		float textWidth = f.getStringWidthFloat(title);
		int rightBorder = (int)(getWidth() - textWidth);
		juce::Path path;
		path.lineTo(textWidth+rightBorder-2, 0);
		path.quadraticTo(textWidth+rightBorder/2.f, getHeight()-2.f, textWidth, getHeight()-2.f);
		path.lineTo(0.f, getHeight()-2.f);
		path.lineTo(0.f, 0.f);
		
		g.setColour (juce::Colours::purple.withAlpha(0.75f));
		g.fillPath(path);

		g.setColour (juce::Colours::purple.brighter());
		g.strokePath(path, juce::PathStrokeType(2));

		
		g.setColour (juce::Colours::white);
		g.drawFittedText (title,
							2, 2,getWidth()-4,getHeight()-4,
							juce::Justification::centredLeft, 
							1, //1 line
							1.f//no h scale
							);
	}
	void mouseDown (const juce::MouseEvent& e)
	{
		if(e.eventComponent == this && isVisible())
		{
			dragger.startDraggingComponent (m_componentToMove, e.getEventRelativeTo(m_componentToMove));
		}
	}

	void mouseDrag (const juce::MouseEvent& e)
	{
		if(e.eventComponent == this && isVisible())
		{
			dragger.dragComponent (m_componentToMove, e.getEventRelativeTo(m_componentToMove), nullptr);
		}
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
	:juce::Component("JuceVLC")
#endif
	,m_settings(juce::File::getCurrentWorkingDirectory().getChildFile("settings.xml"), options())
	,m_mediaTimes(juce::File::getCurrentWorkingDirectory().getChildFile("mediaTimes.xml"), options())
	,m_canHideOSD(true)
	,m_autoSubtitlesHeight(true)
{    
	Languages::getInstance();

	//m_toolTip = new juce::TooltipWindow( this,50);

    appImage = juce::ImageFileFormat::loadFrom(vlc_png, vlc_pngSize);

    itemImage = juce::Drawable::createFromImageData (blue_svg, blue_svgSize);
    folderImage = juce::Drawable::createFromImageData (folder_svg, folder_svgSize);
    playlistImage = juce::Drawable::createFromImageData (list_svg, list_svgSize);
    folderShortcutImage = juce::Drawable::createFromImageData (folderShortcut_svg, folderShortcut_svgSize);
    hideFolderShortcutImage = juce::Drawable::createFromImageData (hideFolderShortcut_svg, hideFolderShortcut_svgSize);
    audioImage = juce::Drawable::createFromImageData (audio_svg, audio_svgSize);
    displayImage = juce::Drawable::createFromImageData (display_svg, display_svgSize);
    subtitlesImage = juce::Drawable::createFromImageData (sub_svg, sub_svgSize);
    exitImage = juce::Drawable::createFromImageData (exit_svg, exit_svgSize);
    settingsImage = juce::Drawable::createFromImageData (gears_svg, gears_svgSize);
    speedImage = juce::Drawable::createFromImageData (speed_svg, speed_svgSize);
    audioShiftImage = juce::Drawable::createFromImageData (audioShift_svg, audioShift_svgSize);
    clockImage = juce::Drawable::createFromImageData (clock_svg, clock_svgSize);

	
	EXTENSIONS_VIDEO(m_videoExtensions.insert)
	EXTENSIONS_PLAYLIST(m_playlistExtensions.insert)
	EXTENSIONS_SUBTITLE(m_subtitlesExtensions.insert)

	m_suportedExtensions.push_back(m_videoExtensions);
	m_suportedExtensions.push_back(m_subtitlesExtensions);
	m_suportedExtensions.push_back(m_playlistExtensions);


	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	controlComponent = new ControlComponent ();
	controlComponent->slider().addListener(this);
	controlComponent->playPauseButton().addListener(this);
	controlComponent->stopButton().addListener(this);
	controlComponent->fullscreenButton().addListener(this);
	controlComponent->menuButton().addListener(this);
	controlComponent->auxilliarySliderModeButton().addListener(this);
	controlComponent->resetButton().addListener(this);
	controlComponent->addMouseListener(this, true);

	tree = new MenuTree ();
	tree->setItemImage(getItemImage());
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

	tree->setRootAction(boost::bind(&VideoComponent::onMenuRoot, this, _1));
		
	////////////////
	tree->setScaleComponent(this);
	controlComponent->setScaleComponent(this);
	controlComponent->slider().setScaleComponent(this);

	
    defaultConstrainer.setMinimumSize (100, 100);
	addChildComponent (titleBar = new TitleComponent(this));
	titleBar->addMouseListener(this, true);
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
	//prevent processing
	sliderUpdating = true;
	videoUpdating = true;

	Languages::getInstance().clear();

	setVisible(false);

	saveCurrentMediaTime();

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
	titleBar = nullptr;

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
		pause();
	}
	else if(vlc->isPaused())
	{
		play();
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
	m_canHideOSD = e.eventComponent == this;//cannot hide sub component while moving on sub component
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

	if(e.eventComponent == &controlComponent->slider())
	{
		float min = controlComponent->slider().getPositionOfValue(controlComponent->slider().getMinimum());
		float w = controlComponent->slider().getPositionOfValue(controlComponent->slider().getMaximum()) - min;
		double mouseMoveValue = (e.x - min)/w;
		controlComponent->slider().setMouseOverTime(e.x, (juce::int64)(mouseMoveValue*vlc->GetLength()));
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
	
	//DBG ( "click " << e.eventComponent->getName() );
	if(e.eventComponent == this)
	{
		if(e.mods.isRightButtonDown())
		{
			//prevent menu to disappear too quickly
			m_canHideOSD = false;
			lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();	

			if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, true));
		}
		if(e.mods.isLeftButtonDown())
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, vlc->isStopped()));
		}
	}
}

void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
	m_canHideOSD = e.eventComponent == this;//cannot hide sub component while dragging on sub component
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();	

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
static void auxilliarySliderModeButtonCallback (int result, VideoComponent* videoComponent)
{
    if (result != 0 && videoComponent != 0)
        videoComponent->auxilliarySliderModeButton(result);
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
			vlc->play();
		}
		else
		{
			pause();
		}
	}
	else if(button == &controlComponent->stopButton())
	{
		stop();
	}
	else if(button == &controlComponent->fullscreenButton())
	{
		switchFullScreen();
	}
	else if(button == &controlComponent->menuButton())
	{
		setMenuTreeVisibleAndUpdateMenuButtonIcon(!tree->isVisible());
	}
	else if(button == &controlComponent->resetButton())
	{
		controlComponent->auxilliaryControlComponent().reset();
	}
	else if(button == &controlComponent->auxilliarySliderModeButton())
	{
		
		int buttonWidth = (int)(0.03*controlComponent->getWidth());

        juce::PopupMenu m;
		m.addCustomItem (E_POPUP_ITEM_VOLUME_SLIDER, new DrawableMenuComponent(audioImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_SUBTITLES_DELAY_SLIDER, new DrawableMenuComponent(subtitlesImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_VOLUME_DELAY_SLIDER,new DrawableMenuComponent(audioShiftImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_PLAY_SPEED_SLIDER, new DrawableMenuComponent(speedImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_SHOW_CURRENT_TIME, new DrawableMenuComponent(clockImage.get(), buttonWidth));

        m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (button),
                             juce::ModalCallbackFunction::forComponent (auxilliarySliderModeButtonCallback, this));

		//todo update icon/checked item
	}
}

void VideoComponent::auxilliarySliderModeButton(int result)
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
		controlComponent->auxilliaryControlComponent().disableAndHide();
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
		g.drawImageAt(appImage, (getWidth() - appImage.getWidth())/2, (getHeight() - appImage.getHeight())/2 );

		
		juce::Font f = g.getCurrentFont().withHeight(tree->getFontHeight());
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);
		g.setColour (juce::Colours::grey);
		g.drawText(juce::String("Featuring VLC ") + vlc->getInfo().c_str(),(getWidth() - appImage.getWidth())/2,  
			(getHeight() + appImage.getHeight())/2, appImage.getWidth(), 
			(int)tree->getFontHeight(), 
			juce::Justification::centred, true);
	}
	else
	{
		//g.fillAll (juce::Colours::black);
	}
#endif
	
}
	
void VideoComponent::updateSubComponentsBounds()
{
	int w =  getWidth();
	int h =  getHeight();
	
	int hMargin = (int)(tree->getItemHeight()/2.);
	int treeWidth = (browsingFiles?3:1)*w/4;
	int controlHeight = 3*(int)tree->getItemHeight();
	
    tree->setBounds (w-treeWidth, hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2);
	controlComponent->setBounds (hMargin, h-controlHeight, w-2*hMargin, controlHeight);
}
	
void VideoComponent::resized()
{
	updateSubComponentsBounds();

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
	vlcNativePopupComponent->setBounds(getScreenX(), getScreenY(), getWidth(), getHeight());
	//DBG("resized")
	if(vlcNativePopupComponent->getPeer() && getPeer())
	{
		if(getPeer())getPeer()->toBehind(vlcNativePopupComponent->getPeer());
		toFront(false);
	}
#endif//BUFFER_DISPLAY
    if (titleBar != nullptr)
    {
        titleBar->setVisible (! (isFullScreen() ));
		titleBar->setBounds(0, 0, getWidth()/3, (int)tree->getItemHeight());
		titleBar->toFront(false);
    }
    if (resizableBorder != nullptr)
    {
        resizableBorder->setVisible (! (isFullScreen() ));

        resizableBorder->setBorderThickness (juce::BorderSize<int> (2));
        resizableBorder->setSize (getWidth(), getHeight());
		resizableBorder->toFront(false);
    }
	
}


////////////////////////////////////////////////////////////
//
// MEDIA PLAYER METHODS
//
////////////////////////////////////////////////////////////

void VideoComponent::forceSetVideoTime(int64_t start)
{
	//dirty but vlc would not apply set time!!
	const int stepMs = 30;
	const int maxWaitMs = 750;
	for(int i= 0;i<maxWaitMs &&(vlc->GetTime()<start);i+=stepMs)
	{
		juce::Thread::sleep(stepMs);
	}
	vlc->SetTime(start);
}
void VideoComponent::forceSetVideoTime(std::string const& name)
{
	int time = m_mediaTimes.getIntValue(name.c_str(), 0);
	if(time>0)
	{
		forceSetVideoTime(time*1000);
	}
}
void VideoComponent::appendAndPlay(std::string const& path)
{
	saveCurrentMediaTime();

	if(!vlc)
	{
		return;
	}
	std::string::size_type i = path.find_last_of("/\\");
	std::string name =  i == std::string::npos ? path : path.substr(i+1);

	int index = vlc->addPlayListItem(path);

#ifdef BUFFER_DISPLAY
	img = new juce::Image(img->rescaled(getWidth(), getHeight()));
	ptr = new juce::Image::BitmapData (*img, juce::Image::BitmapData::readWrite);
	vlc->SetBufferFormat(img->getWidth(), img->getHeight(), ptr->lineStride);
#else
    resized();
#endif

	vlc->playPlayListItem(index);
	
	forceSetVideoTime(name);

}

void VideoComponent::play()
{
	if(!vlc)
	{
		return;
	}

	vlc->play();
	
}

void VideoComponent::pause()
{
	if(!vlc)
	{
		return;
	}
	saveCurrentMediaTime();
	vlc->Pause();
}
void VideoComponent::stop()
{
	if(!vlc)
	{
		return;
	}
	saveCurrentMediaTime();
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
	if(wasResized)
	{
		resized();
	}
	else
	{
		vlcNativePopupComponent->setBounds(getScreenX(), getScreenY(), getWidth(), getHeight());
	}
}
void VideoComponent::componentVisibilityChanged(Component &  component)
{
    resized();
}

#endif

void VideoComponent::showVolumeSlider()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Audio Volume: %.f%%"),
		boost::bind<void>(&VLCWrapper::setVolume, vlc.get(), _1),
		vlc->getVolume(), 100., 1., 200., .1);
}
void VideoComponent::showPlaybackSpeedSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Speed: %.f%%"),
		boost::bind<void>(&VLCWrapper::setRate, vlc.get(), _1),
		vlc->getRate(), 100., 50., 800., .1);
}
void VideoComponent::showZoomSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Zoom: %.f%%"),
		boost::bind<void>(&VLCWrapper::setScale, vlc.get(), _1),
		vlc->getScale(), 100., 50., 500., .1);
}
void VideoComponent::showAudioOffsetSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Audio offset: %+.3fs"),
		boost::bind<void>(&VideoComponent::onMenuShiftAudio, boost::ref(*this), _1),
		vlc->getAudioDelay()/1000000., 0., -2., 2., .01, 2.);
}
void VideoComponent::showSubtitlesOffsetSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Subtitles offset: %+.3fs"),
		boost::bind<void>(&VideoComponent::onMenuShiftSubtitles, boost::ref(*this), _1),
		vlc->getSubtitleDelay()/1000000., 0., -2., 2., .01, 2.);
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
		updateSubComponentsBounds();//tree may be larger! (or not)
	}
}
void VideoComponent::onMenuListMediaFiles(MenuTreeItem& item)
{
	onMenuListFiles(item, &VideoComponent::onMenuOpen);
}
void VideoComponent::onMenuListSubtitlesFiles(MenuTreeItem& item)
{
	onMenuListFiles(item, &VideoComponent::onMenuOpenSubtitle);
}
juce::String name(juce::File const& file)
{
	juce::File p = file.getParentDirectory();
	return p.getFullPathName() == file.getFullPathName() ?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileName();
}

void VideoComponent::onMenuListFiles(MenuTreeItem& item, FileMethod fileMethod)
{
	setBrowsingFiles();

	juce::String path = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
	juce::File f(path);
	if(item.isMenuShortcut() || path.isEmpty() || !f.exists())
	{
		onMenuListFavorites(item, fileMethod);
	}
	else
	{
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
		MenuTreeItem* last =&item;
		for(int i=parentFolders.size()-1;i>=0;--i)
		{
			juce::File const& file(parentFolders[i]);
			last = last->addAction( name(file), boost::bind(fileMethod, this, _1, file), getIcon(file));
		}
		//select the last item
		if(last)
		{
			last->forceSelection();
		}


	}
}

void VideoComponent::onMenuListRootFiles(MenuTreeItem& item, FileMethod fileMethod)
{
	item.focusItemAsMenuShortcut();
	
	juce::Array<juce::File> destArray;
	juce::File::findFileSystemRoots(destArray);
	
	for(int i=0;i<destArray.size();++i)
	{
		juce::File const& file(destArray[i]);
		item.addAction( name(file), boost::bind(fileMethod, this, _1, file), getIcon(file));
	}
	
	item.addAction( TRANS("UPNP videos..."), boost::bind(&VideoComponent::onMenuListUPNPFiles, this, _1, std::vector<std::string>()), getItemImage());
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

void VideoComponent::onMenuListUPNPFiles(MenuTreeItem& item, std::vector<std::string> path)
{
	item.focusItemAsMenuShortcut();
	
	std::vector<std::pair<std::string, std::string> > list = vlc->getUPNPList(path);
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		if(std::string::npos == std::string(it->second).find("vlc://nop"))
		{
			item.addAction( it->first.c_str(), boost::bind(&VideoComponent::onMenuOpenUnconditionnal, this, _1, juce::String(it->second.c_str())), getIcon(getPathExtensionWithoutDot(it->first).c_str()));
		}
		else
		{
			std::vector<std::string> newPath(path);
			newPath.push_back(it->first);
			item.addAction( it->first.c_str(), boost::bind(&VideoComponent::onMenuListUPNPFiles, this, _1, newPath), folderImage);
		}
	}

	
}
void VideoComponent::onMenuListFavorites(MenuTreeItem& item, FileMethod fileMethod)
{
	item.focusItemAsMenuShortcut();

	mayPurgeFavorites();

	item.addAction( TRANS("All videos..."), boost::bind(&VideoComponent::onMenuListRootFiles, this, _1, fileMethod), getItemImage());

	for(int i=0;i<m_shortcuts.size();++i)
	{
		juce::File path(m_shortcuts[i]);
		juce::String driveRoot = path.getFullPathName().upToFirstOccurrenceOf(juce::File::separatorString, false, false);
		juce::String drive = path.getVolumeLabel().isEmpty() ? driveRoot : (path.getVolumeLabel()+"("+driveRoot + ")" );
		item.addAction(path.getFileName() + "-" + drive, boost::bind(fileMethod, this, _1, path));
	}
	
}

void VideoComponent::mayPurgeFavorites()
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
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
	appendAndPlay(path.toUTF8().getAddress());
}
void VideoComponent::onMenuQueue (MenuTreeItem& item, juce::String path)
{
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
	vlc->addPlayListItem(path.toUTF8().getAddress());
}
bool extensionMatch(std::set<juce::String> const& e, juce::String const& ex)
{
	juce::String ext = ex.toLowerCase();
	return e.end() != e.find(ext.startsWith(".")?ext.substring(1):ext);
}
bool extensionMatch(std::set<juce::String> const& e, juce::File const& f)
{
	return extensionMatch(e, f.getFileExtension());
}
struct FileSorter
{
	std::vector< std::set<juce::String> > priorityExtensions;
	FileSorter(std::set<juce::String> const& priorityExtensions_){priorityExtensions.push_back(priorityExtensions_);}
	FileSorter(std::vector< std::set<juce::String> > const& priorityExtensions_):priorityExtensions(priorityExtensions_) {}
	int rank(juce::File const& f)
	{
		if(f.isDirectory())
		{
			return 0;
		}
		for(std::vector< std::set<juce::String> >::const_iterator it = priorityExtensions.begin();it != priorityExtensions.end();++it)
		{
			if(extensionMatch(*it, f))
			{
				return 1+(it-priorityExtensions.begin());
			}
		}
		return priorityExtensions.size()+1;
	}
	int compareElements(juce::File const& some, juce::File const& other)
	{
		int r1 = rank(some);
		int r2 = rank(other);
		if(r1 == r2)
		{
			return some.getFileName().compareIgnoreCase(other.getFileName());
		}
		return r1 - r2;
	}
};
juce::Drawable const* VideoComponent::getIcon(juce::String const& e)
{
	if(extensionMatch(m_videoExtensions, e))
	{
		return displayImage;
	}
	if(extensionMatch(m_playlistExtensions, e))
	{
		return playlistImage;
	}
	if(extensionMatch(m_subtitlesExtensions, e))
	{
		return subtitlesImage;
	}
	return nullptr;
}
juce::Drawable const* VideoComponent::getIcon(juce::File const& f)
{
	if(f.isDirectory())
	{
		return folderImage.get();
	}
	return getIcon(f.getFileExtension());
}

void VideoComponent::onMenuOpen (MenuTreeItem& item, juce::File file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		item.focusItemAsMenuShortcut();
		
		item.addAction(TRANS("Play All"), boost::bind(&VideoComponent::onMenuOpenUnconditionnal, this, _1, 
				file.getFullPathName()), getItemImage());
		item.addAction(TRANS("Add All"), boost::bind(&VideoComponent::onMenuQueue, this, _1, 
				file.getFullPathName()), getItemImage());

		
		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		destArray.sort(FileSorter(m_suportedExtensions));
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			item.addAction( name(file), boost::bind(&VideoComponent::onMenuOpen, this, _1, file), getIcon(file));

		}

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			item.addAction(TRANS("Add to favorites"), boost::bind(&VideoComponent::onMenuAddFavorite, this, _1, 
				file.getFullPathName()), getItemImage());
		}
		else
		{
			item.addAction(TRANS("Remove from favorites"), boost::bind(&VideoComponent::onMenuRemoveFavorite, this, _1, 
				file.getFullPathName()), getItemImage());
		}
		
	}
	else
	{
		if(extensionMatch(m_subtitlesExtensions, file))
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
			vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
		}
		else
		{
			onMenuOpenUnconditionnal(item, file.getFullPathName());
		}
	}
}
void VideoComponent::restart(MenuTreeItem& item)
{
	int64_t pos = vlc->GetTime();
	vlc->Stop();
	vlc->play();
	forceSetVideoTime(pos);

}

void VideoComponent::onVLCOptionIntSelect(MenuTreeItem& item, std::string name, int v)
{
	vlc->setConfigOptionInt(name.c_str(), v);
	m_settings.setValue(name.c_str(), (int)v);

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onVLCOptionIntListMenu(MenuTreeItem& item, std::string name)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	
	std::pair<int, std::vector<std::pair<int, std::string> > > res = vlc->getConfigOptionInfoInt(name.c_str());
	for(std::vector<std::pair<int, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		item.addAction( TRANS(it->second.c_str()), boost::bind(&VideoComponent::onVLCOptionIntSelect, this, _1, name, it->first), it->first==vlc->getConfigOptionInt(name.c_str())?getItemImage():nullptr);
	}
	item.addAction( TRANS("Apply"), boost::bind(&VideoComponent::restart, this, _1), getItemImage());

}

void VideoComponent::onVLCOptionStringSelect(MenuTreeItem& item, std::string name, std::string v)
{
	vlc->setConfigOptionString(name.c_str(), v);
	m_settings.setValue(name.c_str(), juce::String(v.c_str()));

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onVLCOptionStringMenu (MenuTreeItem& item, std::string name)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	
	std::pair<std::string, std::vector<std::pair<std::string, std::string> > > res = vlc->getConfigOptionInfoString(name.c_str());
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		item.addAction( TRANS(it->second.c_str()), boost::bind(&VideoComponent::onVLCOptionStringSelect, this, _1, name, it->first), it->first==vlc->getConfigOptionString(name.c_str())?getItemImage():nullptr);
	}
	item.addAction( TRANS("Apply"), boost::bind(&VideoComponent::restart, this, _1));

}
void setVoutOptionInt(VLCWrapper * vlc, std::string option, double value)
{
	vlc->setVoutOptionInt(option.c_str(), (int)value);

}
void VideoComponent::onMenuVoutIntOption (MenuTreeItem& item, juce::String label, std::string option, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(label, 
		boost::bind<void>(&::setVoutOptionInt, vlc.get(), option, _1), value, resetValue, volumeMin, volumeMax, step, buttonsStep);
}
void VideoComponent::onMenuSubtitlePositionMode(MenuTreeItem& item, bool automatic)
{
	setBrowsingFiles(false);
	
	m_autoSubtitlesHeight = automatic;
	m_settings.setValue(SETTINGS_AUTO_SUBTITLES_HEIGHT, automatic);

	if(!m_autoSubtitlesHeight)
	{
		onMenuVoutIntOption(item,TRANS("Subtitle pos.: %+.f"), 
		std::string(CONFIG_INT_OPTION_SUBTITLE_MARGIN),
		(double)vlc->getVoutOptionInt(CONFIG_INT_OPTION_SUBTITLE_MARGIN), 0., 0., (double)getHeight(), 1., 0.);
	}
	else
	{
		if(invokeLater)invokeLater->queuef(boost::bind<void>(&VideoComponent::handleIdleTimeAndControlsVisibility, this));
	}
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));

}
void VideoComponent::onMenuSubtitlePosition(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Automatic"), boost::bind(&VideoComponent::onMenuSubtitlePositionMode, this, _1, true),m_autoSubtitlesHeight?getItemImage():nullptr);
	item.addAction( TRANS("Custom"), boost::bind(&VideoComponent::onMenuSubtitlePositionMode, this, _1, false),(!m_autoSubtitlesHeight)?getItemImage():nullptr);
	//item.addAction( TRANS("Setup"), boost::bind(&VideoComponent::onMenuVoutIntOption, this, _1,
	//	TRANS("Subtitle pos.: %+.f"), 
	//	std::string(CONFIG_INT_OPTION_SUBTITLE_MARGIN),
	//	(double)vlc->getVoutOptionInt(CONFIG_INT_OPTION_SUBTITLE_MARGIN), 0., (double)getHeight(), 1., 0.));

	
}
inline int RGB2ARGB(int rgb)
{
	return 0xFF000000 | (rgb&0xFFFFFF);
}

inline int ARGB2RGB(int argb)
{
	return (argb&0xFFFFFF);
}
void VideoComponent:: onVLCOptionColor(MenuTreeItem& item, std::string attr)
{
	setBrowsingFiles(false);

	juce::Colour init(RGB2ARGB(vlc->getConfigOptionInt(attr.c_str())));

	juce::ColourSelector colourSelector(juce::ColourSelector::showColourspace);
	colourSelector.setCurrentColour(init);
	colourSelector.setSize(getWidth() / 2, getHeight() /2);

    juce::CallOutBox callOut(colourSelector, tree->getBounds(), this);
    callOut.runModalLoop();

	int newCol = ARGB2RGB(colourSelector.getCurrentColour().getPixelARGB().getARGB());
	vlc->setConfigOptionInt(attr.c_str(), newCol);

	
	m_settings.setValue(attr.c_str(), newCol);
}


void VideoComponent::onVLCOptionIntRangeMenu(MenuTreeItem& item, std::string attr, const char* format, int min, int max, int defaultVal)
{
	setBrowsingFiles(false);
	
	int init(vlc->getConfigOptionInt(attr.c_str()));
	
	SliderWithInnerLabel slider(attr.c_str());
	slider.setRange((double)min, (double)max, 1.);
	slider.setValue(defaultVal);
	slider.setLabelFormat(format);
	slider.setSize(getWidth()/2, (int)tree->getItemHeight());
	
	juce::Rectangle<int> componentParentBounds(tree->getBounds());
	componentParentBounds.setTop(0);
	componentParentBounds.setHeight(getHeight());

    juce::CallOutBox callOut(slider, componentParentBounds, this);
    callOut.runModalLoop();
	
	vlc->setConfigOptionInt(attr.c_str(), (int)slider.getValue());
	m_settings.setValue(attr.c_str(), (int)slider.getValue());
}
#define OPT_TRANS(label) ( juce::String("*") + TRANS(label) )

void VideoComponent::onMenuSubtitleMenu(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	int cnt = vlc->getSubtitlesCount();
	int current = vlc->getCurrentSubtitleIndex();
	if(cnt)
	{
		//int commonCharacters = 0;
		//std::vector<std::string> path = vlc->getSubtitlesPath();
		//if(path.size()>2)
		//{
		//	//subs are indexed at 1 and the first is the media subtitle
		//	std::string referenceName = juce::File(path.at(1).c_str()).getFileNameWithoutExtension().toUTF8().getAddress();
		//	commonCharacters = referenceName.size();

		//	for(int i = 2;i<cnt;++i)
		//	{
		//		std::string::size_type commonCharactersForThisPair = 0;
		//		std::string currentName = juce::File(path.at(i).c_str()).getFileNameWithoutExtension().toUTF8().getAddress();

		//		std::string::const_iterator itFirst = referenceName.begin();
		//		for (std::string::const_iterator it = currentName.begin() ; 
		//			it != currentName.end() && itFirst != referenceName.end() && *it == *itFirst && commonCharactersForThisPair <= commonCharacters; 
		//			it++ ,itFirst++,commonCharactersForThisPair++);
		//		
		//		commonCharacters = commonCharactersForThisPair;
		//	}
		//}

		item.addAction( juce::String::formatted(TRANS("Disable")), boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, 0), (0==current||-1==current)?getItemImage():nullptr);
		for(int i = 1;i<cnt;++i)
		{
			item.addAction( juce::String::formatted(TRANS("Slot %d"), i), boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, i), i==current?getItemImage():nullptr);
		}
	}
	else
	{
		item.addAction( juce::String::formatted(TRANS("No subtitles")), boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, -1), -1==current?getItemImage():nullptr);
	}
	item.addAction( TRANS("Add..."), boost::bind(&VideoComponent::onMenuListSubtitlesFiles, this, _1));
	item.addAction( TRANS("Delay"), boost::bind(&VideoComponent::onMenuShiftSubtitlesSlider, this, _1));
	item.addAction( TRANS("Position"), boost::bind(&VideoComponent::onMenuSubtitlePosition, this, _1));
	item.addAction( OPT_TRANS("Size"), boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_SIZE)));
	item.addAction( OPT_TRANS("Opacity"), boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OPACITY), "Opacity: %.0f",0, 255, 255));
	item.addAction( OPT_TRANS("Color"), boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_COLOR)));
	item.addAction( OPT_TRANS("Outline"), boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_THICKNESS)));
	item.addAction( OPT_TRANS("Outline opacity"), boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_OPACITY), "Opacity: %.0f",0, 255, 255));
	item.addAction( OPT_TRANS("Outline Color"), boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_OUTLINE_COLOR)));
	item.addAction( OPT_TRANS("Background opacity"), boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_BACKGROUND_OPACITY), "Opacity: %.0f",0, 255, 0));
	item.addAction( OPT_TRANS("Background Color"), boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_BACKGROUND_COLOR)));
	item.addAction( OPT_TRANS("Shadow opacity"), boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_SHADOW_OPACITY), "Opacity: %.0f",0, 255, 0));
	item.addAction( OPT_TRANS("Shadow Color"), boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_SHADOW_COLOR)));

}
void VideoComponent::onMenuSubtitleSelect(MenuTreeItem& item, int i)
{
	setBrowsingFiles(false);
	vlc->setSubtitleIndex(i);

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onMenuOpenSubtitle (MenuTreeItem& item, juce::File file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		item.focusItemAsMenuShortcut();
						
		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false, "*");
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			item.addAction( name(file), boost::bind(&VideoComponent::onMenuOpenSubtitle, this, _1, file), folderImage.get());
		}


		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		destArray.sort(FileSorter(m_subtitlesExtensions));
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			item.addAction( name(file), boost::bind(&VideoComponent::onMenuOpenSubtitle, this, _1, file), extensionMatch(m_subtitlesExtensions, destArray[i])?subtitlesImage.get():nullptr);//getIcon(destArray[i]));//
		}
	}
	else
	{
		if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
		vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
	}
}
void VideoComponent::onMenuOpenPlaylist (MenuTreeItem& item, juce::File file)
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

	
	m_settings.setValue(SETTINGS_CROP, ratio);

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

	//item.addAction( TRANS("Auto"), boost::bind(&VideoComponent::onMenuAutoCrop, this, _1), vlc->isAutoCrop()?getItemImage():nullptr);
	std::string current = vlc->getCrop();
	std::vector<std::string> list = vlc->getCropList();
	for(std::vector<std::string>::const_iterator it = list.begin();it != list.end();++it)
	{	
		juce::String ratio(it->c_str());
		item.addAction( ratio.isEmpty()?TRANS("Original"):ratio, boost::bind(&VideoComponent::onMenuCrop, this, _1, ratio), *it==current?getItemImage():nullptr);
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
	item.addAction( "50%", boost::bind(&VideoComponent::onMenuRate, this, _1, 50.), 50==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "100%", boost::bind(&VideoComponent::onMenuRate, this, _1, 100.), 100==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "125%", boost::bind(&VideoComponent::onMenuRate, this, _1, 125.), 125==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "150%", boost::bind(&VideoComponent::onMenuRate, this, _1, 150.), 150==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "200%", boost::bind(&VideoComponent::onMenuRate, this, _1, 200.), 200==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "300%", boost::bind(&VideoComponent::onMenuRate, this, _1, 300.), 300==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "400%", boost::bind(&VideoComponent::onMenuRate, this, _1, 400.), 400==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "600%", boost::bind(&VideoComponent::onMenuRate, this, _1, 600.), 600==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "800%", boost::bind(&VideoComponent::onMenuRate, this, _1, 800.), 800==(int)(vlc->getRate())?getItemImage():nullptr);

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

void VideoComponent::onVLCAoutStringSelect(MenuTreeItem& item, std::string filter, std::string name, std::string v)
{
	vlc->setAoutFilterOptionString(name.c_str(), filter, v);
	m_settings.setValue(name.c_str(), v.c_str());

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onVLCAoutStringSelectListMenu(MenuTreeItem& item, std::string filter, std::string name)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	std::string current = vlc->getAoutFilterOptionString(name.c_str());
	item.addAction( TRANS("Disable"), boost::bind(&VideoComponent::onVLCAoutStringSelect, this, _1, filter, name, std::string("")), current.empty()?getItemImage():nullptr);

	std::pair<std::string, std::vector<std::pair<std::string, std::string> > > res = vlc->getConfigOptionInfoString(name.c_str());
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		item.addAction( TRANS(it->second.c_str()), boost::bind(&VideoComponent::onVLCAoutStringSelect, this, _1, filter, name, it->first), it->first==current?getItemImage():nullptr);
	}

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
	item.addAction( "10%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 10.), 10==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "25%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 25.), 25==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "50%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 50.), 50==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "75%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 75.), 75==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "100%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 100.), 100==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "125%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 125.), 125==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "150%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 150.), 150==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "175%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 175.), 175==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "200%", boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 200.), 200==(int)(vlc->getVolume())?getItemImage():nullptr);
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
		item.addAction(it->second.c_str(), boost::bind(&VideoComponent::onMenuAudioTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
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
	controlComponent->auxilliaryControlComponent().show(TRANS("Contrast: %+.3fs"),
		boost::bind<void>(&VLCWrapper::setVideoContrast, vlc.get(), _1),
		vlc->getVideoContrast(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoBrightness (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Brightness: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoBrightness, vlc.get(), _1),
		vlc->getVideoBrightness(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoHue (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Hue"),
		boost::bind<void>(&VLCWrapper::setVideoHue, vlc.get(), _1),
		vlc->getVideoHue(), vlc->getVideoHue(), 0, 256., .1);
}
void  VideoComponent::onMenuVideoSaturation (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Saturation: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoSaturation, vlc.get(), _1),
		vlc->getVideoSaturation(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoGamma (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Gamma: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoGamma, vlc.get(), _1),
		vlc->getVideoGamma(), 1., 0., 2., .01);
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
		item.addAction(it->second.c_str(), boost::bind(&VideoComponent::onMenuVideoTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuSoundOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Volume"), boost::bind(&VideoComponent::onMenuAudioVolumeSlider, this, _1));
	item.addAction( TRANS("Delay"), boost::bind(&VideoComponent::onMenuShiftAudioSlider, this, _1));
	item.addAction( TRANS("Equalizer"), boost::bind(&VideoComponent::onVLCAoutStringSelectListMenu, this, _1, std::string(AOUT_FILTER_EQUALIZER), std::string(CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET)));
	item.addAction( TRANS("Select Track"), boost::bind(&VideoComponent::onMenuAudioTrackList, this, _1));
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
	item.addAction( TRANS("Original"), boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("")), current==""?getItemImage():nullptr);
	item.addAction( "1:1", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("1:1")), current=="1:1"?getItemImage():nullptr);
	item.addAction( "4:3", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("4:3")), current=="4:3"?getItemImage():nullptr);
	item.addAction( "16:10", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("16:10")), current=="16:10"?getItemImage():nullptr);
	item.addAction( "16:9", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("16:9")), current=="16:9"?getItemImage():nullptr);
	item.addAction( "2.21:1", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.21:1")), current=="2.21:1"?getItemImage():nullptr);
	item.addAction( "2.35:1", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.35:1")), current=="2.35:1"?getItemImage():nullptr);
	item.addAction( "2.39:1", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.39:1")), current=="2.39:1"?getItemImage():nullptr);
	item.addAction( "5:4", boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("5:4")), current=="5:4"?getItemImage():nullptr);
	
}

void VideoComponent::onMenuVideoAdjustOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Enable"), boost::bind(&VideoComponent::onMenuVideoAdjust, this, _1), vlc->getVideoAdjust()?getItemImage():nullptr);
	item.addAction( TRANS("Contrast"), boost::bind(&VideoComponent::onMenuVideoContrast, this, _1));
	item.addAction( TRANS("Brightness"), boost::bind(&VideoComponent::onMenuVideoBrightness, this, _1));
	item.addAction( TRANS("Saturation"), boost::bind(&VideoComponent::onMenuVideoSaturation, this, _1));
	item.addAction( TRANS("Hue"), boost::bind(&VideoComponent::onMenuVideoHue, this, _1));
	item.addAction( TRANS("Gamma"), boost::bind(&VideoComponent::onMenuVideoGamma, this, _1));
}

void VideoComponent::onMenuVideoOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Speed"), boost::bind(&VideoComponent::onMenuRateSlider, this, _1));
	item.addAction( TRANS("Zoom"), boost::bind(&VideoComponent::onMenuCropList, this, _1));
	item.addAction( TRANS("Aspect Ratio"), boost::bind(&VideoComponent::onMenuRatio, this, _1));
	item.addAction( TRANS("Select Track"), boost::bind(&VideoComponent::onMenuVideoTrackList, this, _1));
	item.addAction( TRANS("Adjust"), boost::bind(&VideoComponent::onMenuVideoAdjustOptions, this, _1));
	item.addAction( OPT_TRANS("Quality"), boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_QUALITY)));
	item.addAction( OPT_TRANS("Deinterlace"), boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_DEINTERLACE)));
	item.addAction( OPT_TRANS("Deint. mode"), boost::bind(&VideoComponent::onVLCOptionStringMenu, this, _1, std::string(CONFIG_STRING_OPTION_VIDEO_DEINTERLACE_MODE)));
}
void VideoComponent::onMenuExit(MenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void VideoComponent::onPlaylistItem(MenuTreeItem& item, int index)
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

	vlc->playPlayListItem(index);

	forceSetVideoTime(name);
	
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
		item.addAction(it->c_str(), boost::bind(&VideoComponent::onPlaylistItem, this, _1, i), i==current?getItemImage():nullptr);
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
		item.addAction(it->c_str(), boost::bind(&VideoComponent::onLanguageSelect, this, _1, *it), (*it==Languages::getInstance().getCurrentLanguage())?getItemImage():nullptr);
	}
}
void VideoComponent::onSetPlayerFonSize(MenuTreeItem& item, int size)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();

	AppProportionnalComponent::setItemHeightPercentageRelativeToScreen(size);
	
	m_settings.setValue(SETTINGS_FONT_SIZE, size);
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&VideoComponent::resized, this));
}
void VideoComponent::onPlayerFonSize(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	for(int i=50;i<=175;i+=25)
	{
		item.addAction( juce::String::formatted("%d%%", i), boost::bind(&VideoComponent::onSetPlayerFonSize, this, _1, i), i==(AppProportionnalComponent::getItemHeightPercentageRelativeToScreen())?getItemImage():nullptr);
	}
}

void VideoComponent::onSetVLCOptionInt(MenuTreeItem& item, std::string name, int enable)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();

	vlc->setConfigOptionInt(name.c_str(), enable);
	
	m_settings.setValue(name.c_str(), enable);
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onSetVLCOption(MenuTreeItem& item, std::string name, bool enable)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();

	vlc->setConfigOptionBool(name.c_str(), enable);
	
	m_settings.setValue(name.c_str(), enable);
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::onPlayerOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("FullScreen"), boost::bind(&VideoComponent::onMenuFullscreen, this, _1, true), isFullScreen()?getItemImage():nullptr);
	item.addAction( TRANS("Windowed"), boost::bind(&VideoComponent::onMenuFullscreen, this, _1, false), isFullScreen()?nullptr:getItemImage());

	item.addAction( TRANS("Language"), boost::bind(&VideoComponent::onLanguageOptions, this, _1));
	item.addAction( TRANS("Menu font size"), boost::bind(&VideoComponent::onPlayerFonSize, this, _1));

	item.addAction( TRANS("Hardware"), boost::bind(&VideoComponent::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), true), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)?getItemImage():nullptr);
	item.addAction( TRANS("No hardware"), boost::bind(&VideoComponent::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), false), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)?nullptr:getItemImage());
}
void VideoComponent::onMenuRoot(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( TRANS("Open"), boost::bind(&VideoComponent::onMenuListMediaFiles, this, _1), getFolderShortcutImage());
	item.addAction( TRANS("Now playing"), boost::bind(&VideoComponent::onShowPlaylist, this, _1), getPlaylistImage());
	item.addAction( TRANS("Subtitles"), boost::bind(&VideoComponent::onMenuSubtitleMenu, this, _1), getSubtitlesImage());
	item.addAction( TRANS("Video"), boost::bind(&VideoComponent::onMenuVideoOptions, this, _1), getDisplayImage());
	item.addAction( TRANS("Sound"), boost::bind(&VideoComponent::onMenuSoundOptions, this, _1), getAudioImage());
	item.addAction( TRANS("Player"), boost::bind(&VideoComponent::onPlayerOptions, this, _1), getSettingsImage());
	item.addAction( TRANS("Exit"), boost::bind(&VideoComponent::onMenuExit, this, _1), getExitImage());

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
	if(timeFromLastMouseMove<(DISAPEAR_DELAY_MS+DISAPEAR_SPEED_MS) || !m_canHideOSD)
	{
		if(timeFromLastMouseMove<DISAPEAR_DELAY_MS || !m_canHideOSD)
		{
			setAlpha(1.f);
		}
		else
		{
			setAlpha(1.f-(float)(timeFromLastMouseMove-DISAPEAR_DELAY_MS)/(float)DISAPEAR_SPEED_MS );
		}
		//DBG ( (long)timeFromLastMouseMove  << "->" << (long)timeFromLastMouseMove-DISAPEAR_DELAY_MS << "/" << DISAPEAR_SPEED_MS << "=" << getAlpha() );
		bool showControls = vlc->isPlaying() || vlc->isPaused();
		controlComponent->setVisible(showControls);
		titleBar->setVisible(!isFullScreen() && showControls);
	}
	else
	{
		//DBG ( (long)timeFromLastMouseMove  << "->" << (long)timeFromLastMouseMove-DISAPEAR_DELAY_MS << "/" << DISAPEAR_SPEED_MS << " -> hide() " );
		setMenuTreeVisibleAndUpdateMenuButtonIcon(false);
		controlComponent->setVisible(false);
		titleBar->setVisible(false);
	}
	if(m_autoSubtitlesHeight)
	{
		vlc->setVoutOptionInt(CONFIG_INT_OPTION_SUBTITLE_MARGIN, controlComponent->isVisible()?controlComponent->getHeight():0);
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
	//DBG("vlcPopupCallback." << (rightClick?"rightClick":"leftClick") );
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();	

	//prevent menu to disappear too quickly
	m_canHideOSD = !rightClick;

	bool showMenu = rightClick || vlc->isStopped();
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, showMenu));
	if(invokeLater)invokeLater->queuef(boost::bind  (&Component::toFront,this, true));
	if(invokeLater)invokeLater->queuef(std::bind  (&VideoComponent::handleIdleTimeAndControlsVisibility,this));
	
}
void VideoComponent::vlcFullScreenControlCallback()
{
	//DBG("vlcFullScreenControlCallback");
}
void VideoComponent::vlcMouseMove(int x, int y, int button)
{
	m_canHideOSD = true;//can hide sub component while moving video
	bool controlsExpired = (juce::Time::currentTimeMillis () - lastMouseMoveMovieTime) - DISAPEAR_DELAY_MS - DISAPEAR_SPEED_MS > 0;
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();
	if(controlsExpired || vlc->isPaused())
	{
		//reactivateControls
		if(invokeLater)invokeLater->queuef(std::bind  (&VideoComponent::handleIdleTimeAndControlsVisibility,this));
	}
}
void VideoComponent::vlcMouseClick(int x, int y, int button)
{
	//DBG ( "vlcMouseClick " );

	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();
}

void VideoComponent::startedSynchronous()
{
	if(!vlcNativePopupComponent->isVisible())
	{		
		setAlpha(1.f);
		setOpaque(false);
		vlcNativePopupComponent->addToDesktop(juce::ComponentPeer::windowIsTemporary); 
		controlComponent->setVisible(true);
		vlcNativePopupComponent->setVisible(true);

		getPeer()->getComponent().removeComponentListener(this);
		getPeer()->getComponent().addComponentListener(this);

		resized();
	}
	initFromMediaDependantSettings();
}
void VideoComponent::stoppedSynchronous()
{
	
	if(vlcNativePopupComponent->isVisible())
	{
		setAlpha(1.f);
		vlcNativePopupComponent->setVisible(false);
		setMenuTreeVisibleAndUpdateMenuButtonIcon(true);
		getPeer()->getComponent().removeComponentListener(this);
	}
}

void VideoComponent::initFromMediaDependantSettings()
{
	vlc->setVolume(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));
	
	vlc->setCrop(m_settings.getValue(SETTINGS_CROP, "").toUTF8().getAddress());	
}
void VideoComponent::initBoolSetting(const char* name)
{
	vlc->setConfigOptionBool(name, m_settings.getBoolValue(name, vlc->getConfigOptionBool(name)));
}
void VideoComponent::initIntSetting(const char* name)
{
	initIntSetting(name, vlc->getConfigOptionInt(name));
}
void VideoComponent::initIntSetting(const char* name, int defaultVal)
{
	vlc->setConfigOptionInt(name, m_settings.getIntValue(name, defaultVal));
}
void VideoComponent::initStrSetting(const char* name)
{
	vlc->setConfigOptionString(name, m_settings.getValue(name, vlc->getConfigOptionString(name).c_str()).toUTF8().getAddress());
}

void VideoComponent::initFromSettings()
{
	setFullScreen(m_settings.getBoolValue(SETTINGS_FULLSCREEN, true));
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

void VideoComponent::saveCurrentMediaTime()
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
