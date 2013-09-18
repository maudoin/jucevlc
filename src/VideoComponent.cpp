
#include "VideoComponent.h"
#include "Icons.h"
#include "MenuComponent.h"
#include "Languages.h"
#include "FileSorter.h"
#include <algorithm>
#include <set>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#define DISAPEAR_DELAY_MS 500
#define DISAPEAR_SPEED_MS 500
#define MAX_SUBTITLE_ARCHIVE_SIZE 1024*1024
#define SUBTITLE_DOWNLOAD_TIMEOUT_MS 30000

#define SETTINGS_FULLSCREEN "SETTINGS_FULLSCREEN"
#define SETTINGS_VOLUME "SETTINGS_VOLUME"
#define SETTINGS_CROP "SETTINGS_CROP"
#define SETTINGS_FONT_SIZE "SETTINGS_FONT_SIZE"
#define SETTINGS_LAST_OPEN_PATH "SETTINGS_LAST_OPEN_PATH"
#define SETTINGS_LANG "SETTINGS_LANG"
#define SETTINGS_AUTO_SUBTITLES_HEIGHT "SETTINGS_AUTO_SUBTITLES_HEIGHT"
#define SHORTCUTS_FILE "shortcuts.list"
#define MAX_MEDIA_TIME_IN_SETTINGS 30



#define EXTENSIONS_VIDEO(add_) add_("3g2");add_("3gp");add_("3gp2");add_("3gpp");add_("amv");add_("asf");add_("avi");add_("bin");add_("divx");add_("drc");add_("dv");add_("f4v");add_("flv");add_("gxf");add_("iso");add_("m1v");add_("m2v");\
                         add_("m2t");add_("m2ts");add_("m4v");add_("mkv");add_("mov");add_("mp2");add_("mp2v");add_("mp4");add_("mp4v");add_("mpa");add_("mpe");add_("mpeg");add_("mpeg1");\
                         add_("mpeg2");add_("mpeg4");add_("mpg");add_("mpv2");add_("mts");add_("mtv");add_("mxf");add_("mxg");add_("nsv");add_("nuv");\
                         add_("ogg");add_("ogm");add_("ogv");add_("ogx");add_("ps");\
                         add_("rec");add_("rm");add_("rmvb");add_("tod");add_("ts");add_("tts");add_("vob");add_("vro");add_("webm");add_("wm");add_("wmv");add_("flv");

#define EXTENSIONS_PLAYLIST(add_) add_("asx");add_("b4s");add_("cue");add_("ifo");add_("m3u");add_("m3u8");add_("pls");add_("ram");add_("rar");add_("sdp");add_("vlc");add_("xspf");add_("wvx");add_("zip");add_("conf");

#define EXTENSIONS_SUBTITLE(add_) add_("cdg");add_("idx");add_("srt"); \
                            add_("sub");add_("utf");add_("ass"); \
                            add_("ssa");add_("aqt"); \
                            add_("jss");add_("psb"); \
                            add_("rt");add_("smi");add_("txt"); \
							add_("smil");add_("stl");add_("usf"); \
                            add_("dks");add_("pjs");add_("mpl2");


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

	menu = new MenuComponent ();
	menu->setItemImage(getItemImage());
	menu->asComponent()->addMouseListener(this, true);
	
    addChildComponent(controlComponent);
    addChildComponent (menu->asComponent());
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
	
	menu->addRecentMenuItem("Menu", AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuRoot, this, _1), getItemImage());
	menu->forceMenuRefresh();
 	
	////////////////
	menu->setScaleComponent(this);
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

	
	m_iconMenu.setFilter(m_videoExtensions);
	m_iconMenu.setMediaRootPath( m_settings.getValue(SETTINGS_LAST_OPEN_PATH).toUTF8().getAddress() );

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
	menu = nullptr;
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
	if(e.eventComponent == this && (!vlcNativePopupComponent->isVisible() || vlc->isStopped()) && ! menu->asComponent()->isVisible())
	{
		if(m_iconMenu.highlight((float)e.x, (float)e.y, (float)getWidth(), (float)getHeight()))
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,e.eventComponent));
		}
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
			if( (!vlcNativePopupComponent->isVisible() || vlc->isStopped()) && ! menu->asComponent()->isVisible())
			{
				bool repaint = m_iconMenu.clickOrDrag((float)e.x, (float)e.y, (float)getWidth(), (float)getHeight());

				std::string media = m_iconMenu.getMediaAt((float)e.x, (float)e.y, (float)getWidth(), (float)getHeight());
				if(!media.empty())
				{
					juce::File m(media.c_str());
					if(m.isDirectory())
					{
						m_iconMenu.setMediaRootPath(media);
						repaint = true;
					}
					else
					{
						repaint = false;
						if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::appendAndPlay,this, media));
					}
				}

				if(repaint)
				{
					//repaint on click
					if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,e.eventComponent));
				}
			}
			else
			{
				if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, vlc->isStopped()));
			}
		}
	}
}

void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
	m_canHideOSD = e.eventComponent == this;//cannot hide sub component while dragging on sub component
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();	

	
	if(e.eventComponent == this && (!vlcNativePopupComponent->isVisible() || vlc->isStopped()) && ! menu->asComponent()->isVisible())
	{
		if(m_iconMenu.clickOrDrag((float)e.x, (float)e.y, (float)getWidth(), (float)getHeight()))
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,e.eventComponent));
		}
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
	menu->asComponent()->setVisible(visible);
	controlComponent->menuButton().setImages(menu->asComponent()->isVisible()?hideFolderShortcutImage:folderShortcutImage);
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
		setMenuTreeVisibleAndUpdateMenuButtonIcon(!menu->asComponent()->isVisible());
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
	if(!vlcNativePopupComponent->isVisible() || vlc->isStopped() )
	{
		g.fillAll (juce::Colours::black);
		if(menu->asComponent()->isVisible())
		{
			g.drawImageAt(appImage, (getWidth() - appImage.getWidth())/2, (getHeight() - appImage.getHeight())/2 );

		
			juce::Font f = g.getCurrentFont().withHeight(menu->getFontHeight());
			f.setStyleFlags(juce::Font::plain);
			g.setFont(f);
			g.setColour (juce::Colours::grey);
			g.drawText(juce::String("Featuring VLC ") + vlc->getInfo().c_str(),(getWidth() - appImage.getWidth())/2,  
				(getHeight() + appImage.getHeight())/2, appImage.getWidth(), 
				(int)menu->getFontHeight(), 
				juce::Justification::centred, true);
		}
		else
		{
			m_iconMenu.paintMenu(g, appImage, (float)getWidth(), (float)getHeight());
		}
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
	
	int hMargin = (int)(menu->getItemHeight()/2.);
	int treeWidth = (browsingFiles?3:1)*w/4;
	int controlHeight = 3*(int)menu->getItemHeight();
	
    menu->asComponent()->setBounds (w-treeWidth, hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2);
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
		titleBar->setBounds(0, 0, getWidth()/3, (int)menu->getItemHeight());
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
		updateSubComponentsBounds();//menu may be larger! (or not)
	}
}
juce::String name(juce::File const& file)
{
	juce::File p = file.getParentDirectory();
	return p.getFullPathName() == file.getFullPathName() ?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileName();
}

void VideoComponent::onMenuListFiles(AbstractMenuItem& item, FileMethod fileMethod)
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
		AbstractMenuItem* last =&item;
		for(int i=parentFolders.size()-1;i>=0;--i)
		{
			juce::File const& file(parentFolders[i]);
			menu->addRecentMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY, boost::bind(fileMethod, this, _1, file), getIcon(file));
		}
		//select the last item
		menu->forceMenuRefresh();


	}
}

void VideoComponent::onMenuListRootFiles(AbstractMenuItem& item, FileMethod fileMethod)
{	
	juce::Array<juce::File> destArray;
	juce::File::findFileSystemRoots(destArray);
	
	for(int i=0;i<destArray.size();++i)
	{
		juce::File const& file(destArray[i]);
		menu->addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(fileMethod, this, _1, file), getIcon(file));
	}
	
	menu->addMenuItem( TRANS("UPNP videos..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuListUPNPFiles, this, _1, std::vector<std::string>()), getItemImage());
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
		return juce::String::empty;
	}
	return path.substring(p+1);
}

void VideoComponent::onMenuListUPNPFiles(AbstractMenuItem& item, std::vector<std::string> path)
{	
	std::vector<std::pair<std::string, std::string> > list = vlc->getUPNPList(path);
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		if(std::string::npos == std::string(it->second).find("vlc://nop"))
		{
			menu->addMenuItem( it->first.c_str(), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuOpenUnconditionnal, this, _1, juce::String(it->second.c_str())), getIcon(getPathExtensionWithoutDot(it->first).c_str()));
		}
		else
		{
			std::vector<std::string> newPath(path);
			newPath.push_back(it->first);
			menu->addMenuItem( it->first.c_str(), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuListUPNPFiles, this, _1, newPath), folderImage);
		}
	}

	
}
void VideoComponent::onMenuListFavorites(AbstractMenuItem& item, FileMethod fileMethod)
{

	mayPurgeFavorites();

	menu->addMenuItem( TRANS("All videos..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuListRootFiles, this, _1, fileMethod), getItemImage());

	for(int i=0;i<m_shortcuts.size();++i)
	{
		juce::File path(m_shortcuts[i]);
		juce::String driveRoot = path.getFullPathName().upToFirstOccurrenceOf(juce::File::separatorString, false, false);
		juce::String drive = path.getVolumeLabel().isEmpty() ? driveRoot : (path.getVolumeLabel()+"("+driveRoot + ")" );
		menu->addMenuItem(path.getFileName() + "-" + drive, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(fileMethod, this, _1, path));
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

void VideoComponent::onMenuAddFavorite(AbstractMenuItem& item, juce::String path)
{
	m_shortcuts.add(path);
	writeFavorites();
}
void VideoComponent::onMenuRemoveFavorite(AbstractMenuItem& item, juce::String path)
{
	m_shortcuts.removeString(path);
	writeFavorites();
}

void VideoComponent::onMenuOpenUnconditionnal (AbstractMenuItem& item, juce::String path)
{	
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
	appendAndPlay(path.toUTF8().getAddress());
}
void VideoComponent::onMenuQueue (AbstractMenuItem& item, juce::String path)
{
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
	vlc->addPlayListItem(path.toUTF8().getAddress());
}
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

void VideoComponent::onMenuOpenFolder (AbstractMenuItem& item, juce::File file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());
				
		menu->addMenuItem(TRANS("Play All"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuOpenUnconditionnal, this, _1, 
				file.getFullPathName()), getItemImage());
		menu->addMenuItem(TRANS("Add All"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuQueue, this, _1, 
				file.getFullPathName()), getItemImage());

		
		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false);
		destArray.sort(FileSorter(m_suportedExtensions));
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			menu->addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuOpenFolder, this, _1, file), getFolderImage());

		}
		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		destArray.sort(FileSorter(m_suportedExtensions));
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			menu->addMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuOpenFile, this, _1, file), getIcon(file));

		}

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			menu->addMenuItem(TRANS("Add to favorites"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAddFavorite, this, _1, 
				file.getFullPathName()), getItemImage());
		}
		else
		{
			menu->addMenuItem(TRANS("Remove from favorites"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRemoveFavorite, this, _1, 
				file.getFullPathName()), getItemImage());
		}
		
	}
}

void VideoComponent::onMenuOpenFile (AbstractMenuItem& item, juce::File file)
{
	if(!file.isDirectory())
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
void VideoComponent::restart(AbstractMenuItem& item)
{
	int64_t pos = vlc->GetTime();
	vlc->Stop();
	vlc->play();
	forceSetVideoTime(pos);

}

void VideoComponent::onVLCOptionIntSelect(AbstractMenuItem& item, std::string name, int v)
{
	vlc->setConfigOptionInt(name.c_str(), v);
	m_settings.setValue(name.c_str(), (int)v);
}
void VideoComponent::onVLCOptionIntListMenu(AbstractMenuItem& item, std::string name)
{
	setBrowsingFiles(false);
	
	std::pair<int, std::vector<std::pair<int, std::string> > > res = vlc->getConfigOptionInfoInt(name.c_str());
	for(std::vector<std::pair<int, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		menu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onVLCOptionIntSelect, this, _1, name, it->first), it->first==vlc->getConfigOptionInt(name.c_str())?getItemImage():nullptr);
	}
	menu->addMenuItem( TRANS("Apply"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::restart, this, _1), getItemImage());

}

void VideoComponent::onVLCOptionStringSelect(AbstractMenuItem& item, std::string name, std::string v)
{
	vlc->setConfigOptionString(name.c_str(), v);
	m_settings.setValue(name.c_str(), juce::String(v.c_str()));
}
void VideoComponent::onVLCOptionStringMenu (AbstractMenuItem& item, std::string name)
{
	setBrowsingFiles(false);
	
	std::pair<std::string, std::vector<std::pair<std::string, std::string> > > res = vlc->getConfigOptionInfoString(name.c_str());
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		menu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onVLCOptionStringSelect, this, _1, name, it->first), it->first==vlc->getConfigOptionString(name.c_str())?getItemImage():nullptr);
	}
	menu->addMenuItem( TRANS("Apply"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::restart, this, _1));

}
void setVoutOptionInt(VLCWrapper * vlc, std::string option, double value)
{
	vlc->setVoutOptionInt(option.c_str(), (int)value);

}
void VideoComponent::onMenuVoutIntOption (AbstractMenuItem& item, juce::String label, std::string option, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(label, 
		boost::bind<void>(&::setVoutOptionInt, vlc.get(), option, _1), value, resetValue, volumeMin, volumeMax, step, buttonsStep);
}
void VideoComponent::onMenuSubtitlePositionMode(AbstractMenuItem& item, bool automatic)
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

}
void VideoComponent::onMenuSubtitlePosition(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("Automatic"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSubtitlePositionMode, this, _1, true),m_autoSubtitlesHeight?getItemImage():nullptr);
	menu->addMenuItem( TRANS("Custom"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSubtitlePositionMode, this, _1, false),(!m_autoSubtitlesHeight)?getItemImage():nullptr);
	//menu->addMenuItem( TRANS("Setup"), boost::bind(&VideoComponent::onMenuVoutIntOption, this, _1,
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
void VideoComponent:: onVLCOptionColor(AbstractMenuItem& item, std::string attr)
{
	setBrowsingFiles(false);

	juce::Colour init(RGB2ARGB(vlc->getConfigOptionInt(attr.c_str())));

	juce::ColourSelector colourSelector(juce::ColourSelector::showColourspace);
	colourSelector.setCurrentColour(init);
	colourSelector.setSize(getWidth() / 2, getHeight() /2);

    juce::CallOutBox callOut(colourSelector, menu->asComponent()->getBounds(), this);
    callOut.runModalLoop();

	int newCol = ARGB2RGB(colourSelector.getCurrentColour().getPixelARGB().getARGB());
	vlc->setConfigOptionInt(attr.c_str(), newCol);

	
	m_settings.setValue(attr.c_str(), newCol);
}


void VideoComponent::onVLCOptionIntRangeMenu(AbstractMenuItem& item, std::string attr, const char* format, int min, int max, int defaultVal)
{
	setBrowsingFiles(false);
	
	int init(vlc->getConfigOptionInt(attr.c_str()));
	
	SliderWithInnerLabel slider(attr.c_str());
	slider.setRange((double)min, (double)max, 1.);
	slider.setValue(defaultVal);
	slider.setLabelFormat(format);
	slider.setSize(getWidth()/2, (int)menu->getItemHeight());
	
	juce::Rectangle<int> componentParentBounds(menu->asComponent()->getBounds());
	componentParentBounds.setTop(0);
	componentParentBounds.setHeight(getHeight());

    juce::CallOutBox callOut(slider, componentParentBounds, this);
    callOut.runModalLoop();
	
	vlc->setConfigOptionInt(attr.c_str(), (int)slider.getValue());
	m_settings.setValue(attr.c_str(), (int)slider.getValue());
}
#define OPT_TRANS(label) ( juce::String("*") + TRANS(label) )

void VideoComponent::onMenuSubtitleMenu(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

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

		menu->addMenuItem( juce::String::formatted(TRANS("Disable")), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, 0), (0==current||-1==current)?getItemImage():nullptr);
		for(int i = 1;i<cnt;++i)
		{
			menu->addMenuItem( juce::String::formatted(TRANS("Slot %d"), i), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, i), i==current?getItemImage():nullptr);
		}
	}
	else
	{
		menu->addMenuItem( juce::String::formatted(TRANS("No subtitles")), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, -1), -1==current?getItemImage():nullptr);
	}
	menu->addMenuItem( TRANS("Add..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuListFiles, this, _1, &VideoComponent::onMenuOpenSubtitleFolder));
	menu->addMenuItem( TRANS("opensubtitles.org"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchOpenSubtitles, this, _1));
	menu->addMenuItem( TRANS("SubtitleSeeker.com"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitleSeeker, this, _1));
	menu->addMenuItem( TRANS("Delay"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuShiftSubtitlesSlider, this, _1));
	menu->addMenuItem( TRANS("Position"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSubtitlePosition, this, _1));
	menu->addMenuItem( OPT_TRANS("Size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_SIZE)));
	menu->addMenuItem( OPT_TRANS("Opacity"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OPACITY), "Opacity: %.0f",0, 255, 255));
	menu->addMenuItem( OPT_TRANS("Color"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_COLOR)));
	menu->addMenuItem( OPT_TRANS("Outline"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_THICKNESS)));
	menu->addMenuItem( OPT_TRANS("Outline opacity"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_OPACITY), "Opacity: %.0f",0, 255, 255));
	menu->addMenuItem( OPT_TRANS("Outline Color"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_OUTLINE_COLOR)));
	menu->addMenuItem( OPT_TRANS("Background opacity"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_BACKGROUND_OPACITY), "Opacity: %.0f",0, 255, 0));
	menu->addMenuItem( OPT_TRANS("Background Color"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_BACKGROUND_COLOR)));
	menu->addMenuItem( OPT_TRANS("Shadow opacity"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_SHADOW_OPACITY), "Opacity: %.0f",0, 255, 0));
	menu->addMenuItem( OPT_TRANS("Shadow Color"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_SHADOW_COLOR)));

}
void VideoComponent::onMenuSearchOpenSubtitles(AbstractMenuItem& item)
{
	onMenuSearchOpenSubtitlesSelectLanguage(item, vlc->getCurrentPlayListItem().c_str());
}
void VideoComponent::onMenuSearchSubtitleSeeker(AbstractMenuItem& item)
{
	onMenuSearchSubtitleSeeker(item, vlc->getCurrentPlayListItem().c_str());
}
void VideoComponent::onMenuSearchSubtitlesManually(AbstractMenuItem& item, juce::String lang)
{
	juce::TextEditor editor("Subtitle search");
	
	editor.setText(vlc->getCurrentPlayListItem().c_str());
	editor.setLookAndFeel(&getLookAndFeel());
	editor.setSize(3*(int)getWidth() / 4, 2*(int)menu->getFontHeight());

    juce::CallOutBox callOut(editor, menu->asComponent()->getBounds(), this);

	//relayout before callout window
	menu->forceMenuRefresh();

	callOut.runModalLoop();

	onMenuSearchOpenSubtitles(item, lang, editor.getText());
}

#define OPEN_SUBTITLES_LANGUAGUES(add) \
add("all","en","All");\
add("eng","en","English");\
add("alb","sq","Albanian");\
add("ara","ar","Arabic");\
add("arm","hy","Armenian");\
add("baq","eu","Basque");\
add("ben","bn","Bengali");\
add("bos","bs","Bosnian");\
add("pob","pb","Portuguese-BR");\
add("bre","br","Breton");\
add("bul","bg","Bulgarian");\
add("bur","my","Burmese");\
add("cat","ca","Catalan");\
add("chi","zh","Chinese");\
add("hrv","hr","Croatian");\
add("cze","cs","Czech");\
add("dan","da","Danish");\
add("dut","nl","Dutch");\
add("eng","en","English");\
add("epo","eo","Esperanto");\
add("est","et","Estonian");\
add("fin","fi","Finnish");\
add("fre","fr","French");\
add("glg","gl","Galician");\
add("geo","ka","Georgian");\
add("ger","de","German");\
add("ell","el","Greek");\
add("heb","he","Hebrew");\
add("hin","hi","Hindi");\
add("hun","hu","Hungarian");\
add("ice","is","Icelandic");\
add("ind","id","Indonesian");\
add("ita","it","Italian");\
add("jpn","ja","Japanese");\
add("kaz","kk","Kazakh");\
add("khm","km","Khmer");\
add("kor","ko","Korean");\
add("lav","lv","Latvian");\
add("lit","lt","Lithuanian");\
add("ltz","lb","Luxembourgish");\
add("mac","mk","Macedonian");\
add("may","ms","Malay");\
add("mal","ml","Malayalam");\
add("mon","mn","Mongolian");\
add("nor","no","Norwegian");\
add("oci","oc","Occitan");\
add("per","fa","Farsi");\
add("pol","pl","Polish");\
add("por","pt","Portuguese");\
add("rum","ro","Romanian");\
add("rus","ru","Russian");\
add("scc","sr","Serbian");\
add("sin","si","Sinhalese");\
add("slo","sk","Slovak");\
add("slv","sl","Slovenian");\
add("spa","es","Spanish");\
add("swa","sw","Swahili");\
add("swe","sv","Swedish");\
add("syr","","Syriac");\
add("tgl","tl","Tagalog");\
add("tam","ta","Tamil");\
add("tel","te","Telugu");\
add("tha","th","Thai");\
add("tur","tr","Turkish");\
add("ukr","uk","Ukrainian");\
add("urd","ur","Urdu");\
add("vie","vi","Vietnamese");


#define addItem(shortName, ui, label) menu->addMenuItem( label, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchOpenSubtitles, this, _1 , shortName, movieName))

void VideoComponent::onMenuSearchOpenSubtitlesSelectLanguage(AbstractMenuItem& item, juce::String movieName)
{
	OPEN_SUBTITLES_LANGUAGUES(addItem);
}
void VideoComponent::onMenuSearchOpenSubtitles(AbstractMenuItem& item, juce::String lang, juce::String movieName)
{
	setBrowsingFiles(false);
	movieName = movieName.replace("%", "%37");
	movieName = movieName.replace(" ", "%20");
	movieName = movieName.replace("_", "%20");
	movieName = movieName.replace(".", "%20");
	std::string language=std::string(lang.toUTF8().getAddress());
	std::string name = str( boost::format("http://www.opensubtitles.org/en/search/sublanguageid-%s/moviename-%s/simplexml")%language%std::string(movieName.toUTF8().getAddress()) );
	juce::URL url(name.c_str());
	

	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		menu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchOpenSubtitles, this, _1 , lang, movieName));
		return;
	}
	juce::ScopedPointer<juce::XmlElement> e(juce::XmlDocument::parse(pIStream->readEntireStreamAsString()));
	if(e.get())
	{
		juce::XmlElement* results(e->getChildByName("results"));
		if(results)
		{
			juce::XmlElement* sub = results->getFirstChildElement();
			if(sub)
			{
				setBrowsingFiles(true);
				do
				{
					juce::String name = sub->getChildElementAllSubText("releasename", juce::String::empty);
					juce::String downloadURL = sub->getChildElementAllSubText("download", juce::String::empty);
					juce::String language = sub->getChildElementAllSubText("language", juce::String::empty);
					juce::String user = sub->getChildElementAllSubText("user", juce::String::empty);
					if(!name.isEmpty() && !downloadURL.isEmpty())
					{
						menu->addMenuItem(name + juce::String(" by ") + user + juce::String(" (") + language + juce::String(")"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuDowloadOpenSubtitle, this, _1, downloadURL));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}

	
	//menu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	menu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchOpenSubtitles, this, _1 , lang, movieName));
}

void VideoComponent::onMenuSearchSubtitleSeeker(AbstractMenuItem& item, juce::String movieName)
{
	setBrowsingFiles(false);
	movieName = movieName.replace("%", "%37");
	movieName = movieName.replace(" ", "+");
	movieName = movieName.replace("_", "+");
	movieName = movieName.replace(".", "+");
	std::string name = str( boost::format("http://api.subtitleseeker.com/search/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&q=%s")%std::string(movieName.toUTF8().getAddress()) );
	juce::URL url(name.c_str());
	

	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		menu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeeker, this, _1 ,  movieName));
		return;
	}
	juce::ScopedPointer<juce::XmlElement> e(juce::XmlDocument::parse(pIStream->readEntireStreamAsString()));
	if(e.get() && e->getTagName() == "results")
	{
		juce::XmlElement* items(e->getChildByName("items"));
		if(items)
		{
			juce::XmlElement* sub(items->getChildByName("item"));
			if(sub)
			{
				setBrowsingFiles(true);
				do
				{
					juce::String name = sub->getChildElementAllSubText("title", juce::String::empty);
					juce::String year = sub->getChildElementAllSubText("year", juce::String::empty);
					juce::String imdb = sub->getChildElementAllSubText("imdb", juce::String::empty);
					if(!name.isEmpty())
					{
						menu->addMenuItem(name + juce::String(" (") + year + juce::String(")"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1, imdb));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}

	
	//menu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	menu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeeker, this, _1 ,  movieName));
}

void VideoComponent::onMenuSearchSubtitleSeekerImdb(AbstractMenuItem& item, juce::String imdb)
{
	setBrowsingFiles(false);
	std::string name = str( boost::format("http://api.subtitleseeker.com/get/title_languages/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&imdb=%s")%imdb);
	juce::URL url(name.c_str());
	

	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		menu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1 ,  imdb));
		return;
	}
	juce::ScopedPointer<juce::XmlElement> e(juce::XmlDocument::parse(pIStream->readEntireStreamAsString()));
	if(e.get() && e->getTagName() == "results")
	{
		juce::XmlElement* items(e->getChildByName("items"));
		if(items)
		{
			juce::XmlElement* sub(items->getChildByName("item"));
			if(sub)
			{
				setBrowsingFiles(true);
				do
				{
					juce::String lang = sub->getChildElementAllSubText("lang", juce::String::empty);
					if(!lang.isEmpty())
					{
						menu->addMenuItem(lang, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1, imdb ,lang));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}

	
	//menu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	menu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1 , imdb));
}
void VideoComponent::onMenuSearchSubtitleSeekerImdbLang(AbstractMenuItem& item, juce::String imdb, juce::String lang)
{
	setBrowsingFiles(false);
	std::string name = str( boost::format("http://api.subtitleseeker.com/get/title_subtitles/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&imdb=%s&language=%s")%imdb%lang);
	juce::URL url(name.c_str());
	

	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		menu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1 ,  imdb, lang));
		return;
	}
	juce::ScopedPointer<juce::XmlElement> e(juce::XmlDocument::parse(pIStream->readEntireStreamAsString()));
	if(e.get() && e->getTagName() == "results")
	{
		juce::XmlElement* items(e->getChildByName("items"));
		if(items)
		{
			juce::XmlElement* sub(items->getChildByName("item"));
			if(sub)
			{
				setBrowsingFiles(true);
				do
				{
					juce::String release = sub->getChildElementAllSubText("release", juce::String::empty);
					juce::String site = sub->getChildElementAllSubText("site", juce::String::empty);
					juce::String url = sub->getChildElementAllSubText("url", juce::String::empty);
					if(!lang.isEmpty())
					{
						menu->addMenuItem(site + ": " + release, AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuDowloadSubtitleSeeker, this, _1, url, site));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}

	
	//menu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	menu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1 ,  imdb, lang));
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
};/*
#define PREV__MSVC_RUNTIME_CHECKS __MSVC_RUNTIME_CHECKS
#undef __MSVC_RUNTIME_CHECKS
#define PREV_DLL _DLL
#undef _DLL
#define PREV_DEBUG _DEBUG
#undef _DEBUG*/
#include <boost/regex.hpp> /*
#define _DLL PREV_DLL 
#define _DEBUG PREV_DEBUG
#define __MSVC_RUNTIME_CHECKS PREV__MSVC_RUNTIME_CHECKS*/

void VideoComponent::onMenuDowloadSubtitleSeeker(AbstractMenuItem& item, juce::String downloadUrl, juce::String site)
{
	juce::URL url(downloadUrl);
	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", SUBTITLE_DOWNLOAD_TIMEOUT_MS));
	juce::String fileName = downloadUrl.fromLastOccurrenceOf("/", false, false);
	if(pIStream.get())
	{
		juce::MemoryOutputStream memStream(10000);//10ko at least
		if(memStream.writeFromInputStream(*pIStream, MAX_SUBTITLE_ARCHIVE_SIZE)>0)
		{
			
			
			std::string ex("href=\"([^\"]*");
			ex += site.toUTF8().getAddress();
			ex += "[^\"]*)";
			boost::regex expression(ex, boost::regex::icase); 

			memStream.writeByte(0);
		   boost::cmatch matches; 
		   if(boost::regex_search((char*)memStream.getData(), matches, expression)) 
		   {
				juce::Process::openDocument(matches[1].str().c_str(),juce::String::empty);
/*
				juce::URL other(matches[1].str().c_str());
				juce::ScopedPointer<juce::InputStream> pIStreamOther(other.createInputStream(false, 0, 0, "", SUBTITLE_DOWNLOAD_TIMEOUT_MS));
				if(pIStreamOther.get())
				{	
					memStream.reset();
					memStream.writeFromInputStream(*pIStreamOther, MAX_SUBTITLE_ARCHIVE_SIZE);
				}
				else
				{
					menu->addMenuItem(TRANS(">")+matches[1].str().c_str(), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuDowloadSubtitleSeeker, this, _1, downloadUrl, site));

					return;
				}
			*/		
		   }
		   else
		   {
				juce::Process::openDocument(downloadUrl,juce::String::empty);
			   /*
			    //get html
			    juce::String outPath = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
				juce::File out(outPath);
				out = out.getChildFile(fileName+".html");
				juce::FileOutputStream outStream(out);
				if(outStream.openedOk())
				{
					if(outStream.write(memStream.getData(), memStream.getDataSize())>0)
					{
						onMenuOpenSubtitleFile(item,out);
					}
				}*/
		   }

		}
	}
}
void VideoComponent::onMenuDowloadOpenSubtitle(AbstractMenuItem& item, juce::String downloadUrl)
{
	juce::URL url(downloadUrl);


	juce::StringPairArray response;
	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", SUBTITLE_DOWNLOAD_TIMEOUT_MS, &response));
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
						onMenuOpenSubtitleFile(item,out);
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
			EXTENSIONS_SUBTITLE(subExtensions.insert);
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

			//output entry
			juce::File out(outPath);
			
			const juce::ZipFile::ZipEntry* supposedSubtitle = entries.getFirst();
			zip.uncompressEntry(zip.getIndexOfFileName(supposedSubtitle->filename), out, false);
			
			onMenuOpenSubtitleFile(item,out.getChildFile(supposedSubtitle->filename));
		}
	}
}
void VideoComponent::onMenuSubtitleSelect(AbstractMenuItem& item, int i)
{
	setBrowsingFiles(false);
	vlc->setSubtitleIndex(i);
}
void VideoComponent::onMenuOpenSubtitleFolder (AbstractMenuItem& item, juce::File file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());
								
		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false, "*");
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			menu->addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuOpenSubtitleFolder, this, _1, file), folderImage.get());
		}


		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		destArray.sort(FileSorter(m_subtitlesExtensions));
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			menu->addMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuOpenSubtitleFile, this, _1, file), extensionMatch(m_subtitlesExtensions, destArray[i])?subtitlesImage.get():nullptr);//getIcon(destArray[i]));//
		}
	}
}
void VideoComponent::onMenuOpenSubtitleFile (AbstractMenuItem& item, juce::File file)
{
	if(!file.isDirectory())
	{
		if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
		vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
	}
}
void VideoComponent::onMenuOpenPlaylist (AbstractMenuItem& item, juce::File file)
{
}

void VideoComponent::onMenuZoom(AbstractMenuItem& item, double ratio)
{
	setBrowsingFiles(false);
	vlc->setScale(ratio);

	showZoomSlider();
}
void VideoComponent::onMenuCrop (AbstractMenuItem& item, juce::String ratio)
{
	setBrowsingFiles(false);

	vlc->setAutoCrop(false);
	vlc->setCrop(std::string(ratio.getCharPointer().getAddress()));
	
	m_settings.setValue(SETTINGS_CROP, ratio);
}
void VideoComponent::onMenuAutoCrop (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	vlc->setAutoCrop(true);
}
void VideoComponent::onMenuCropList (AbstractMenuItem& item)
{
	setBrowsingFiles(false);	

	//menu->addMenuItem( TRANS("Auto"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAutoCrop, this, _1), vlc->isAutoCrop()?getItemImage():nullptr);
	std::string current = vlc->getCrop();
	std::vector<std::string> list = vlc->getCropList();
	for(std::vector<std::string>::const_iterator it = list.begin();it != list.end();++it)
	{	
		juce::String ratio(it->c_str());
		menu->addMenuItem( ratio.isEmpty()?TRANS("Original"):ratio, AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuCrop, this, _1, ratio), *it==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuRate (AbstractMenuItem& item, double rate)
{
	setBrowsingFiles(false);
	vlc->setRate(rate);

	showPlaybackSpeedSlider();
}
void VideoComponent::onMenuRateListAndSlider (AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	showPlaybackSpeedSlider();
	
	menu->addMenuItem( "50%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 50.), 50==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "100%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 100.), 100==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "125%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 125.), 125==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "150%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 150.), 150==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "200%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 200.), 200==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "300%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 300.), 300==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "400%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 400.), 400==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "600%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 600.), 600==(int)(vlc->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "800%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 800.), 800==(int)(vlc->getRate())?getItemImage():nullptr);

}
void VideoComponent::onMenuShiftAudio(double s)
{
	setBrowsingFiles(false);
	vlc->setAudioDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftAudioSlider(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	showAudioOffsetSlider();
}
void VideoComponent::onMenuShiftSubtitles(double s)
{
	setBrowsingFiles(false);
	vlc->setSubtitleDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftSubtitlesSlider(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	showSubtitlesOffsetSlider();
}

void VideoComponent::onVLCAoutStringSelect(AbstractMenuItem& item, std::string filter, std::string name, std::string v)
{
	vlc->setAoutFilterOptionString(name.c_str(), filter, v);
	m_settings.setValue(name.c_str(), v.c_str());
}
void VideoComponent::onVLCAoutStringSelectListMenu(AbstractMenuItem& item, std::string filter, std::string name)
{
	setBrowsingFiles(false);

	std::string current = vlc->getAoutFilterOptionString(name.c_str());
	menu->addMenuItem( TRANS("Disable"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onVLCAoutStringSelect, this, _1, filter, name, std::string("")), current.empty()?getItemImage():nullptr);

	std::pair<std::string, std::vector<std::pair<std::string, std::string> > > res = vlc->getConfigOptionInfoString(name.c_str());
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		menu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onVLCAoutStringSelect, this, _1, filter, name, it->first), it->first==current?getItemImage():nullptr);
	}

}
void VideoComponent::onMenuAudioVolume(AbstractMenuItem& item, double volume)
{
	setBrowsingFiles(false);
	vlc->setVolume(volume);

	showVolumeSlider();

	m_settings.setValue(SETTINGS_VOLUME, vlc->getVolume());
}

void VideoComponent::onMenuAudioVolumeListAndSlider(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	showVolumeSlider();
	
	menu->addMenuItem( "10%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 10.), 10==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "25%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 25.), 25==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "50%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 50.), 50==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "75%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 75.), 75==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "100%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 100.), 100==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "125%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 125.), 125==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "150%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 150.), 150==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "175%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 175.), 175==(int)(vlc->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "200%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 200.), 200==(int)(vlc->getVolume())?getItemImage():nullptr);
}

void VideoComponent::onMenuFullscreen(AbstractMenuItem& item, bool fs)
{
	setBrowsingFiles(false);
	setFullScreen(fs);
}

void VideoComponent::onMenuAudioTrack (AbstractMenuItem& item, int id)
{
	setBrowsingFiles(false);
	vlc->setAudioTrack(id);
}
void VideoComponent::onMenuAudioTrackList (AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	int current = vlc->getAudioTrack();
	std::vector<std::pair<int, std::string> > list = vlc->getAudioTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{	
		menu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuVideoAdjust (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	vlc->setVideoAdjust(!vlc->getVideoAdjust());
}
void VideoComponent::onMenuVideoContrast (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Contrast: %+.3fs"),
		boost::bind<void>(&VLCWrapper::setVideoContrast, vlc.get(), _1),
		vlc->getVideoContrast(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoBrightness (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Brightness: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoBrightness, vlc.get(), _1),
		vlc->getVideoBrightness(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoHue (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Hue"),
		boost::bind<void>(&VLCWrapper::setVideoHue, vlc.get(), _1),
		vlc->getVideoHue(), vlc->getVideoHue(), 0, 256., .1);
}
void  VideoComponent::onMenuVideoSaturation (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Saturation: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoSaturation, vlc.get(), _1),
		vlc->getVideoSaturation(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoGamma (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Gamma: %+.3f"),
		boost::bind<void>(&VLCWrapper::setVideoGamma, vlc.get(), _1),
		vlc->getVideoGamma(), 1., 0., 2., .01);
}

void VideoComponent::onMenuVideoTrack (AbstractMenuItem& item, int id)
{
	setBrowsingFiles(false);
	vlc->setVideoTrack(id);
}
void VideoComponent::onMenuVideoTrackList (AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	int current = vlc->getVideoTrack();
	std::vector<std::pair<int, std::string> > list = vlc->getVideoTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{	
		menu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuVideoTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuSoundOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("Volume"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuAudioVolumeListAndSlider, this, _1));
	menu->addMenuItem( TRANS("Delay"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuShiftAudioSlider, this, _1));
	menu->addMenuItem( TRANS("Equalizer"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCAoutStringSelectListMenu, this, _1, std::string(AOUT_FILTER_EQUALIZER), std::string(CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET)));
	menu->addMenuItem( TRANS("Select Track"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuAudioTrackList, this, _1));
}

void VideoComponent::onMenuSetAspectRatio(AbstractMenuItem& item, juce::String ratio)
{
	setBrowsingFiles(false);
	vlc->setAspect(ratio.getCharPointer().getAddress());
}
void VideoComponent::onMenuRatio(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	std::string current = vlc->getAspect();

	menu->addMenuItem( TRANS("Original"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("")), current==""?getItemImage():nullptr);
	menu->addMenuItem( "1:1", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("1:1")), current=="1:1"?getItemImage():nullptr);
	menu->addMenuItem( "4:3", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("4:3")), current=="4:3"?getItemImage():nullptr);
	menu->addMenuItem( "16:10", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("16:10")), current=="16:10"?getItemImage():nullptr);
	menu->addMenuItem( "16:9", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("16:9")), current=="16:9"?getItemImage():nullptr);
	menu->addMenuItem( "2.21:1", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.21:1")), current=="2.21:1"?getItemImage():nullptr);
	menu->addMenuItem( "2.35:1", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.35:1")), current=="2.35:1"?getItemImage():nullptr);
	menu->addMenuItem( "2.39:1", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.39:1")), current=="2.39:1"?getItemImage():nullptr);
	menu->addMenuItem( "5:4", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("5:4")), current=="5:4"?getItemImage():nullptr);
	
}

void VideoComponent::onMenuVideoAdjustOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("Enable"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuVideoAdjust, this, _1), vlc->getVideoAdjust()?getItemImage():nullptr);
	menu->addMenuItem( TRANS("Contrast"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuVideoContrast, this, _1));
	menu->addMenuItem( TRANS("Brightness"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuVideoBrightness, this, _1));
	menu->addMenuItem( TRANS("Saturation"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuVideoSaturation, this, _1));
	menu->addMenuItem( TRANS("Hue"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuVideoHue, this, _1));
	menu->addMenuItem( TRANS("Gamma"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuVideoGamma, this, _1));
}

void VideoComponent::onMenuVideoOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("Speed"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuRateListAndSlider, this, _1));
	menu->addMenuItem( TRANS("Zoom"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuCropList, this, _1));
	menu->addMenuItem( TRANS("Aspect Ratio"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuRatio, this, _1));
	menu->addMenuItem( TRANS("Select Track"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuVideoTrackList, this, _1));
	menu->addMenuItem( TRANS("Adjust"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuVideoAdjustOptions, this, _1));
	menu->addMenuItem( OPT_TRANS("Quality"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_QUALITY)));
	menu->addMenuItem( OPT_TRANS("Deinterlace"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_DEINTERLACE)));
	menu->addMenuItem( OPT_TRANS("Deint. mode"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCOptionStringMenu, this, _1, std::string(CONFIG_STRING_OPTION_VIDEO_DEINTERLACE_MODE)));
}
void VideoComponent::onMenuExit(AbstractMenuItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void VideoComponent::onPlaylistItem(AbstractMenuItem& item, int index)
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
}
void VideoComponent::onShowPlaylist(AbstractMenuItem& item)
{
	setBrowsingFiles(true);

	int current = vlc->getCurrentPlayListItemIndex ();
	std::vector<std::string > list = vlc->getCurrentPlayList();
	int i=0;
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{	
		menu->addMenuItem(it->c_str(), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onPlaylistItem, this, _1, i), i==current?getItemImage():nullptr);
		++i;
	}
	
}
void VideoComponent::onLanguageSelect(AbstractMenuItem& item, std::string lang)
{
	setBrowsingFiles(false);
	Languages::getInstance().setCurrentLanguage(lang);
	
	m_settings.setValue(SETTINGS_LANG, lang.c_str());
}
void VideoComponent::onLanguageOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	std::vector< std::string > list = Languages::getInstance().getLanguages();
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{	
		menu->addMenuItem(it->c_str(), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onLanguageSelect, this, _1, *it), (*it==Languages::getInstance().getCurrentLanguage())?getItemImage():nullptr);
	}
}
void VideoComponent::onSetPlayerFonSize(AbstractMenuItem& item, int size)
{
	setBrowsingFiles(false);

	AppProportionnalComponent::setItemHeightPercentageRelativeToScreen(size);
	
	m_settings.setValue(SETTINGS_FONT_SIZE, size);
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&VideoComponent::resized, this));
}
void VideoComponent::onPlayerFonSize(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	for(int i=50;i<=175;i+=25)
	{
		menu->addMenuItem( juce::String::formatted("%d%%", i), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onSetPlayerFonSize, this, _1, i), i==(AppProportionnalComponent::getItemHeightPercentageRelativeToScreen())?getItemImage():nullptr);
	}
}

void VideoComponent::onSetVLCOptionInt(AbstractMenuItem& item, std::string name, int enable)
{
	setBrowsingFiles(false);

	vlc->setConfigOptionInt(name.c_str(), enable);
	
	m_settings.setValue(name.c_str(), enable);
}
void VideoComponent::onSetVLCOption(AbstractMenuItem& item, std::string name, bool enable)
{
	setBrowsingFiles(false);

	vlc->setConfigOptionBool(name.c_str(), enable);
	
	m_settings.setValue(name.c_str(), enable);
}
void VideoComponent::onPlayerOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("FullScreen"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuFullscreen, this, _1, true), isFullScreen()?getItemImage():nullptr);
	menu->addMenuItem( TRANS("Windowed"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuFullscreen, this, _1, false), isFullScreen()?nullptr:getItemImage());

	menu->addMenuItem( TRANS("Language"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onLanguageOptions, this, _1));
	menu->addMenuItem( TRANS("Menu font size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onPlayerFonSize, this, _1));

	menu->addMenuItem( TRANS("Hardware"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), true), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)?getItemImage():nullptr);
	menu->addMenuItem( TRANS("No hardware"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), false), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)?nullptr:getItemImage());
}
void VideoComponent::onMenuRoot(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("Open"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuListFiles, this, _1, &VideoComponent::onMenuOpenFolder), getFolderShortcutImage());
	menu->addMenuItem( TRANS("Now playing"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onShowPlaylist, this, _1), getPlaylistImage());
	menu->addMenuItem( TRANS("Subtitles"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSubtitleMenu, this, _1), getSubtitlesImage());
	menu->addMenuItem( TRANS("Video"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuVideoOptions, this, _1), getDisplayImage());
	menu->addMenuItem( TRANS("Sound"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSoundOptions, this, _1), getAudioImage());
	menu->addMenuItem( TRANS("Player"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onPlayerOptions, this, _1), getSettingsImage());
	menu->addMenuItem( TRANS("Exit"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuExit, this, _1), getExitImage());

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
