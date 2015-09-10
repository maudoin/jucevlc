
#include "VideoComponent.h"
#include "HueSelector.h"
#include "Icons.h"
#include "MenuComponent.h"
#include "Languages.h"
#include "FileSorter.h"
#include "RegularExpression.h"
#include <algorithm>
#include <set>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "InvokeLater.h"

#define DISAPEAR_DELAY_MS 500
#define DISAPEAR_SPEED_MS 500
#define MAX_SUBTITLE_ARCHIVE_SIZE 1024*1024
#define SUBTITLE_DOWNLOAD_TIMEOUT_MS 30000

#define SETTINGS_FULLSCREEN "SETTINGS_FULLSCREEN"
#define SETTINGS_VOLUME "SETTINGS_VOLUME"
#define SETTINGS_CROP "SETTINGS_CROP"
#define SETTINGS_FONT_SIZE "SETTINGS_FONT_SIZE"
#define SETTINGS_LAST_OPEN_PATH "SETTINGS_LAST_OPEN_PATH"
#define SETTINGS_POSTER_BROWSER_ROOT_PATH "SETTINGS_POSTER_BROWSER_ROOT_PATH"
#define SETTINGS_LANG "SETTINGS_LANG"
#define SETTINGS_AUTO_SUBTITLES_HEIGHT "SETTINGS_AUTO_SUBTITLES_HEIGHT"
#define SETTINGS_AUDIO_DEVICE "SETTINGS_AUDIO_DEVICE"
#define SETTINGS_AUDIO_OUTPUT "SETTINGS_AUDIO_OUTPUT"
#define SHORTCUTS_FILE "shortcuts.list"
#define MAX_MEDIA_TIME_IN_SETTINGS 30



#define EXTENSIONS_VIDEO(add_) add_("3g2");add_("3gp");add_("3gp2");add_("3gpp");add_("amv");add_("asf");add_("avi");add_("bin");add_("divx");add_("drc");add_("dv");add_("f4v");add_("flv");add_("gxf");add_("iso");add_("m1v");add_("m2v");\
                         add_("m2t");add_("m2ts");add_("m4v");add_("mkv");add_("mov");add_("mp2");add_("mp2v");add_("mp4");add_("mp4v");add_("mpa");add_("mpe");add_("mpeg");add_("mpeg1");\
                         add_("mpeg2");add_("mpeg4");add_("mpg");add_("mpv2");add_("mts");add_("mtv");add_("mxf");add_("mxg");add_("nsv");add_("nuv");\
                         add_("ogg");add_("ogm");add_("ogv");add_("ogx");add_("ps");\
                         add_("rec");add_("rm");add_("rmvb");add_("tod");add_("ts");add_("tts");add_("vob");add_("vro");add_("webm");add_("wm");add_("wmv");add_("flv");

#define EXTENSIONS_PLAYLIST(add_) add_("asx");add_("b4s");add_("cue");add_("ifo");add_("m3u");add_("m3u8");add_("pls");add_("ram");add_("rar");add_("sdp");add_("m_player");add_("xspf");add_("wvx");add_("zip");add_("conf");

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
	opts.applicationName = "JucePlayer";
	opts.folderName = "JucePlayer";
	opts.commonToAllUsers = false;
	opts.filenameSuffix = "xml";
	opts.ignoreCaseOfKeyNames = true;
	opts.millisecondsBeforeSaving = 1000;
	opts.storageFormat = juce::PropertiesFile::storeAsXML;
	return opts;
}

////////////////////////////////////////////////////////////
//
// NATIVE m_player COMPONENT
//
////////////////////////////////////////////////////////////
/*
class VLCNativePopupComponent   : public juce::DirectShowComponent
{
public:
	VLCNativePopupComponent()
	{
		setMouseClickGrabsKeyboardFocus(false);
	}
};
*/
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
	virtual ~VLCNativePopupComponent(){}/*
	void paint (juce::Graphics& g)
	{
	}*/
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
	std::string m_title;
	bool m_allowDrag;
public:
	TitleComponent(juce::Component* componentToMove)
		:juce::Component("Title")
		,m_componentToMove(componentToMove)
		,m_allowDrag(false)
	{
		setOpaque(false);
	}
	virtual ~TitleComponent(){}
	void setTitle(std::string const& title){m_title=title;}
	void allowDrag(bool allow){m_allowDrag=allow;}
	void paint (juce::Graphics& g)
	{
		juce::String title = juce::String::fromUTF8(m_title.empty()? "Juce player":m_title.c_str());
		juce::Font f = g.getCurrentFont().withHeight((float)getHeight());
		f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);
		float textWidth = f.getStringWidthFloat(title);
		float rightBorder = 2.f*getHeight();
		textWidth = getWidth() - rightBorder;
		juce::Path path;
		path.lineTo(textWidth+rightBorder-2, 0);
		path.quadraticTo(textWidth+rightBorder/2.f, getHeight()-2.f, textWidth, getHeight()-2.f);
		path.lineTo(0.f, getHeight()-2.f);
		path.lineTo(0.f, 0.f);

		g.setGradientFill (juce::ColourGradient (juce::Colours::darkgrey.withAlpha(0.75f),
											getWidth()/2.f, (float)getHeight(),
											juce::Colours::black,
											getWidth()/2.f, 0.f,
											false));
		g.fillPath(path);

		g.setGradientFill (juce::ColourGradient (juce::Colours::grey.withAlpha(0.75f),
											getWidth()/2.f, (float)getHeight(),
											juce::Colours::black,
											getWidth()/2.f, 0.f,
											false));
		g.strokePath(path, juce::PathStrokeType(1.f));


		g.setColour (juce::Colours::white);
		g.drawFittedText (title,
							2, 2,(int)textWidth-4,getHeight()-4,
							juce::Justification::centredLeft,
							1, //1 line
							1.f//no h scale
							);
	}
	void mouseDown (const juce::MouseEvent& e)
	{
		if(e.eventComponent == this && isVisible() && m_allowDrag)
		{
			dragger.startDraggingComponent (m_componentToMove, e.getEventRelativeTo(m_componentToMove));
		}
	}

	void mouseDrag (const juce::MouseEvent& e)
	{
		if(e.eventComponent == this && isVisible() && m_allowDrag)
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
	:juce::Component("JucePlayer")
#endif
	,m_settings(juce::File::getCurrentWorkingDirectory().getChildFile("settings.xml"), options())
	,m_mediaTimes(juce::File::getCurrentWorkingDirectory().getChildFile("mediaTimes.xml"), options())
	,m_canHideOSD(true)
	,m_autoSubtitlesHeight(true)
	,m_backgroundTasks("BG tasks")
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
    asFrontpageImage = juce::Drawable::createFromImageData (frontpage_svg, frontpage_svgSize);
    likeAddImage = juce::Drawable::createFromImageData (likeadd_svg, likeadd_svgSize);
	likeRemoveImage = juce::Drawable::createFromImageData (likeremove_svg, likeremove_svgSize);
    addAllImage = juce::Drawable::createFromImageData (addall_svg, addall_svgSize);
	playAllImage = juce::Drawable::createFromImageData (play_svg, play_svgSize);

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
	setMenuTreeVisibleAndUpdateMenuButtonIcon(false);

	sliderUpdating = false;
	videoUpdating = false;


	//after set Size
	m_player = new Player();

#ifdef BUFFER_DISPLAY

	m_player->SetDisplayCallback(this);
#else
	vlcNativePopupComponent = new VLCNativePopupComponent();
	m_player->SetOutputWindow(vlcNativePopupComponent->getWindowHandle());
#endif

    m_player->SetEventCallBack(this);

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

	m_player->SetInputCallBack(this);
	mousehookset=  false;

	showVolumeSlider(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));

	addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar);

	setSize(800, 600);

	initFromSettings();

	setVisible (true);

	if(!isFullScreen())
	{
		//default window size if windowed
		centreWithSize(800, 600);
	}

	invokeLater = new InvokeLater();


	std::set<juce::String> m_frontpageExtensions;
	EXTENSIONS_VIDEO(m_frontpageExtensions.insert)
	EXTENSIONS_PLAYLIST(m_frontpageExtensions.insert)
	m_iconMenu.setFilter(m_frontpageExtensions);
	m_iconMenu.setMediaRootPath( m_settings.getValue(SETTINGS_POSTER_BROWSER_ROOT_PATH).toUTF8().getAddress() );

	m_backgroundTasks.addTimeSliceClient(this);
	m_backgroundTasks.startThread();

}

VideoComponent::~VideoComponent()
{
	m_backgroundTasks.removeTimeSliceClient(this);
	m_backgroundTasks.stopThread(30000);//wait 30sec max!

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
    m_player->SetEventCallBack(NULL);
#ifdef BUFFER_DISPLAY
	{
		m_player->SetTime(m_player->GetLength());
		m_player->Pause();
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
		m_player = nullptr;
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
// BG thread
//
////////////////////////////////////////////////////////////

int VideoComponent::useTimeSlice()
{
	if(isFrontpageVisible() && m_iconMenu.updatePreviews())
	{
		if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,this));
	}
	return 100;//ms before next call
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
		if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::switchFullScreen,this));
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
	if(m_player->isPlaying())
	{
		pause();
	}
	else if(m_player->isPaused())
	{
		play();
	}
}

void VideoComponent::setFullScreen(bool fs)
{
	juce::Desktop::getInstance().setBailoutKioskOnFocusLost(false);
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
bool VideoComponent::isFrontpageVisible()
{
	return (!vlcNativePopupComponent->isVisible() || m_player->isStopped()) && ! menu->asComponent()->isVisible();
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
		controlComponent->slider().setMouseOverTime(e.x, (juce::int64)(mouseMoveValue*m_player->GetLength()));
		//if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,boost::ref(controlComponent->slider())));
	}
	if(e.eventComponent == this && isFrontpageVisible())
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
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setAlpha,this, 1.f));

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
			if( isFrontpageVisible())
			{
				bool repaint = m_iconMenu.clickOrDrag((float)e.x, (float)e.y, (float)getWidth(), (float)getHeight());

				std::string media = m_iconMenu.getMediaAtIndexOnScreen((float)e.x, (float)e.y, (float)getWidth(), (float)getHeight());
				if(!media.empty())
				{
					juce::File m(juce::String::fromUTF8(media.c_str()));
					if(m.isDirectory())
					{
						m_iconMenu.setCurrentMediaRootPath(media);
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
				if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
			}
		}

	}
}

void VideoComponent::mouseWheelMove (const juce::MouseEvent& e,
                                const juce::MouseWheelDetails& wheel)
{

	if(e.eventComponent == this)
	{
		if( isFrontpageVisible())
		{
			float delta = wheel.isReversed ? - wheel.deltaY : wheel.deltaY;
			if(delta>0.f)
			{
				m_iconMenu.scrollDown();
				if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,e.eventComponent));
			}
			else if(delta<0.f)
			{
				m_iconMenu.scrollUp();
				if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,e.eventComponent));
			}


		}
	}
}
void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
	m_canHideOSD = e.eventComponent == this;//cannot hide sub component while dragging on sub component
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();


	if(e.eventComponent == this && isFrontpageVisible())
	{
		if(m_iconMenu.clickOrDrag((float)e.x, (float)e.y, (float)getWidth(), (float)getHeight()))
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&Component::repaint,e.eventComponent));
		}
	}

}
void VideoComponent::sliderValueChanged (juce::Slider* slider)
{
	if(!m_player)
	{
		return;
	}
	if(!videoUpdating)
	{
		sliderUpdating = true;
		m_player->SetTime((int64_t)(controlComponent->slider().getValue()*m_player->GetLength()/10000.));
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
	if(!m_player)
	{
		return;
	}
	if(button == &controlComponent->playPauseButton())
	{
		if(m_player->isPaused())
		{
			m_player->play();
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
	if(!vlcNativePopupComponent->isVisible() || m_player->isStopped() )
	{
		g.fillAll (juce::Colours::black);
		if(menu->asComponent()->isVisible())
		{
			g.drawImageAt(appImage, (getWidth() - appImage.getWidth())/2, (getHeight() - appImage.getHeight())/2 );


			juce::Font f = g.getCurrentFont().withHeight(menu->getFontHeight());
			f.setStyleFlags(juce::Font::plain);
			g.setFont(f);
			g.setColour (juce::Colours::grey);
			g.drawText(m_player->getInfo().c_str(),(getWidth() - appImage.getWidth())/2,
				(getHeight() + appImage.getHeight())/2, appImage.getWidth(),
				(int)menu->getFontHeight(),
				juce::Justification::centred, true);
		}
		else
		{
			m_iconMenu.paintMenu(g, (float)getWidth(), (float)getHeight());
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

    menu->asComponent()->setBounds (w-treeWidth,(int) menu->getItemHeight()+hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2-(int)menu->getItemHeight());
	controlComponent->setBounds (0, h-controlHeight, w, controlHeight);
}

void VideoComponent::resized()
{
	updateSubComponentsBounds();

#ifdef BUFFER_DISPLAY
	if(m_player)
	{
		//rebuild buffer
		bool restart(m_player->isPaused());
		m_player->Pause();

		const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);

		std::ostringstream oss;
		oss << "m_player "<< m_player->getInfo()<<"\n";
		oss << getWidth()<<"x"<< getHeight();
		juce::Graphics g(*img);
		g.fillAll(juce::Colour::fromRGB(0, 0, 0));
		g.setColour(juce::Colour::fromRGB(255, 0, 255));
		g.drawText(oss.str().c_str(), juce::Rectangle<int>(0, 0, img->getWidth(), img->getHeight()/10), juce::Justification::bottomLeft, true);
		if(restart)
		{
			m_player->Play();
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
        titleBar->setVisible (menu->asComponent()->isVisible() || !isFullScreen());
		titleBar->allowDrag(!isFullScreen());
		titleBar->setBounds(0, 0, getWidth(), (int)menu->getItemHeight());
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
	//dirty but m_player would not apply set time!!
	const int stepMs = 30;
	const int maxWaitMs = 750;
	for(int i= 0;i<maxWaitMs &&(m_player->GetTime()<start);i+=stepMs)
	{
		juce::Thread::sleep(stepMs);
	}
	m_player->SetTime(start);
}
void VideoComponent::forceSetVideoTime(std::string const& name)
{
	int time = m_mediaTimes.getIntValue(name.c_str(), 0);
	if(time>0)
	{
		forceSetVideoTime(time*1000);
	}
}
//shuffled addition:
//srand(unsigned(time(NULL)));
//std::random_shuffle(pathes.begin(), pathes.end());
void VideoComponent::appendAndPlay(std::string const& path)
{
	saveCurrentMediaTime();

	if(!m_player)
	{
		return;
	}
	std::string::size_type i = path.find_last_of("/\\");
	std::string name =  i == std::string::npos ? path : path.substr(i+1);

	int index = m_player->addPlayListItem(path);

#ifdef BUFFER_DISPLAY
	img = new juce::Image(img->rescaled(getWidth(), getHeight()));
	ptr = new juce::Image::BitmapData (*img, juce::Image::BitmapData::readWrite);
	m_player->SetBufferFormat(img->getWidth(), img->getHeight(), ptr->lineStride);
#else
    resized();
#endif

	//force mouseinput to be set again when the media starts to play
	m_player->setMouseInputCallBack(NULL);
    m_player->SetEventCallBack(this);

	m_player->playPlayListItem(index);

	forceSetVideoTime(name);

}

void VideoComponent::play()
{
	if(!m_player)
	{
		return;
	}

	m_player->play();

}

void VideoComponent::pause()
{
	if(!m_player)
	{
		return;
	}
	saveCurrentMediaTime();
	m_player->Pause();
}
void VideoComponent::stop()
{
	if(!m_player)
	{
		return;
	}
	saveCurrentMediaTime();
	m_player->Pause();
	controlComponent->slider().setValue(10000, juce::sendNotificationSync);
	m_player->Stop();
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
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::repaint,this));
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
	showVolumeSlider(m_player->getVolume());
}
void VideoComponent::showVolumeSlider(double value)
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Audio Volume: %.f%%"),
		boost::bind<void>(&Player::setVolume, m_player.get(), _1),
		value, 100., 1., 200., .1);
}
void VideoComponent::showPlaybackSpeedSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Speed: %.f%%"),
		boost::bind<void>(&Player::setRate, m_player.get(), _1),
		m_player->getRate(), 100., 50., 800., .1);
}
void VideoComponent::showZoomSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Zoom: %.f%%"),
		boost::bind<void>(&Player::setScale, m_player.get(), _1),
		m_player->getScale(), 100., 50., 500., .1);
}
void VideoComponent::showAudioOffsetSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Audio offset: %+.3fs"),
		boost::bind<void>(&VideoComponent::onMenuShiftAudio, boost::ref(*this), _1),
		m_player->getAudioDelay()/1000000., 0., -2., 2., .01, 2.);
}
void VideoComponent::showSubtitlesOffsetSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Subtitles offset: %+.3fs"),
		boost::bind<void>(&VideoComponent::onMenuShiftSubtitles, boost::ref(*this), _1),
		m_player->getSubtitleDelay()/1000000., 0., -2., 2., .01, 2.);
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
/*
    vlcNativePopupComponent->loadMovie(path);
    vlcNativePopupComponent->play();*/
}
void VideoComponent::onMenuQueue (AbstractMenuItem& item, juce::String path)
{
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
	m_player->addPlayListItem(path.toUTF8().getAddress());
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
				file.getFullPathName()), playAllImage.get());
		menu->addMenuItem(TRANS("Add All"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuQueue, this, _1,
				file.getFullPathName()), addAllImage.get());


		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false);
		FileSorter sorter(m_suportedExtensions);
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			menu->addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuOpenFolder, this, _1, file), getFolderImage());

		}
		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			menu->addMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuOpenFile, this, _1, file), getIcon(file));

		}

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			menu->addMenuItem(TRANS("Add to favorites"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAddFavorite, this, _1,
				file.getFullPathName()), likeAddImage.get());
		}
		else
		{
			menu->addMenuItem(TRANS("Remove from favorites"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRemoveFavorite, this, _1,
				file.getFullPathName()), likeRemoveImage.get());
		}


		menu->addMenuItem(TRANS("Set as frontpage"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSetFrontPage, this, _1,
				file.getFullPathName()), asFrontpageImage.get());

	}
}

void VideoComponent::onMenuSetFrontPage (AbstractMenuItem& item, juce::String path)
{
	m_settings.setValue(SETTINGS_POSTER_BROWSER_ROOT_PATH, path);
	m_iconMenu.setMediaRootPath(path.toUTF8().getAddress());
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
}
void VideoComponent::onMenuOpenFile (AbstractMenuItem& item, juce::File file)
{
	if(!file.isDirectory())
	{
		if(extensionMatch(m_subtitlesExtensions, file))
		{
			if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
			m_player->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
		}
		else
		{
			onMenuOpenUnconditionnal(item, file.getFullPathName());
		}
	}
}

void VideoComponent::onMenuSubtitleMenu(AbstractMenuItem& item)
{
	setBrowsingFiles(true);

	std::vector<std::pair<int, std::string> > subs = m_player->getSubtitles();
	int current = m_player->getCurrentSubtitleIndex();
	if(!subs.empty())
	{
		for(std::vector<std::pair<int, std::string> >::const_iterator i = subs.begin();i != subs.end();++i)
		{
			menu->addMenuItem( juce::String("-> ") + i->second.c_str(),
                     AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, i->first), i->first==current?getItemImage():nullptr);
		}
	}
	else
	{
		menu->addMenuItem( juce::String::formatted(TRANS("No subtitles")), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, -1), 0==current?getItemImage():nullptr);
	}
	menu->addMenuItem( TRANS("Add..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuListFiles, this, _1, &VideoComponent::onMenuOpenSubtitleFolder));
	//menu->addMenuItem( TRANS("opensubtitles.org"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchOpenSubtitles, this, _1));
	menu->addMenuItem( TRANS("SubtitleSeeker.com"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitleSeeker, this, _1));
	menu->addMenuItem( TRANS("Delay"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuShiftSubtitlesSlider, this, _1));

}
void VideoComponent::onMenuSearchOpenSubtitles(AbstractMenuItem& item)
{
	onMenuSearchOpenSubtitlesSelectLanguage(item, m_player->getCurrentPlayListItem().c_str());
}
void VideoComponent::onMenuSearchSubtitleSeeker(AbstractMenuItem& item)
{
	onMenuSearchSubtitleSeeker(item, m_player->getCurrentPlayListItem().c_str());
}
void VideoComponent::onMenuSearchSubtitlesManually(AbstractMenuItem& item, juce::String lang)
{
	juce::TextEditor editor("Subtitle search");

	editor.setText(m_player->getCurrentPlayListItem().c_str());
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
	movieName = movieName.replace(" ", "-");
	movieName = movieName.replace("_", "-");
	movieName = movieName.replace(".", "-");
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
bool isTVEpisode(juce::String str, int &season, int& episode)
{
#define EPISODE_NAME_PATTERN "\\+s([0-9]+)e([0-9]+)\\+"
    RegularExpression::Result matchesSubscene;
    if( RegularExpression::search(str.toRawUTF8(), matchesSubscene, RegularExpression(EPISODE_NAME_PATTERN, RegularExpression::icase)) )
    {
        season = boost::lexical_cast<int>(matchesSubscene[1].str());
        episode = boost::lexical_cast<int>(matchesSubscene[2].str());
        return true;
    }
    return false;
}
#define SUBTITLESEEKER_URL "&search_in=tv_episodes"
#define SUBTITLESEEKER_TV_EPISODE_OPTION "&search_in=tv_episodes"
void VideoComponent::onMenuSearchSubtitleSeeker(AbstractMenuItem& item, juce::String movieName)
{
	setBrowsingFiles(false);
	movieName = movieName.replace("%", "%37");
	movieName = movieName.replace("%20", "+");
	movieName = movieName.replace(" ", "+");
	movieName = movieName.replace("_", "+");
	movieName = movieName.replace(".", "+");
	std::string name = str( boost::format("http://api.subtitleseeker.com/search/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&q=%s&max_results=100")%std::string(movieName.toUTF8().getAddress()) );
	int season;
	int episode;
	bool tvEpisode = isTVEpisode(movieName, season, episode);
    if(tvEpisode)
    {
        name+=SUBTITLESEEKER_TV_EPISODE_OPTION;
    }
	juce::URL url(name.c_str());


	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		menu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeeker, this, _1 ,  movieName));
		return;
	}
	juce::String content = pIStream->readEntireStreamAsString();
	content = content.replace("&", "n");//xml would be invalid otherwise
	juce::ScopedPointer<juce::XmlElement> e(juce::XmlDocument::parse(content));
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
                        juce::String SEsuffix;
					    if(tvEpisode)
                        {
                            juce::String seasonStr = sub->getChildElementAllSubText("season", juce::String::empty);
                            juce::String episodeStr = sub->getChildElementAllSubText("episode", juce::String::empty);
//                            if(!seasonStr.isEmpty() && season!=boost::lexical_cast<int>( seasonStr ) &&
//                                !episodeStr.isEmpty() && episode!=boost::lexical_cast<int>( episodeStr ) )
//                            {
//                                continue;
//                            }
                            SEsuffix += " S"+seasonStr+"E"+episodeStr;
                        }
						menu->addMenuItem(name + SEsuffix + juce::String(" (") + year + juce::String(")"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1, imdb, tvEpisode, season, episode));
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

void VideoComponent::onMenuSearchSubtitleSeekerImdb(AbstractMenuItem& item, juce::String imdb, bool tvEpisode, int season, int episode)
{
	setBrowsingFiles(false);
	std::string name = str( boost::format("http://api.subtitleseeker.com/get/title_languages/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&imdb=%s")%imdb);
    juce::URL url(name.c_str());


	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		menu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1 ,  imdb, tvEpisode, season, episode));
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
						menu->addMenuItem(lang, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1, imdb ,lang, tvEpisode, season, episode));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}


	//menu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	menu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1 , imdb, tvEpisode, season, episode));
}
void VideoComponent::onMenuSearchSubtitleSeekerImdbLang(AbstractMenuItem& item, juce::String imdb, juce::String lang, bool tvEpisode, int season, int episode)
{
	setBrowsingFiles(false);
	std::string name = str( boost::format("http://api.subtitleseeker.com/get/title_subtitles/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&imdb=%s&language=%s")%imdb%lang);
    if(tvEpisode)
    {
        name+=str( boost::format("%s&season=%d&episode=%d")%SUBTITLESEEKER_TV_EPISODE_OPTION%season%episode);
    }
    juce::URL url(name.c_str());


	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		menu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1 ,  imdb, lang, tvEpisode, season, episode));
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
	menu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1 ,  imdb, lang, tvEpisode, season, episode));
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

bool VideoComponent::downloadedSubtitleSeekerResult(AbstractMenuItem& item, juce::String const& resultSite,
                                                     char* cstr,
                                                     juce::String const& siteTarget,
                                                     std::string const& match,
                                                     std::string const& downloadURLPattern )
{

    if(resultSite==siteTarget)
    {
        RegularExpression expressionSubscene(match, RegularExpression::icase);
        RegularExpression::Result matchesSubscene;
        if(RegularExpression::search(cstr, matchesSubscene, expressionSubscene))
        {

            juce::String downloadURL( str( boost::format(downloadURLPattern)% matchesSubscene[1].str()).c_str() );
            onMenuDowloadOpenSubtitle(item, downloadURL);
/*
            //get html
            juce::String outPath = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
            juce::File out(outPath);
            out = out.getChildFile(resultSite+".html");
            juce::FileOutputStream outStream(out);
            if(outStream.openedOk())
            {
                outStream.write(memStream.getData(), memStream.getDataSize());
                outStream.write(resultSite.getCharPointer().getAddress(), resultSite.length());
                outStream.write(cstr, strlen(cstr));
            }
            */
            return true;

        }
    }
    return false;
}
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
            memStream.writeByte(0);//simulate end of c string

            //find subseeker link to other subtitle site:
            std::string ex("href=\"([^\"]*");
            ex += site.toUTF8().getAddress();
            ex += "[^\"]*)";
            RegularExpression expression(ex, RegularExpression::icase);

           RegularExpression::Result matches;
           if(RegularExpression::search((char*)memStream.getData(), matches, expression))
           {
                juce::String otherStr(matches[1].str().c_str());

                if(downloadedSubtitleSeekerResult(item, site, otherStr.getCharPointer().getAddress(), "Opensubtitles.org",
                                                                        "/subtitles/([^/]*)/",
                                                                        "http://dl.opensubtitles.org/en/download/sub/%s"))
                {
                    return;
                }
                http://subsmax.com/subtitles-movie/treme-2010-dizicd-23-976fps-en-301kb-english-subtitle-zip/3821804
               //download other site page
                juce::URL other(otherStr);
                juce::ScopedPointer<juce::InputStream> pIStreamOther(other.createInputStream(false, 0, 0, "", SUBTITLE_DOWNLOAD_TIMEOUT_MS));
                if(pIStreamOther.get())
                {
                    memStream.reset();
                    if(memStream.writeFromInputStream(*pIStreamOther, MAX_SUBTITLE_ARCHIVE_SIZE)>0)
                    {
                        memStream.writeByte(0);//simulate end of c string

                        if(downloadedSubtitleSeekerResult(item, site, (char*)memStream.getData(), "Podnapisi.net",
                                                            "<a[^>]*class=\"button big download\"[^>]*href=\"([^\"]*)\"[^>]*>",
                                                            "http://www.podnapisi.net%s"))
                        {

                            //second level: "<a href='([^'])'>here</a>" --> "http://www.podnapisi.net%s"))
                            return;
                        }
                        if(downloadedSubtitleSeekerResult(item, site, (char*)memStream.getData(), "Subscene.com",
                                                            "<a.*href=\"([^\"]*)\".*id=\"downloadButton\".*>",
                                                            "http://subscene.com%s"))
                        {
                            return;
                        }
                        //need to look at iframe target before inspecting some pages:
                        //	<iframe width="100%" height="9000px" frameborder="0" marginheight="0" marginwidth="0" scrolling="no" src="http://www.engsub.net/NNNNNNNN/">
                        if(downloadedSubtitleSeekerResult(item, site, (char*)memStream.getData(), "Undertexter.se",
                                                            "<a[^>]*title=\"Download subtitle to[^\"][^>]*\".*href=\"([^\"]*)\".*>",
                                                            "%s"))
                        {
                            return;
                        }
                    }
                }
                //found other site page but could not go further us it at least
                juce::Process::openDocument(otherStr,juce::String::empty);
                return;
           }

        }
    }
    juce::Process::openDocument(downloadUrl,juce::String::empty);
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
                onMenuOpenSubtitleFile(item,out.getChildFile(entriesToExtract.getFirst()->filename));

            }
		}
	}
}
void VideoComponent::onMenuSubtitleSelect(AbstractMenuItem& item, int i)
{
	setBrowsingFiles(false);
	m_player->setSubtitleIndex(i);
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
		FileSorter sorter(m_subtitlesExtensions);
		destArray.sort(sorter);
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
		m_player->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
	}
}
void VideoComponent::onMenuOpenPlaylist (AbstractMenuItem& item, juce::File file)
{
}

void VideoComponent::onMenuZoom(AbstractMenuItem& item, double ratio)
{
	setBrowsingFiles(false);
	m_player->setScale(ratio);

	showZoomSlider();
}
void VideoComponent::onMenuCrop (AbstractMenuItem& item, juce::String ratio)
{
	setBrowsingFiles(false);

	m_player->setAutoCrop(false);
	m_player->setCrop(std::string(ratio.getCharPointer().getAddress()));

	m_settings.setValue(SETTINGS_CROP, ratio);
}
void VideoComponent::onMenuAutoCrop (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	m_player->setAutoCrop(true);
}
void VideoComponent::onMenuCropList (AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	//menu->addMenuItem( TRANS("Auto"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAutoCrop, this, _1), m_player->isAutoCrop()?getItemImage():nullptr);
	std::string current = m_player->getCrop();
	std::vector<std::string> list = m_player->getCropList();
	for(std::vector<std::string>::const_iterator it = list.begin();it != list.end();++it)
	{
		juce::String ratio(it->c_str());
		menu->addMenuItem( ratio.isEmpty()?TRANS("Original"):ratio, AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuCrop, this, _1, ratio), *it==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuRate (AbstractMenuItem& item, double rate)
{
	setBrowsingFiles(false);
	m_player->setRate(rate);

	showPlaybackSpeedSlider();
}
void VideoComponent::onMenuRateListAndSlider (AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	showPlaybackSpeedSlider();

	menu->addMenuItem( "50%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 50.), 50==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "100%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 100.), 100==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "125%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 125.), 125==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "150%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 150.), 150==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "200%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 200.), 200==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "300%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 300.), 300==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "400%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 400.), 400==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "600%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 600.), 600==(int)(m_player->getRate())?getItemImage():nullptr);
	menu->addMenuItem( "800%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuRate, this, _1, 800.), 800==(int)(m_player->getRate())?getItemImage():nullptr);

}
void VideoComponent::onMenuShiftAudio(double s)
{
	setBrowsingFiles(false);
	m_player->setAudioDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftAudioSlider(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	showAudioOffsetSlider();
}
void VideoComponent::onMenuShiftSubtitles(double s)
{
	setBrowsingFiles(false);
	m_player->setSubtitleDelay((int64_t)(s*1000000.));
}
void VideoComponent::onMenuShiftSubtitlesSlider(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	showSubtitlesOffsetSlider();
}

void VideoComponent::onMenuAudioVolume(AbstractMenuItem& item, double volume)
{
	setBrowsingFiles(false);
	m_player->setVolume(volume);

	showVolumeSlider();

	m_settings.setValue(SETTINGS_VOLUME, m_player->getVolume());
}

void VideoComponent::onMenuAudioVolumeListAndSlider(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	showVolumeSlider();

	menu->addMenuItem( "10%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 10.), 10==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "25%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 25.), 25==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "50%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 50.), 50==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "75%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 75.), 75==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "100%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 100.), 100==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "125%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 125.), 125==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "150%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 150.), 150==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "175%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 175.), 175==(int)(m_player->getVolume())?getItemImage():nullptr);
	menu->addMenuItem( "200%", AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioVolume, this, _1, 200.), 200==(int)(m_player->getVolume())?getItemImage():nullptr);
}

void VideoComponent::onMenuFullscreen(AbstractMenuItem& item, bool fs)
{
	setBrowsingFiles(false);
	setFullScreen(fs);
}

void VideoComponent::onMenuAudioTrack (AbstractMenuItem& item, int id)
{
	setBrowsingFiles(false);
	m_player->setAudioTrack(id);
}
void VideoComponent::onMenuAudioTrackList (AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	int current = m_player->getAudioTrack();
	std::vector<std::pair<int, std::string> > list = m_player->getAudioTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		menu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuAudioTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onMenuVideoAdjust (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	m_player->setVideoAdjust(!m_player->getVideoAdjust());
}
void VideoComponent::onMenuVideoContrast (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Contrast: %+.3fs"),
		boost::bind<void>(&Player::setVideoContrast, m_player.get(), _1),
		m_player->getVideoContrast(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoBrightness (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Brightness: %+.3f"),
		boost::bind<void>(&Player::setVideoBrightness, m_player.get(), _1),
		m_player->getVideoBrightness(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoHue (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Hue"),
		boost::bind<void>(&Player::setVideoHue, m_player.get(), _1),
		m_player->getVideoHue(), m_player->getVideoHue(), 0, 256., .1);
}
void  VideoComponent::onMenuVideoSaturation (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Saturation: %+.3f"),
		boost::bind<void>(&Player::setVideoSaturation, m_player.get(), _1),
		m_player->getVideoSaturation(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoGamma (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Gamma: %+.3f"),
		boost::bind<void>(&Player::setVideoGamma, m_player.get(), _1),
		m_player->getVideoGamma(), 1., 0., 2., .01);
}

void VideoComponent::onMenuVideoTrack (AbstractMenuItem& item, int id)
{
	setBrowsingFiles(false);
	m_player->setVideoTrack(id);
}
void VideoComponent::onMenuVideoTrackList (AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	int current = m_player->getVideoTrack();
	std::vector<std::pair<int, std::string> > list = m_player->getVideoTrackList();
	for(std::vector<std::pair<int, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		menu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuVideoTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onVLCAudioChannelSelect(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	Player::AudioChannel c = m_player->getAudioChannel();

	menu->addMenuItem(TRANS("Stereo"), AbstractMenuItem::REFRESH_MENU, boost::bind(&Player::setAudioChannel, m_player.get(), Player::VLCWrapperAudioChannel_Stereo), c==Player::VLCWrapperAudioChannel_Stereo?getItemImage():nullptr);
	menu->addMenuItem(TRANS("Reverse"), AbstractMenuItem::REFRESH_MENU, boost::bind(&Player::setAudioChannel, m_player.get(), Player::VLCWrapperAudioChannel_RStereo), c==Player::VLCWrapperAudioChannel_RStereo?getItemImage():nullptr);
	menu->addMenuItem(TRANS("Left"), AbstractMenuItem::REFRESH_MENU, boost::bind(&Player::setAudioChannel, m_player.get(), Player::VLCWrapperAudioChannel_Left), c==Player::VLCWrapperAudioChannel_Left?getItemImage():nullptr);
	menu->addMenuItem(TRANS("Right"), AbstractMenuItem::REFRESH_MENU, boost::bind(&Player::setAudioChannel, m_player.get(), Player::VLCWrapperAudioChannel_Right), c==Player::VLCWrapperAudioChannel_Right?getItemImage():nullptr);
	menu->addMenuItem(TRANS("Dolby"), AbstractMenuItem::REFRESH_MENU, boost::bind(&Player::setAudioChannel, m_player.get(), Player::VLCWrapperAudioChannel_Dolbys), c==Player::VLCWrapperAudioChannel_Dolbys?getItemImage():nullptr);
}

void VideoComponent::onVLCAudioOutputDeviceSelect(AbstractMenuItem& item, std::string output, std::string device)
{
	setBrowsingFiles(false);
	m_player->setAudioOutputDevice(output, device);
	m_settings.setValue(SETTINGS_AUDIO_OUTPUT, juce::String(output.c_str()));
	m_settings.setValue(SETTINGS_AUDIO_DEVICE, juce::String(device.c_str()));
}
void VideoComponent::onVLCAudioOutputSelect(AbstractMenuItem& item, std::string output, std::vector< std::pair<std::string, std::string> > list)
{
	setBrowsingFiles(false);

	for(std::vector< std::pair<std::string, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		bool selected = m_settings.getValue(SETTINGS_AUDIO_DEVICE)==juce::String(it->second.c_str());
		menu->addMenuItem(it->first, AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onVLCAudioOutputDeviceSelect, this, _1, output, it->second), selected?getItemImage():nullptr);
	}
}
void VideoComponent::onVLCAudioOutputList(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > > list = m_player->getAudioOutputList();

	if(list.size() == 1)
	{
		onVLCAudioOutputSelect(item, list.front().first.second, list.front().second);
	}
	else
	{
		for(std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > >::const_iterator it = list.begin();it != list.end();++it)
		{
			bool selected = m_settings.getValue(SETTINGS_AUDIO_OUTPUT)==juce::String(it->first.second.c_str());
			menu->addMenuItem(it->first.first, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCAudioOutputSelect, this, _1, it->first.second, it->second), selected?getItemImage():nullptr);
		}
	}
}
void VideoComponent::onMenuSoundOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("Volume"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuAudioVolumeListAndSlider, this, _1));
	menu->addMenuItem( TRANS("Delay"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuShiftAudioSlider, this, _1));
	menu->addMenuItem( TRANS("Select Track"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuAudioTrackList, this, _1));
	menu->addMenuItem( TRANS("Channel"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCAudioChannelSelect, this, _1));
	menu->addMenuItem( TRANS("Output"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCAudioOutputList, this, _1));
//	menu->addMenuItem( TRANS("Audio visu."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onVLCOptionStringMenu, this, _1, std::string(CONFIG_STRING_OPTION_AUDIO_VISUAL)));
}

void VideoComponent::onMenuSetAspectRatio(AbstractMenuItem& item, juce::String ratio)
{
	setBrowsingFiles(false);
	m_player->setAspect(ratio.getCharPointer().getAddress());
}
void VideoComponent::onMenuRatio(AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	std::string current = m_player->getAspect();

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

	menu->addMenuItem( TRANS("Enable"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuVideoAdjust, this, _1), m_player->getVideoAdjust()?getItemImage():nullptr);
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
		std::vector<std::string > list = m_player->getCurrentPlayList();
		name = list.at(index);
	}
	catch(std::exception const& )
	{
	}
	//force mouseinput to be set again when the media starts to play
	m_player->setMouseInputCallBack(NULL);
    m_player->SetEventCallBack(this);

	m_player->playPlayListItem(index);

	forceSetVideoTime(name);
}
void VideoComponent::onShowPlaylist(AbstractMenuItem& item)
{
	setBrowsingFiles(true);

	int current = m_player->getCurrentPlayListItemIndex ();
	std::vector<std::string > list = m_player->getCurrentPlayList();
	int i=0;
	for(std::vector< std::string >::const_iterator it = list.begin();it != list.end();++it)
	{
		menu->addMenuItem(juce::CharPointer_UTF8(it->c_str()), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onPlaylistItem, this, _1, i), i==current?getItemImage():nullptr);
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

void VideoComponent::onPlayerOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("FullScreen"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuFullscreen, this, _1, true), isFullScreen()?getItemImage():nullptr);
	menu->addMenuItem( TRANS("Windowed"), AbstractMenuItem::REFRESH_MENU, boost::bind(&VideoComponent::onMenuFullscreen, this, _1, false), isFullScreen()?nullptr:getItemImage());

	menu->addMenuItem( TRANS("Language"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onLanguageOptions, this, _1));
	menu->addMenuItem( TRANS("Menu font size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onPlayerFonSize, this, _1));
}
void VideoComponent::onMenuRoot(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	menu->addMenuItem( TRANS("Open"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuListFiles, this, _1, &VideoComponent::onMenuOpenFolder), getFolderImage());
	menu->addMenuItem( TRANS("Now playing"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onShowPlaylist, this, _1), getPlaylistImage());
	menu->addMenuItem( TRANS("Subtitles"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSubtitleMenu, this, _1), getSubtitlesImage());
	menu->addMenuItem( TRANS("Video"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuVideoOptions, this, _1), getDisplayImage());
	menu->addMenuItem( TRANS("Sound"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onMenuSoundOptions, this, _1), getAudioImage());
	menu->addMenuItem( TRANS("Player"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, boost::bind(&VideoComponent::onPlayerOptions, this, _1), getSettingsImage());
	menu->addMenuItem( TRANS("Exit"), AbstractMenuItem::EXECUTE_ONLY, boost::bind(&VideoComponent::onMenuExit, this, _1), getExitImage());

}
////////////////////////////////////////////////////////////
//
// m_player CALLBACKS
//
////////////////////////////////////////////////////////////
void VideoComponent::vlcTimeChanged(int64_t newTime)
{
	if(!m_player)
	{
		return;
	}
	if(!mousehookset)
	{
		mousehookset = m_player->setMouseInputCallBack(this);
	}
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::updateTimeAndSlider,this, newTime));
}

void VideoComponent::updateTimeAndSlider(int64_t newTime)
{
	if(!sliderUpdating)
	{
		videoUpdating = true;
		controlComponent->slider().setValue(newTime*10000./m_player->GetLength(), juce::sendNotificationSync);
		controlComponent->setTime(newTime, m_player->GetLength());
		if(invokeLater)invokeLater->queuef(boost::bind  (&ControlComponent::repaint,controlComponent.get()));
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
		bool showControls = m_player->isPlaying() || m_player->isPaused();
		controlComponent->setVisible(showControls);
		titleBar->setVisible((menu->asComponent()->isVisible() || !isFullScreen()) && showControls);
		titleBar->allowDrag(!isFullScreen());
	}
	else
	{
		//DBG ( (long)timeFromLastMouseMove  << "->" << (long)timeFromLastMouseMove-DISAPEAR_DELAY_MS << "/" << DISAPEAR_SPEED_MS << " -> hide() " );
		setMenuTreeVisibleAndUpdateMenuButtonIcon(false);
		controlComponent->setVisible(false);
		titleBar->setVisible(false);
	}

}
void VideoComponent::vlcPaused()
{
	if(invokeLater)invokeLater->queuef(boost::bind  (&ControlComponent::showPausedControls,controlComponent.get()));
}
void VideoComponent::vlcStarted()
{
	titleBar->setTitle(m_player->getCurrentPlayListItem());
	if(invokeLater)invokeLater->queuef(boost::bind  (&ControlComponent::showPlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::startedSynchronous,this));
}
void VideoComponent::vlcStopped()
{
	titleBar->setTitle(std::string());
	if(invokeLater)invokeLater->queuef(boost::bind  (&ControlComponent::hidePlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::stoppedSynchronous,this));
}

void VideoComponent::vlcPopupCallback(bool rightClick)
{
	//DBG("vlcPopupCallback." << (rightClick?"rightClick":"leftClick") );
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

	//prevent menu to disappear too quickly
	m_canHideOSD = !rightClick;

	bool showMenu = rightClick || m_player->isStopped();
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, showMenu));
	if(invokeLater)invokeLater->queuef(boost::bind  (&Component::toFront,this, true));
	if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::handleIdleTimeAndControlsVisibility,this));

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
	if(controlsExpired || m_player->isPaused())
	{
		//reactivateControls
		if(invokeLater)invokeLater->queuef(boost::bind  (&VideoComponent::handleIdleTimeAndControlsVisibility,this));
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
		setMenuTreeVisibleAndUpdateMenuButtonIcon(false);
		getPeer()->getComponent().removeComponentListener(this);
	}
}

void VideoComponent::initFromMediaDependantSettings()
{
	m_player->setVolume(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));

	m_player->setCrop(m_settings.getValue(SETTINGS_CROP, "").toUTF8().getAddress());

	m_player->setAudioOutputDevice(m_settings.getValue(SETTINGS_AUDIO_OUTPUT).toUTF8().getAddress(), m_settings.getValue(SETTINGS_AUDIO_DEVICE).toUTF8().getAddress());
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
	if(!m_player || !m_player->isPlaying())
	{
		return;
	}
	std::string media = m_player->getCurrentPlayListItem();
	if(media.empty())
	{

		return;
	}
	int64_t time = m_player->GetTime();
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
