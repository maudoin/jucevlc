
#include "VideoComponent.h"
#include "HueSelector.h"
#include "Icons.h"
#include "MenuComponent.h"
#include "Languages.h"
#include "FileSorter.h"
#include "InvokeLater.h"
#include "Extensions.h"
#include <algorithm>
#include <set>
#include <format>
#include <functional>
#include <regex>

#define DISAPEAR_DELAY_MS 500
#define DISAPEAR_SPEED_MS 500
#define MAX_SUBTITLE_ARCHIVE_SIZE 1024*1024
#define SUBTITLE_DOWNLOAD_TIMEOUT_MS 30000
#define TIME_JUMP 10000
#define VOLUME_SCROLL_STEP 2.

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
		juce::String title = juce::String::fromUTF8(m_title.empty()? "JuceVLC player":m_title.c_str());
		juce::Font f = g.getCurrentFont().withHeight((float)getHeight());
		//f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);
		float textWidth = f.getStringWidthFloat(title);
		float rightBorder = 2.f*getHeight();
		textWidth = getWidth() - rightBorder;

		g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha(0.f),
											getWidth()/2.f, (float)getHeight(),
											juce::Colours::black,
											getWidth()/2.f, 0.f,
											false));
		g.fillRect(0.f, 0.f, float(getWidth()), float(getHeight()));


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
	, browsingFiles(false)
	,m_settings(juce::File::getCurrentWorkingDirectory().getChildFile("settings.xml"), options())
	,m_mediaTimes(juce::File::getCurrentWorkingDirectory().getChildFile("mediaTimes.xml"), options())
	,m_canHideOSD(true)
	,m_autoSubtitlesHeight(true)
	,m_backgroundTasks("BG tasks")
	, m_fileMenu(std::make_unique<MenuComponent>(false))
{
	Languages::getInstance();

	vlcMediaUPNPList = std::make_unique<BackgoundUPNP>();

	//m_toolTip = new juce::TooltipWindow( this,50);

    appImage = juce::ImageFileFormat::loadFrom(Icons::vlc_png, Icons::vlc_pngSize);

    itemImage               = juce::Drawable::createFromImageData (Icons::blank_svg, Icons::blank_svgSize);
    folderImage             = juce::Drawable::createFromImageData (Icons::openmenu_svg, Icons::openmenu_svgSize);
    playlistImage           = juce::Drawable::createFromImageData (Icons::playlist_svg, Icons::playlist_svgSize);
    folderShortcutImage     = juce::Drawable::createFromImageData (Icons::openshort_svg, Icons::openshort_svgSize);
    hideFolderShortcutImage = juce::Drawable::createFromImageData (Icons::hideopen_svg, Icons::hideopen_svgSize);
    audioImage              = juce::Drawable::createFromImageData (Icons::soundon_svg, Icons::soundon_svgSize);
    displayImage            = juce::Drawable::createFromImageData (Icons::image_svg, Icons::image_svgSize);
    subtitlesImage          = juce::Drawable::createFromImageData (Icons::subtitles_svg, Icons::subtitles_svgSize);
    exitImage               = juce::Drawable::createFromImageData (Icons::off_svg, Icons::off_svgSize);
    settingsImage           = juce::Drawable::createFromImageData (Icons::optionssettings_svg, Icons::optionssettings_svgSize);
    speedImage              = juce::Drawable::createFromImageData (Icons::speed_svg, Icons::speed_svgSize);
    audioShiftImage         = juce::Drawable::createFromImageData (Icons::soundshift_svg, Icons::soundshift_svgSize);
    clockImage              = juce::Drawable::createFromImageData (Icons::clock_svg, Icons::clock_svgSize);
    asFrontpageImage        = juce::Drawable::createFromImageData (Icons::frontpage_svg, Icons::frontpage_svgSize);
    likeAddImage            = juce::Drawable::createFromImageData (Icons::likeadd_svg, Icons::likeadd_svgSize);
	likeRemoveImage         = juce::Drawable::createFromImageData (Icons::likeremove_svg, Icons::likeremove_svgSize);
    addAllImage             = juce::Drawable::createFromImageData (Icons::addall_svg, Icons::addall_svgSize);
	playAllImage            = juce::Drawable::createFromImageData (Icons::play_svg, Icons::play_svgSize);


	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	controlComponent = std::make_unique<ControlComponent>();
	controlComponent->slider().addListener(this);
	controlComponent->playPauseButton().addListener(this);
	controlComponent->stopButton().addListener(this);
	controlComponent->fullscreenButton().addListener(this);
	controlComponent->menuButton().addListener(this);
	controlComponent->auxilliarySliderModeButton().addListener(this);
	controlComponent->resetButton().addListener(this);
	controlComponent->addMouseListener(this, true);

	m_optionsMenu = std::make_unique<MenuComponent>();
	m_optionsMenu->asComponent()->addMouseListener(this, true);

    addChildComponent(*controlComponent);
    addChildComponent (m_optionsMenu->asComponent());
	setMenuTreeVisibleAndUpdateMenuButtonIcon(false);

	sliderUpdating = false;
	videoUpdating = false;


	//after set Size
	vlc = std::make_unique<VLCWrapper>();

#ifdef BUFFER_DISPLAY

	vlc->SetDisplayCallback(this);
#else
	vlcNativePopupComponent = std::make_unique<VLCNativePopupComponent>();
	vlc->SetOutputWindow(vlcNativePopupComponent->getWindowHandle());

#endif

    vlc->SetEventCallBack(this);


	////////////////
	m_optionsMenu->setScaleComponent(this);
	controlComponent->setScaleComponent(this);
	controlComponent->slider().setScaleComponent(this);
	m_fileMenu->setScaleComponent(this);


    defaultConstrainer.setMinimumSize (100, 100);
	addChildComponent (*(titleBar = std::make_unique<TitleComponent>(this)));
	titleBar->addMouseListener(this, true);
    addChildComponent (*(resizableBorder = std::make_unique<juce::ResizableBorderComponent>(this, &defaultConstrainer)));

	addKeyListener(this);

    // And show it!
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);

	vlc->SetInputCallBack(this);
	mousehookset=  false;

	showVolumeSlider(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));

	addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar);

	setSize(800, 600);

	initFromSettings();

	m_optionsMenu->addRecentMenuItem("Menu", AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onOptionMenuRoot, this, _1), getItemImage());
	m_optionsMenu->forceMenuRefresh();

	m_fileMenu->addRecentMenuItem(juce::String("JUCE + VLC ") + vlc->getInfo().c_str(), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		[this](auto& item){this->onFileMenuRoot( item, [this](auto& item, auto const& file){this->onMenuOpenFolder(item, file);});},
		getItemImage());
	m_fileMenu->forceMenuRefresh();
    addAndMakeVisible (m_fileMenu->asComponent());

	setVisible (true);

	if(!isFullScreen())
	{
		//default window size if windowed
		centreWithSize(800, 600);
	}

	invokeLater = std::make_unique<InvokeLater>();


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
	m_optionsMenu = nullptr;
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
// BG thread
//
////////////////////////////////////////////////////////////

int VideoComponent::useTimeSlice()
{
	if(isFrontpageVisible() != m_fileMenu->asComponent()->isVisible())
	{
		if(invokeLater)invokeLater->queuef([this]{this->m_fileMenu->asComponent()->setVisible(isFrontpageVisible());});
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
		if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::switchFullScreen,this));
		return true;
	}
	if(key.isKeyCurrentlyDown(juce::KeyPress::spaceKey))
	{
		switchPlayPause();
		return true;
	}
	if(key.isKeyCurrentlyDown(juce::KeyPress::leftKey))
	{
		rewindTime();
		return true;
	}
	if(key.isKeyCurrentlyDown(juce::KeyPress::rightKey))
	{
		advanceTime();
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
	//juce::Desktop::getInstance().setBailoutKioskOnFocusLost(false);
	juce::Desktop::getInstance().setKioskModeComponent (fs?getTopLevelComponent():nullptr);
	if(fs)
	{
		controlComponent->showFullscreenControls();
	}
	else
	{
		controlComponent->showWindowedControls();
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
	return (!vlcNativePopupComponent->isVisible() || vlc->isStopped()) && ! m_optionsMenu->asComponent()->isVisible();
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
		//if(invokeLater)invokeLater->queuef(std::bind(&Component::repaint,std::ref(controlComponent->slider())));
	}
}

void VideoComponent::mouseExit (const juce::MouseEvent& e)
{
	if(e.eventComponent == &controlComponent->slider())
	{
		controlComponent->slider().resetMouseOverTime();
		//if(invokeLater)invokeLater->queuef(std::bind(&Component::repaint,std::ref(controlComponent->slider())));
	}
}

void VideoComponent::mouseDown (const juce::MouseEvent& e)
{
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setAlpha,this, 1.f));

	//DBG ( "click " << e.eventComponent->getName() );
	if(e.eventComponent == this)
	{
		if( !isFrontpageVisible())
		{
			if(e.mods.isRightButtonDown())
			{
				//prevent m_optionsMenu to disappear too quickly
				m_canHideOSD = false;
				lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

				if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, true));
			}
			if(e.mods.isLeftButtonDown())
			{
				if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
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
		}
	}
}
void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
	m_canHideOSD = e.eventComponent == this;//cannot hide sub component while dragging on sub component
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();


	if(e.eventComponent == this && isFrontpageVisible())
	{
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
	m_optionsMenu->asComponent()->setVisible(visible);
	m_fileMenu->asComponent()->setVisible(false);
	controlComponent->menuButton().setImages(m_optionsMenu->asComponent()->isVisible()?hideFolderShortcutImage.get():folderShortcutImage.get());
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
		setMenuTreeVisibleAndUpdateMenuButtonIcon(!m_optionsMenu->asComponent()->isVisible());
	}
	else if(button == &controlComponent->resetButton())
	{
		controlComponent->auxilliaryControlComponent().reset();
	}
	else if(button == &controlComponent->auxilliarySliderModeButton())
	{

		int buttonWidth = (int)(0.03*controlComponent->getWidth());

        juce::PopupMenu m;
		m.addCustomItem (E_POPUP_ITEM_VOLUME_SLIDER, std::make_unique<DrawableMenuComponent>(audioImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_SUBTITLES_DELAY_SLIDER, std::make_unique<DrawableMenuComponent>(subtitlesImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_VOLUME_DELAY_SLIDER, std::make_unique<DrawableMenuComponent>(audioShiftImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_PLAY_SPEED_SLIDER, std::make_unique<DrawableMenuComponent>(speedImage.get(), buttonWidth));
        m.addCustomItem (E_POPUP_ITEM_SHOW_CURRENT_TIME, std::make_unique<DrawableMenuComponent>(clockImage.get(), buttonWidth));

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
		if(m_optionsMenu->asComponent()->isVisible())
		{
			g.drawImageAt(appImage, (getWidth() - appImage.getWidth())/2, (getHeight() - appImage.getHeight())/2 );


			juce::Font f = g.getCurrentFont().withHeight(m_optionsMenu->getFontHeight());
			f.setStyleFlags(juce::Font::plain);
			g.setFont(f);
			g.setColour (juce::Colours::grey);
			g.drawText(juce::String("Featuring VLC ") + vlc->getInfo().c_str(),(getWidth() - appImage.getWidth())/2,
				(getHeight() + appImage.getHeight())/2, appImage.getWidth(),
				(int)m_optionsMenu->getFontHeight(),
				juce::Justification::centred, true);
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

	int hMargin = (int)(m_optionsMenu->getItemHeight()/2.);
	int treeWidth = (browsingFiles?3:1)*w/4;
	int controlHeight = 3*(int)m_optionsMenu->getItemHeight();
    m_optionsMenu->asComponent()->setBounds (w-treeWidth,(int) m_optionsMenu->getItemHeight()+hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2-(int)m_optionsMenu->getItemHeight());

	int frontpageMargin = (int)m_optionsMenu->getItemHeight();
    m_fileMenu->asComponent()->setBounds (frontpageMargin, frontpageMargin, w-2*frontpageMargin, h-2*frontpageMargin);

	controlComponent->setBounds (0, h-controlHeight, w, controlHeight);
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
        titleBar->setVisible (m_optionsMenu->asComponent()->isVisible() || !isFullScreen());
		titleBar->allowDrag(!isFullScreen());
		titleBar->setBounds(0, 0, getWidth(), (int)m_optionsMenu->getItemHeight());
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
//shuffled addition:
//srand(unsigned(time(NULL)));
//std::random_shuffle(pathes.begin(), pathes.end());
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

	//force mouseinput to be set again when the media starts to play
	vlc->setMouseInputCallBack(NULL);
    vlc->SetEventCallBack(this);

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

void VideoComponent::rewindTime ()
{
	if(!vlc)
	{
		return;
	}
	if(!videoUpdating)
	{
		sliderUpdating = true;
		vlc->SetTime(vlc->GetTime()-TIME_JUMP);
		sliderUpdating =false;
		lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();
		handleIdleTimeAndControlsVisibility();
	}
}

void VideoComponent::advanceTime ()
{
	if(!vlc)
	{
		return;
	}
	if(!videoUpdating)
	{
		sliderUpdating = true;
		vlc->SetTime(vlc->GetTime()+TIME_JUMP);
		sliderUpdating =false;
		lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();
		handleIdleTimeAndControlsVisibility();
	}
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
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::repaint,this));
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
	showVolumeSlider(vlc->getVolume());
}
void VideoComponent::showVolumeSlider(double value)
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Audio Volume: %.f%%"),
		std::bind<void>(&VLCWrapper::setVolume, vlc.get(), _1),
		value, 100., 1., 200., .1);
}
void VideoComponent::showPlaybackSpeedSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Speed: %.f%%"),
		std::bind<void>(&VLCWrapper::setRate, vlc.get(), _1),
		vlc->getRate(), 100., 50., 800., .1);
}
void VideoComponent::showZoomSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Zoom: %.f%%"),
		std::bind<void>(&VLCWrapper::setScale, vlc.get(), _1),
		vlc->getScale(), 100., 50., 500., .1);
}
void VideoComponent::showAudioOffsetSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Audio offset: %+.3fs"),
		std::bind<void>(&VideoComponent::onMenuShiftAudio, std::ref(*this), _1),
		vlc->getAudioDelay()/1000000., 0., -2., 2., .01, 2.);
}
void VideoComponent::showSubtitlesOffsetSlider ()
{
	controlComponent->auxilliaryControlComponent().show(TRANS("Subtitles offset: %+.3fs"),
		std::bind<void>(&VideoComponent::onMenuShiftSubtitles, std::ref(*this), _1),
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
		updateSubComponentsBounds();//m_optionsMenu may be larger! (or not)
	}
}
juce::String name(juce::File const& file)
{
	juce::File p = file.getParentDirectory();
	return p.getFullPathName() == file.getFullPathName() ?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileName();
}

void VideoComponent::onFileMenuRoot(AbstractMenuItem& item, FileMethod fileMethod)
{
	juce::String path = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
	juce::File f(path);

	m_fileMenu->addMenuItem( TRANS("Exit"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuExit, this, _1), getExitImage());
	onMenuListFavorites(item, fileMethod);
}

void VideoComponent::onMenuLoadSubtitle(AbstractMenuItem& item, FileMethod fileMethod)
{
	juce::String path = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
	juce::File f(path);
	m_fileMenu->listRecentPath(item, fileMethod, f);
}

void VideoComponent::onMenuListRootFiles(AbstractMenuItem& item, FileMethod fileMethod)
{
	m_fileMenu->listRootFiles(item, fileMethod);
	m_fileMenu->addMenuItem( TRANS("UPNP videos..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuListUPNPFiles, this, _1, std::vector<std::string>()), getItemImage());
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

void VideoComponent::onMenuListUPNPFiles(AbstractMenuItem& item, std::vector<std::string> path)
{
	std::vector<std::pair<std::string, std::string> > list = vlcMediaUPNPList->getUPNPList(path);
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		if(std::string::npos == std::string(it->second).find("vlc://nop"))
		{
			m_fileMenu->addMenuItem( it->first.c_str(), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuOpenUnconditionnal, this, _1, juce::String::fromUTF8(it->second.c_str())), m_optionsMenu->getIcon(getPathExtensionWithoutDot(it->first).c_str()));
		}
		else
		{
			std::vector<std::string> newPath(path);
			newPath.push_back(it->first);
			m_fileMenu->addMenuItem( it->first.c_str(), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuListUPNPFiles, this, _1, newPath), folderImage.get());
		}
	}


}
void VideoComponent::onMenuListFavorites(AbstractMenuItem& item, FileMethod fileMethod)
{

	mayPurgeFavorites();

	m_fileMenu->addMenuItem( TRANS("All videos..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuListRootFiles, this, _1, fileMethod), getItemImage());

	m_fileMenu->listShortcuts(item, fileMethod, m_shortcuts);
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
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
	appendAndPlay(path.toUTF8().getAddress());
}
void VideoComponent::onMenuQueue (AbstractMenuItem& item, juce::String path)
{
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
	vlc->addPlayListItem(path.toUTF8().getAddress());
}

void VideoComponent::onMenuOpenFolder (AbstractMenuItem& item, juce::File file)
{
	if(file.isDirectory())
	{
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		m_fileMenu->addMenuItem(TRANS("Play All"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuOpenUnconditionnal, this, _1,
				file.getFullPathName()), playAllImage.get());
		m_fileMenu->addMenuItem(TRANS("Add All"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuQueue, this, _1,
				file.getFullPathName()), addAllImage.get());

		m_fileMenu->listFiles(item, file, std::bind(&VideoComponent::onMenuOpenFile, this, _1, _2),
								std::bind(&VideoComponent::onMenuOpenFolder, this, _1, _2));

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			m_fileMenu->addMenuItem(TRANS("Add to favorites"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAddFavorite, this, _1,
				file.getFullPathName()), likeAddImage.get());
		}
		else
		{
			m_fileMenu->addMenuItem(TRANS("Remove from favorites"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRemoveFavorite, this, _1,
				file.getFullPathName()), likeRemoveImage.get());
		}
	}
}

void VideoComponent::onMenuOpenFile (AbstractMenuItem& item, juce::File file)
{
	if(!file.isDirectory())
	{
		if(extensionMatch(Extensions::get().subtitlesExtensions(), file))
		{
			if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
			vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
		}
		else
		{
			onMenuOpenUnconditionnal(item, file.getFullPathName());
		}
	}
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
		m_optionsMenu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onVLCOptionIntSelect, this, _1, name, it->first), it->first==vlc->getConfigOptionInt(name.c_str())?getItemImage():nullptr);
	}

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
		m_optionsMenu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onVLCOptionStringSelect, this, _1, name, it->first), it->first==vlc->getConfigOptionString(name.c_str())?getItemImage():nullptr);
	}

}
void setVoutOptionInt(VLCWrapper * vlc, std::string option, double value)
{
	vlc->setVoutOptionInt(option.c_str(), (int)value);

}
void VideoComponent::onMenuVoutIntOption (AbstractMenuItem& item, juce::String label, std::string option, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(label,
		std::bind<void>(&::setVoutOptionInt, vlc.get(), option, _1), value, resetValue, volumeMin, volumeMax, step, buttonsStep);
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
		if(invokeLater)invokeLater->queuef(std::bind<void>(&VideoComponent::handleIdleTimeAndControlsVisibility, this));
	}

}
void VideoComponent::onMenuSubtitlePosition(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	m_optionsMenu->addMenuItem( TRANS("Automatic"), AbstractMenuItem::REFRESH_MENU,
		[this](AbstractMenuItem& item){this->onMenuSubtitlePositionMode(item, true);},
		m_autoSubtitlesHeight?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( TRANS("Custom"), AbstractMenuItem::REFRESH_MENU,
		[this](AbstractMenuItem& item){this->onMenuSubtitlePositionMode(item, false);},
		(!m_autoSubtitlesHeight)?getItemImage():nullptr);
	//m_optionsMenu->addMenuItem( TRANS("Setup"), std::bind(&VideoComponent::onMenuVoutIntOption, this, _1,
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

    juce::CallOutBox callOut(colourSelector, m_optionsMenu->asComponent()->getBounds(), this);
    callOut.runModalLoop();

	int newCol = ARGB2RGB(colourSelector.getCurrentColour().getPixelARGB().getInARGBMemoryOrder());
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
	slider.setSize(getWidth()/2, (int)m_optionsMenu->getItemHeight());

	juce::Rectangle<int> componentParentBounds(m_optionsMenu->asComponent()->getBounds());
	componentParentBounds.setTop(0);
	componentParentBounds.setHeight(getHeight());

    juce::CallOutBox callOut(slider, componentParentBounds, this);
    callOut.runModalLoop();

	vlc->setConfigOptionInt(attr.c_str(), (int)slider.getValue());
	m_settings.setValue(attr.c_str(), (int)slider.getValue());
}

void VideoComponent::onMenuSubtitleMenu(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	std::vector<std::pair<int, std::string> > subs = vlc->getSubtitles();
	int current = vlc->getCurrentSubtitleIndex();
	if(!subs.empty())
	{
		for(std::vector<std::pair<int, std::string> >::const_iterator i = subs.begin();i != subs.end();++i)
		{
			m_optionsMenu->addMenuItem( juce::String("-> ") + i->second.c_str(),
                     AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, i->first), i->first==current?getItemImage():nullptr);
		}
	}
	else
	{
		m_optionsMenu->addMenuItem( juce::String::formatted(TRANS("No subtitles")), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSubtitleSelect, this, _1, -1), 0==current?getItemImage():nullptr);
	}
	m_optionsMenu->addMenuItem( TRANS("Add..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		std::bind(&VideoComponent::onMenuLoadSubtitle, this, _1,
				  [this](auto& item, auto const& file){this->onMenuOpenSubtitleFolder(item, file);}));
	//m_optionsMenu->addMenuItem( TRANS("opensubtitles.org"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSearchOpenSubtitles, this, _1));
	m_optionsMenu->addMenuItem( TRANS("SubtitleSeeker.com"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, [this](AbstractMenuItem& item){this->onMenuSearchSubtitleSeeker(item);});
	m_optionsMenu->addMenuItem( TRANS("Delay"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuShiftSubtitlesSlider, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Position"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSubtitlePosition, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_SIZE)));
	m_optionsMenu->addMenuItem( TRANS("Opacity"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OPACITY), "Opacity: %.0f",0, 255, 255));
	m_optionsMenu->addMenuItem( TRANS("Color"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_COLOR)));
	m_optionsMenu->addMenuItem( TRANS("Outline"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_THICKNESS)));
	m_optionsMenu->addMenuItem( TRANS("Outline opacity"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_OUTLINE_OPACITY), "Opacity: %.0f",0, 255, 255));
	m_optionsMenu->addMenuItem( TRANS("Outline Color"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_OUTLINE_COLOR)));
	m_optionsMenu->addMenuItem( TRANS("Background opacity"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_BACKGROUND_OPACITY), "Opacity: %.0f",0, 255, 0));
	m_optionsMenu->addMenuItem( TRANS("Background Color"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_BACKGROUND_COLOR)));
	m_optionsMenu->addMenuItem( TRANS("Shadow opacity"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionIntRangeMenu, this, _1, std::string(CONFIG_INT_OPTION_SUBTITLE_SHADOW_OPACITY), "Opacity: %.0f",0, 255, 0));
	m_optionsMenu->addMenuItem( TRANS("Shadow Color"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onVLCOptionColor, this, _1, std::string(CONFIG_COLOR_OPTION_SUBTITLE_SHADOW_COLOR)));

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
	editor.setSize(3*(int)getWidth() / 4, 2*(int)m_optionsMenu->getFontHeight());

    juce::CallOutBox callOut(editor, m_optionsMenu->asComponent()->getBounds(), this);

	//relayout before callout window
	m_optionsMenu->forceMenuRefresh();

	callOut.runModalLoop();

	onMenuSearchOpenSubtitles(item, lang, editor.getText());
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


#define addItem

void VideoComponent::onMenuSearchOpenSubtitlesSelectLanguage(AbstractMenuItem& item, juce::String movieName)
{
	applyOnAllSubtitleLanguages([&](const char* shortName, const char* ui, const char* label)
	{
		m_optionsMenu->addMenuItem( label, AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		  [this, shortName, movieName](AbstractMenuItem& item){this->onMenuSearchOpenSubtitles(item, shortName, movieName);});
	});
}
void VideoComponent::onMenuSearchOpenSubtitles(AbstractMenuItem& item, juce::String lang, juce::String movieName)
{
	setBrowsingFiles(false);
	movieName = movieName.replace("%", "%37");
	movieName = movieName.replace(" ", "-");
	movieName = movieName.replace("_", "-");
	movieName = movieName.replace(".", "-");
	std::string language=std::string(lang.toUTF8().getAddress());
	std::string name = std::format("http://www.opensubtitles.org/en/search/sublanguageid-{}/moviename-{}/simplexml",language,std::string(movieName.toUTF8().getAddress()) );
	juce::URL url(name.c_str());


	std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		m_optionsMenu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU,
			[this, lang, movieName](AbstractMenuItem& item){this->onMenuSearchOpenSubtitles(item, lang, movieName);});
		return;
	}
	std::unique_ptr<juce::XmlElement> e(juce::XmlDocument::parse(pIStream->readEntireStreamAsString()));
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
					juce::String name = sub->getChildElementAllSubText("releasename", {});
					juce::String downloadURL = sub->getChildElementAllSubText("download", {});
					juce::String language = sub->getChildElementAllSubText("language", {});
					juce::String user = sub->getChildElementAllSubText("user", {});
					if(!name.isEmpty() && !downloadURL.isEmpty())
					{
						m_optionsMenu->addMenuItem(name + juce::String(" by ") + user + juce::String(" (") + language + juce::String(")"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuDowloadOpenSubtitle, this, _1, downloadURL));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}


	//m_optionsMenu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	m_optionsMenu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU,
		[this, lang, movieName](AbstractMenuItem& item){this->onMenuSearchOpenSubtitles(item, lang, movieName);});
}
bool isTVEpisode(juce::String str, int &season, int& episode)
{
#define EPISODE_NAME_PATTERN "\\+s([0-9]+)e([0-9]+)\\+"
    std::cmatch matchesSubscene;
    if( std::regex_search(str.toRawUTF8(), matchesSubscene, std::regex(EPISODE_NAME_PATTERN, std::regex::icase)) )
    {
        season = std::stoi(matchesSubscene[1]);
        episode = std::stoi(matchesSubscene[2]);
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
	std::string name = std::format("http://api.subtitleseeker.com/search/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&q={}&max_results=100",std::string(movieName.toUTF8().getAddress()) );
	int season;
	int episode;
	bool tvEpisode = isTVEpisode(movieName, season, episode);
    if(tvEpisode)
    {
        name+=SUBTITLESEEKER_TV_EPISODE_OPTION;
    }
	juce::URL url(name.c_str());


	std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		m_optionsMenu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU,
			[this, movieName](AbstractMenuItem& item){this->onMenuSearchSubtitleSeeker(item, movieName);});
		return;
	}
	juce::String content = pIStream->readEntireStreamAsString();
	content = content.replace("&", "n");//xml would be invalid otherwise
	std::unique_ptr<juce::XmlElement> e(juce::XmlDocument::parse(content));
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
					juce::String name = sub->getChildElementAllSubText("title", {});
					juce::String year = sub->getChildElementAllSubText("year", {});
					juce::String imdb = sub->getChildElementAllSubText("imdb", {});
					if(!name.isEmpty())
					{
                        juce::String SEsuffix;
					    if(tvEpisode)
                        {
                            juce::String seasonStr = sub->getChildElementAllSubText("season", {});
                            juce::String episodeStr = sub->getChildElementAllSubText("episode", {});
//                            if(!seasonStr.isEmpty() && season!=boost::lexical_cast<int>( seasonStr ) &&
//                                !episodeStr.isEmpty() && episode!=boost::lexical_cast<int>( episodeStr ) )
//                            {
//                                continue;
//                            }
                            SEsuffix += " S"+seasonStr+"E"+episodeStr;
                        }
						m_optionsMenu->addMenuItem(name + SEsuffix + juce::String(" (") + year + juce::String(")"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1, imdb, tvEpisode, season, episode));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}


	//m_optionsMenu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	m_optionsMenu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU,
		[this, movieName](AbstractMenuItem& item){this->onMenuSearchSubtitleSeeker(item, movieName);});
}

void VideoComponent::onMenuSearchSubtitleSeekerImdb(AbstractMenuItem& item, juce::String imdb, bool tvEpisode, int season, int episode)
{
	setBrowsingFiles(false);
	std::string name = std::format("http://api.subtitleseeker.com/get/title_languages/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&imdb={}",imdb.toUTF8().getAddress());
    juce::URL url(name.c_str());


	std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		m_optionsMenu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1 ,  imdb, tvEpisode, season, episode));
		return;
	}
	std::unique_ptr<juce::XmlElement> e(juce::XmlDocument::parse(pIStream->readEntireStreamAsString()));
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
					juce::String lang = sub->getChildElementAllSubText("lang", {});
					if(!lang.isEmpty())
					{
						m_optionsMenu->addMenuItem(lang, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1, imdb ,lang, tvEpisode, season, episode));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}


	//m_optionsMenu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	m_optionsMenu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdb, this, _1 , imdb, tvEpisode, season, episode));
}
void VideoComponent::onMenuSearchSubtitleSeekerImdbLang(AbstractMenuItem& item, juce::String imdb, juce::String lang, bool tvEpisode, int season, int episode)
{
	setBrowsingFiles(false);
	std::string name = std::format("http://api.subtitleseeker.com/get/title_subtitles/?api_key=d24dcf4eeff7709e62e89385334da2b690da5bf4&imdb={}&language={}",imdb.toUTF8().getAddress(),lang.toUTF8().getAddress());
    if(tvEpisode)
    {
        name+=std::format("{}&season={}&episode={}",SUBTITLESEEKER_TV_EPISODE_OPTION,season,episode);
    }
    juce::URL url(name.c_str());


	std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 10000, 0));
	if(!pIStream.get())
	{
		m_optionsMenu->addMenuItem( TRANS("Network error, Retry..."), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1 ,  imdb, lang, tvEpisode, season, episode));
		return;
	}
	std::unique_ptr<juce::XmlElement> e(juce::XmlDocument::parse(pIStream->readEntireStreamAsString()));
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
					juce::String release = sub->getChildElementAllSubText("release", {});
					juce::String site = sub->getChildElementAllSubText("site", {});
					juce::String url = sub->getChildElementAllSubText("url", {});
					if(!lang.isEmpty())
					{
						m_optionsMenu->addMenuItem(site + ": " + release, AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuDowloadSubtitleSeeker, this, _1, url, site));
					}
					sub = sub->getNextElement();
				}
				while(sub);
			}
		}
	}


	//m_optionsMenu->addMenuItem( TRANS("Manual search..."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSearchSubtitlesManually, this, _1, lang), getItemImage());
	m_optionsMenu->addMenuItem( TRANS("Retry..."), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSearchSubtitleSeekerImdbLang, this, _1 ,  imdb, lang, tvEpisode, season, episode));
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
        std::regex expressionSubscene(match, std::regex::icase);
        std::cmatch matchesSubscene;
        if(std::regex_search(cstr, matchesSubscene, expressionSubscene))
        {

            juce::String downloadURL( std::vformat(downloadURLPattern.c_str(),std::make_format_args(matchesSubscene[1].str())).c_str() );
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
    std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", SUBTITLE_DOWNLOAD_TIMEOUT_MS));
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
            std::regex expression(ex, std::regex::icase);

           std::cmatch matches;
           if(std::regex_search((char*)memStream.getData(), matches, expression))
           {
                juce::String otherStr(matches[1].str().c_str());

                if(downloadedSubtitleSeekerResult(item, site, otherStr.getCharPointer().getAddress(), "Opensubtitles.org",
                                                                        "/subtitles/([^/]*)/",
                                                                        "http://dl.opensubtitles.org/en/download/sub/{}"))
                {
                    return;
                }
                //http://subsmax.com/subtitles-movie/treme-2010-dizicd-23-976fps-en-301kb-english-subtitle-zip/3821804
               //download other site page
                juce::URL other(otherStr);
                std::unique_ptr<juce::InputStream> pIStreamOther(other.createInputStream(false, 0, 0, "", SUBTITLE_DOWNLOAD_TIMEOUT_MS));
                if(pIStreamOther.get())
                {
                    memStream.reset();
                    if(memStream.writeFromInputStream(*pIStreamOther, MAX_SUBTITLE_ARCHIVE_SIZE)>0)
                    {
                        memStream.writeByte(0);//simulate end of c string

                        if(downloadedSubtitleSeekerResult(item, site, (char*)memStream.getData(), "Podnapisi.net",
                                                            "<a[^>]*class=\"button big download\"[^>]*href=\"([^\"]*)\"[^>]*>",
                                                            "http://www.podnapisi.net{}"))
                        {

                            //second level: "<a href='([^'])'>here</a>" --> "http://www.podnapisi.net%s"))
                            return;
                        }
                        if(downloadedSubtitleSeekerResult(item, site, (char*)memStream.getData(), "Subscene.com",
                                                            "<a.*href=\"([^\"]*)\".*id=\"downloadButton\".*>",
                                                            "http://subscene.com{}}"))
                        {
                            return;
                        }
                        //need to look at iframe target before inspecting some pages:
                        //	<iframe width="100%" height="9000px" frameborder="0" marginheight="0" marginwidth="0" scrolling="no" src="http://www.engsub.net/NNNNNNNN/">
                        if(downloadedSubtitleSeekerResult(item, site, (char*)memStream.getData(), "Undertexter.se",
                                                            "<a[^>]*title=\"Download subtitle to[^\"][^>]*\".*href=\"([^\"]*)\".*>",
                                                            "{}}"))
                        {
                            return;
                        }
                    }
                }
                //found other site page but could not go further us it at least
                juce::Process::openDocument(otherStr,{});
                return;
           }

        }
    }
    juce::Process::openDocument(downloadUrl,{});
}
void VideoComponent::onMenuDowloadOpenSubtitle(AbstractMenuItem& item, juce::String downloadUrl)
{
	juce::URL url(downloadUrl);


	juce::StringPairArray response;
	std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", SUBTITLE_DOWNLOAD_TIMEOUT_MS, &response));
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
                onMenuOpenSubtitleFile(item,out.getChildFile(entriesToExtract.getFirst()->filename));

            }
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
		setBrowsingFiles(true);
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		juce::Array<juce::File> destArray;
		file.findChildFiles(destArray, juce::File::findDirectories|juce::File::ignoreHiddenFiles, false, "*");
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			m_optionsMenu->addMenuItem( name(file), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuOpenSubtitleFolder, this, _1, file), folderImage.get());
		}


		destArray.clear();
		file.findChildFiles(destArray, juce::File::findFiles|juce::File::ignoreHiddenFiles, false);
		FileSorter sorter(Extensions::get().subtitlesExtensions());
		destArray.sort(sorter);
		for(int i=0;i<destArray.size();++i)
		{
			juce::File const& file(destArray[i]);
			m_optionsMenu->addMenuItem( name(file), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuOpenSubtitleFile, this, _1, file), extensionMatch(Extensions::get().subtitlesExtensions(), destArray[i])?subtitlesImage.get():nullptr);//getIcon(destArray[i]));//
		}
	}
}
void VideoComponent::onMenuOpenSubtitleFile (AbstractMenuItem& item, juce::File file)
{
	if(!file.isDirectory())
	{
		if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
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

	//m_optionsMenu->addMenuItem( TRANS("Auto"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAutoCrop, this, _1), vlc->isAutoCrop()?getItemImage():nullptr);
	std::string current = vlc->getCrop();
	std::vector<std::string> list = vlc->getCropList();
	for(std::vector<std::string>::const_iterator it = list.begin();it != list.end();++it)
	{
		juce::String ratio(it->c_str());
		m_optionsMenu->addMenuItem( ratio.isEmpty()?TRANS("Original"):ratio, AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuCrop, this, _1, ratio), *it==current?getItemImage():nullptr);
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

	m_optionsMenu->addMenuItem( "50%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 50.), 50==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "100%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 100.), 100==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "125%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 125.), 125==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "150%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 150.), 150==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "200%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 200.), 200==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "300%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 300.), 300==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "400%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 400.), 400==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "600%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 600.), 600==(int)(vlc->getRate())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "800%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuRate, this, _1, 800.), 800==(int)(vlc->getRate())?getItemImage():nullptr);

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
	m_optionsMenu->addMenuItem( TRANS("Disable"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onVLCAoutStringSelect, this, _1, filter, name, std::string("")), current.empty()?getItemImage():nullptr);

	std::pair<std::string, std::vector<std::pair<std::string, std::string> > > res = vlc->getConfigOptionInfoString(name.c_str());
	for(std::vector<std::pair<std::string, std::string> >::const_iterator it = res.second.begin();it != res.second.end();++it)
	{
		m_optionsMenu->addMenuItem( TRANS(it->second.c_str()), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onVLCAoutStringSelect, this, _1, filter, name, it->first), it->first==current?getItemImage():nullptr);
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

	m_optionsMenu->addMenuItem( "10%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 10.), 10==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "25%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 25.), 25==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "50%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 50.), 50==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "75%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 75.), 75==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "100%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 100.), 100==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "125%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 125.), 125==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "150%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 150.), 150==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "175%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 175.), 175==(int)(vlc->getVolume())?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "200%", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioVolume, this, _1, 200.), 200==(int)(vlc->getVolume())?getItemImage():nullptr);
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
		m_optionsMenu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuAudioTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
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
		std::bind<void>(&VLCWrapper::setVideoContrast, vlc.get(), _1),
		vlc->getVideoContrast(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoBrightness (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Brightness: %+.3f"),
		std::bind<void>(&VLCWrapper::setVideoBrightness, vlc.get(), _1),
		vlc->getVideoBrightness(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoHue (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Hue"),
		std::bind<void>(&VLCWrapper::setVideoHue, vlc.get(), _1),
		vlc->getVideoHue(), vlc->getVideoHue(), 0, 256., .1);
}
void  VideoComponent::onMenuVideoSaturation (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Saturation: %+.3f"),
		std::bind<void>(&VLCWrapper::setVideoSaturation, vlc.get(), _1),
		vlc->getVideoSaturation(), 1., 0., 2., .01);
}
void  VideoComponent::onMenuVideoGamma (AbstractMenuItem& item)
{
	setBrowsingFiles(false);
	controlComponent->auxilliaryControlComponent().show(TRANS("Gamma: %+.3f"),
		std::bind<void>(&VLCWrapper::setVideoGamma, vlc.get(), _1),
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
		m_optionsMenu->addMenuItem(it->second.c_str(), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuVideoTrack, this, _1, it->first), it->first==current?getItemImage():nullptr);
	}
}
void VideoComponent::onVLCAudioChannelSelect(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	VLCWrapper::AudioChannel c = vlc->getAudioChannel();

	m_optionsMenu->addMenuItem(TRANS("Stereo"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc.get(), VLCWrapper::VLCWrapperAudioChannel_Stereo), c==VLCWrapper::VLCWrapperAudioChannel_Stereo?getItemImage():nullptr);
	m_optionsMenu->addMenuItem(TRANS("Reverse"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc.get(), VLCWrapper::VLCWrapperAudioChannel_RStereo), c==VLCWrapper::VLCWrapperAudioChannel_RStereo?getItemImage():nullptr);
	m_optionsMenu->addMenuItem(TRANS("Left"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc.get(), VLCWrapper::VLCWrapperAudioChannel_Left), c==VLCWrapper::VLCWrapperAudioChannel_Left?getItemImage():nullptr);
	m_optionsMenu->addMenuItem(TRANS("Right"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc.get(), VLCWrapper::VLCWrapperAudioChannel_Right), c==VLCWrapper::VLCWrapperAudioChannel_Right?getItemImage():nullptr);
	m_optionsMenu->addMenuItem(TRANS("Dolby"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setAudioChannel, vlc.get(), VLCWrapper::VLCWrapperAudioChannel_Dolbys), c==VLCWrapper::VLCWrapperAudioChannel_Dolbys?getItemImage():nullptr);
}

void VideoComponent::onVLCAudioOutputDeviceSelect(AbstractMenuItem& item, std::string output, std::string device)
{
	setBrowsingFiles(false);
	vlc->setAudioOutputDevice(output, device);
	m_settings.setValue(SETTINGS_AUDIO_OUTPUT, juce::String(output.c_str()));
	m_settings.setValue(SETTINGS_AUDIO_DEVICE, juce::String(device.c_str()));
}
void VideoComponent::onVLCAudioOutputSelect(AbstractMenuItem& item, std::string output, std::vector< std::pair<std::string, std::string> > list)
{
	setBrowsingFiles(false);

	for(std::vector< std::pair<std::string, std::string> >::const_iterator it = list.begin();it != list.end();++it)
	{
		bool selected = m_settings.getValue(SETTINGS_AUDIO_DEVICE)==juce::String(it->second.c_str());
		m_optionsMenu->addMenuItem(it->first, AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onVLCAudioOutputDeviceSelect, this, _1, output, it->second), selected?getItemImage():nullptr);
	}
}
void VideoComponent::onVLCAudioOutputList(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > > list = vlc->getAudioOutputList();

	if(list.size() == 1)
	{
		onVLCAudioOutputSelect(item, list.front().first.second, list.front().second);
	}
	else
	{
		for(std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > >::const_iterator it = list.begin();it != list.end();++it)
		{
			bool selected = m_settings.getValue(SETTINGS_AUDIO_OUTPUT)==juce::String(it->first.second.c_str());
			m_optionsMenu->addMenuItem(it->first.first, AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCAudioOutputSelect, this, _1, it->first.second, it->second), selected?getItemImage():nullptr);
		}
	}
}
void VideoComponent::onMenuSoundOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	m_optionsMenu->addMenuItem( TRANS("Volume"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuAudioVolumeListAndSlider, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Delay"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuShiftAudioSlider, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Equalizer"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCAoutStringSelectListMenu, this, _1, std::string(AOUT_FILTER_EQUALIZER), std::string(CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET)));
	m_optionsMenu->addMenuItem( TRANS("Select Track"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuAudioTrackList, this, _1));
	bool currentStatus=vlc->getConfigOptionBool(CONFIG_STRING_OPTION_AUDIO_OUT);
	m_optionsMenu->addMenuItem( currentStatus?TRANS("Disable"):TRANS("Enable"), AbstractMenuItem::REFRESH_MENU, std::bind(&VLCWrapper::setConfigOptionBool, vlc.get(), CONFIG_STRING_OPTION_AUDIO_OUT, !currentStatus));
	m_optionsMenu->addMenuItem( TRANS("Channel"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCAudioChannelSelect, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Output"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCAudioOutputList, this, _1));
//	m_optionsMenu->addMenuItem( TRANS("Audio visu."), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCOptionStringMenu, this, _1, std::string(CONFIG_STRING_OPTION_AUDIO_VISUAL)));
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

	m_optionsMenu->addMenuItem( TRANS("Original"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("")), current==""?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "1:1", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("1:1")), current=="1:1"?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "4:3", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("4:3")), current=="4:3"?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "16:10", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("16:10")), current=="16:10"?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "16:9", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("16:9")), current=="16:9"?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "2.21:1", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.21:1")), current=="2.21:1"?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "2.35:1", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.35:1")), current=="2.35:1"?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "2.39:1", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("2.39:1")), current=="2.39:1"?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( "5:4", AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuSetAspectRatio, this, _1, juce::String("5:4")), current=="5:4"?getItemImage():nullptr);

}

void VideoComponent::onMenuVideoAdjustOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	m_optionsMenu->addMenuItem( TRANS("Enable"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuVideoAdjust, this, _1), vlc->getVideoAdjust()?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( TRANS("Contrast"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuVideoContrast, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Brightness"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuVideoBrightness, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Saturation"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuVideoSaturation, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Hue"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuVideoHue, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Gamma"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuVideoGamma, this, _1));
}

void VideoComponent::onMenuVideoOptions(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	m_optionsMenu->addMenuItem( TRANS("Speed"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuRateListAndSlider, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Zoom"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuCropList, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Aspect Ratio"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuRatio, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Select Track"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuVideoTrackList, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Adjust"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuVideoAdjustOptions, this, _1));
//m_optionsMenu->addMenuItem( TRANS("Quality"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_QUALITY)));
	m_optionsMenu->addMenuItem( TRANS("Deinterlace"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCOptionIntListMenu, this, _1, std::string(CONFIG_INT_OPTION_VIDEO_DEINTERLACE)));
	m_optionsMenu->addMenuItem( TRANS("Deint. mode"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onVLCOptionStringMenu, this, _1, std::string(CONFIG_STRING_OPTION_VIDEO_DEINTERLACE_MODE)));

}
void VideoComponent::onMenuExit(AbstractMenuItem& item)
{
	m_fileMenu->addMenuItem( TRANS("Confirm Exit"), AbstractMenuItem::EXECUTE_ONLY, std::bind(&VideoComponent::onMenuExitConfirmation, this, _1), getExitImage());
}

void VideoComponent::onMenuExitConfirmation(AbstractMenuItem& item)
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
	//force mouseinput to be set again when the media starts to play
	vlc->setMouseInputCallBack(NULL);
    vlc->SetEventCallBack(this);

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
		m_optionsMenu->addMenuItem(juce::CharPointer_UTF8(it->c_str()), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onPlaylistItem, this, _1, i), i==current?getItemImage():nullptr);
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
		m_optionsMenu->addMenuItem(it->c_str(), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onLanguageSelect, this, _1, *it), (*it==Languages::getInstance().getCurrentLanguage())?getItemImage():nullptr);
	}
}
void VideoComponent::onSetPlayerFonSize(AbstractMenuItem& item, int size)
{
	setBrowsingFiles(false);

	AppProportionnalComponent::setItemHeightPercentageRelativeToScreen(size);

	m_settings.setValue(SETTINGS_FONT_SIZE, size);

	if(invokeLater)invokeLater->queuef(std::bind<void>(&VideoComponent::resized, this));
}
void VideoComponent::onPlayerFonSize(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	for(int i=50;i<=175;i+=25)
	{
		m_optionsMenu->addMenuItem( juce::String::formatted("%d%%", i), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onSetPlayerFonSize, this, _1, i), i==(AppProportionnalComponent::getItemHeightPercentageRelativeToScreen())?getItemImage():nullptr);
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

	m_optionsMenu->addMenuItem( TRANS("FullScreen"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuFullscreen, this, _1, true), isFullScreen()?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( TRANS("Windowed"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onMenuFullscreen, this, _1, false), isFullScreen()?nullptr:getItemImage());

	m_optionsMenu->addMenuItem( TRANS("Language"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onLanguageOptions, this, _1));
	m_optionsMenu->addMenuItem( TRANS("Menu font size"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onPlayerFonSize, this, _1));

	m_optionsMenu->addMenuItem( TRANS("Hardware"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), true), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)?getItemImage():nullptr);
	m_optionsMenu->addMenuItem( TRANS("No hardware"), AbstractMenuItem::REFRESH_MENU, std::bind(&VideoComponent::onSetVLCOption, this, _1, std::string(CONFIG_BOOL_OPTION_HARDWARE), false), vlc->getConfigOptionBool(CONFIG_BOOL_OPTION_HARDWARE)?nullptr:getItemImage());
}
void VideoComponent::onOptionMenuRoot(AbstractMenuItem& item)
{
	setBrowsingFiles(false);

	m_optionsMenu->addMenuItem( TRANS("Now playing"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onShowPlaylist, this, _1), getPlaylistImage());
	m_optionsMenu->addMenuItem( TRANS("Subtitles"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSubtitleMenu, this, _1), getSubtitlesImage());
	m_optionsMenu->addMenuItem( TRANS("Video"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuVideoOptions, this, _1), getDisplayImage());
	m_optionsMenu->addMenuItem( TRANS("Sound"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onMenuSoundOptions, this, _1), getAudioImage());
	m_optionsMenu->addMenuItem( TRANS("Player"), AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&VideoComponent::onPlayerOptions, this, _1), getSettingsImage());

}
////////////////////////////////////////////////////////////
//
// VLC CALLBACKS
//
////////////////////////////////////////////////////////////
void VideoComponent::vlcTimeChanged(int64_t newTime)
{
	if(!vlc)
	{
		return;
	}
	if(!mousehookset)
	{
		mousehookset = vlc->setMouseInputCallBack(this);
	}
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::updateTimeAndSlider,this, newTime));
}

void VideoComponent::updateTimeAndSlider(int64_t newTime)
{
	if(!sliderUpdating)
	{
		videoUpdating = true;
		controlComponent->slider().setValue(newTime*10000./vlc->GetLength(), juce::sendNotificationSync);
		controlComponent->setTime(newTime, vlc->GetLength());
		if(invokeLater)invokeLater->queuef([this]{this->controlComponent->repaint();});
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
		titleBar->setVisible((m_optionsMenu->asComponent()->isVisible() || !isFullScreen()) && showControls);
		titleBar->allowDrag(!isFullScreen());
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
	if(invokeLater)invokeLater->queuef(std::bind(&ControlComponent::showPausedControls,controlComponent.get()));
}
void VideoComponent::vlcStarted()
{
	titleBar->setTitle(vlc->getCurrentPlayListItem());
	if(invokeLater)invokeLater->queuef(std::bind(&ControlComponent::showPlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::startedSynchronous,this));
}
void VideoComponent::vlcStopped()
{
	titleBar->setTitle(std::string());
	if(invokeLater)invokeLater->queuef(std::bind(&ControlComponent::hidePlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::stoppedSynchronous,this));
}

void VideoComponent::vlcPopupCallback(bool rightClick)
{
	//DBG("vlcPopupCallback." << (rightClick?"rightClick":"leftClick") );
	//lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

	//prevent m_optionsMenu to disappear too quickly
	//m_canHideOSD = !rightClick;

	//bool showMenu = rightClick || vlc->isStopped();
	//if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, showMenu));
	//if(invokeLater)invokeLater->queuef(std::bind(&Component::toFront,this, true));
	//if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::handleIdleTimeAndControlsVisibility,this));

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
		if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::handleIdleTimeAndControlsVisibility,this));
	}
}
void VideoComponent::vlcMouseClick(int x, int y, int button)
{
	//DBG ( "vlcMouseClick " );

	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

	switch(button)
	{
		case 1:
			if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::switchPlayPause, this));
		break;
		case 4:
			if(invokeLater)invokeLater->queuef([this]{this->showVolumeSlider(vlc->getVolume()+VOLUME_SCROLL_STEP);});
		break;
		case 5:
			if(invokeLater)invokeLater->queuef([this]{this->showVolumeSlider(vlc->getVolume()-VOLUME_SCROLL_STEP);});
		break;
	}
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
	vlc->setVolume(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));

	vlc->setCrop(m_settings.getValue(SETTINGS_CROP, "").toUTF8().getAddress());

	vlc->setAudioOutputDevice(m_settings.getValue(SETTINGS_AUDIO_OUTPUT).toUTF8().getAddress(), m_settings.getValue(SETTINGS_AUDIO_DEVICE).toUTF8().getAddress());
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
