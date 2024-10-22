
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
#include <variant>

#define DISAPEAR_DELAY_MS 500
#define DISAPEAR_SPEED_MS 500
#define TIME_JUMP 10000
#define VOLUME_SCROLL_STEP 2.


using namespace std::placeholders;

namespace
{

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
	void paint (juce::Graphics&)
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
	std::string m_defaultTitle;
	bool m_allowDrag;
	juce::String m_currentTimeString;
public:
	TitleComponent(juce::Component* componentToMove, std::string const& defaultTitle)
		:juce::Component("Title")
		,m_componentToMove(componentToMove)
		,m_defaultTitle(defaultTitle)
		,m_allowDrag(false)
		,m_currentTimeString()
	{
		setOpaque(false);
	}
	virtual ~TitleComponent() = default;
	void setTitle(std::string const& title)
	{
		m_title=title;
		if (title.empty())
		{
			m_currentTimeString="";
		}
	}
	void allowDrag(bool allow){m_allowDrag=allow;}
    void setTime(juce::int64 time, juce::int64 len)
	{
		juce::int64 current = juce::Time::currentTimeMillis();
		juce::int64 eta = current + len - time;

		m_currentTimeString = juce::Time(current).formatted("%H:%M") + juce::Time(eta).formatted(" -> %H:%M");
	}
	void paint (juce::Graphics& g)
	{
		juce::String title = juce::String::fromUTF8((m_title.empty()? m_defaultTitle:m_title).c_str());
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

		if(!m_title.empty())
		{
			float timeWidth = f.getStringWidthFloat(m_currentTimeString);
			g.drawFittedText (m_currentTimeString,
				getWidth()-timeWidth-4, 2, timeWidth+2, getHeight()-4,
				juce::Justification::centredRight,
				1, //1 line
				1.f//no h scale
				);
		}
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

class FileMenuComponent : public MenuComponent
{
	std::unique_ptr<juce::Drawable> m_appImage;
public:

	FileMenuComponent()
    : m_appImage(juce::Drawable::createFromImageData (Icons::vlc_svg, Icons::vlc_svgSize))
	{}
	virtual ~FileMenuComponent() = default;

	void paint (juce::Graphics& g) final
	{
		float w2 = getWidth()/2.f;
		float h2 = getHeight()/2.f;
		g.fillAll(juce::Colours::black);
		m_appImage->drawWithin(g, {w2, h2, w2, h2}, juce::RectanglePlacement::xRight|juce::RectanglePlacement::yBottom, 1.f);
	}
};

class OptionMenuComponent : public MenuComponent
{
	std::vector<std::unique_ptr<juce::Drawable>> m_appImage;
public:
	using MenuComponent::MenuComponent;
	virtual ~OptionMenuComponent() = default;

	void paint (juce::Graphics& g) final
	{
		float w = (float)getWidth();
		float h = (float)getHeight();
		float const roundness = 0.01f*getParentWidth();
		static const juce::Colour color(uint8(20), uint8(20), uint8(20), 0.9f);
		g.setColour (color);
		g.fillRoundedRectangle(0, 0, w, h, roundness);
	}
};

}
////////////////////////////////////////////////////////////
//
// MAIN COMPONENT
//
////////////////////////////////////////////////////////////
VideoComponent::VideoComponent(const juce::String& commandLine)
#ifdef BUFFER_DISPLAY
	:img(new juce::Image(juce::Image::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
#else
	:juce::Component("JuceVLC")
#endif
	, m_canHideOSD(true)
	, m_backgroundTasks("BG tasks")
	, m_fileMenu(std::make_unique<FileMenuComponent>())
	, m_optionsMenu(std::make_unique<OptionMenuComponent>([this]{this->updateOptionMenuBounds();}))
	, controlComponent(std::make_unique<ControlComponent>())
    , settingsImage    (juce::Drawable::createFromImageData (Icons::settings_svg, Icons::settings_svgSize))
    , openSettingsImage(juce::Drawable::createFromImageData (Icons::settings_open_svg, Icons::settings_open_svgSize))
    , audioImage       (juce::Drawable::createFromImageData (Icons::audio_svg, Icons::audio_svgSize))
{
	Languages::getInstance();

	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	controlComponent->slider().addListener(this);
	controlComponent->playPauseButton().addListener(this);
	controlComponent->stopButton().addListener(this);
	controlComponent->fullscreenButton().addListener(this);
	controlComponent->menuButton().addListener(this);
	controlComponent->volumeButton().addListener(this);
	controlComponent->addMouseListener(this, true);
	controlComponent->setVisible(false);

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
	m_fileMenu->setScaleComponent(this);


    defaultConstrainer.setMinimumSize (100, 100);
	addChildComponent (*(titleBar = std::make_unique<TitleComponent>(this, std::string("JuceVLC feat. VLC ")+vlc->getInfo())));
	titleBar->addMouseListener(this, true);
    addChildComponent (*(resizableBorder = std::make_unique<juce::ResizableBorderComponent>(this, &defaultConstrainer)));

	addKeyListener(this);

    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);

	vlc->SetInputCallBack(this);

	m_videoPlayerEngine = std::make_unique<PlayerMenus>(vlc, this);

	addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar);

	setSize(800, 600);

	m_videoPlayerEngine->initFromSettings();
	setupVolumeSlider(vlc->getVolume());

	m_optionsMenu->addRecentMenuItem("Menu", AbstractMenuItem::STORE_AND_OPEN_CHILDREN, std::bind(&PlayerMenus::onOptionMenuRoot, m_videoPlayerEngine.get(), _1));
	m_optionsMenu->forceMenuRefresh();

	m_fileMenu->addRecentMenuItem(juce::String("JUCE + VLC ") + vlc->getInfo().c_str(), AbstractMenuItem::STORE_AND_OPEN_CHILDREN,
		[this](auto& item){m_videoPlayerEngine->onFileMenuRoot( item, [this](auto& item, auto const& file){m_videoPlayerEngine->onMenuOpenFolder(item, file);});});
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

	if(invokeLater)invokeLater->queuef(std::bind(&ControlComponent::hidePlayingControls,controlComponent.get()));

	m_videoPlayerEngine->mayOpen(*m_fileMenu, commandLine);
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

	m_videoPlayerEngine->saveCurrentMediaTime();
	m_videoPlayerEngine.reset();

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


void VideoComponent::handleFullRelayout()
{
	if(invokeLater)invokeLater->queuef(std::bind<void>(&VideoComponent::resized, this));
}

void VideoComponent::handleControlRelayout()
{
	if(invokeLater)invokeLater->queuef(std::bind<void>(&VideoComponent::handleIdleTimeAndControlsVisibility, this));
}

void VideoComponent::closeFileMenu()
{
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon,this, false));
}

////////////////////////////////////////////////////////////
//
// BG thread
//
////////////////////////////////////////////////////////////

int VideoComponent::useTimeSlice()
{
	if(isFrontpageVisible() != m_fileMenu->isShown())
	{
		if(invokeLater)invokeLater->queuef([this]{this->m_fileMenu->setShown(isFrontpageVisible());});
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
                            juce::Component* /*originatingComponent*/)
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
	if(key.isKeyCurrentlyDown(juce::KeyPress::escapeKey))
	{
		stop();
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
	m_videoPlayerEngine->saveFullscreenState(fs);
}

void VideoComponent::switchFullScreen()
{
	setFullScreen(juce::Desktop::getInstance().getKioskModeComponent() == nullptr);
}
bool VideoComponent::isFrontpageVisible()
{
	return (!vlcNativePopupComponent->isVisible() || vlc->isStopped()) && ! m_optionsMenu->isShown();
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
                                const juce::MouseWheelDetails& /*wheel*/)
{
	if(e.eventComponent == this)
	{
	}
}
void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
	m_canHideOSD = e.eventComponent == this;//cannot hide sub component while dragging on sub component
	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

	if(e.eventComponent == this)
	{
	}
}
void VideoComponent::sliderValueChanged (juce::Slider* slider)
{
	if(vlc && (!videoUpdating) && slider == &controlComponent->slider())
	{
		sliderUpdating = true;
		vlc->SetTime((int64_t)(controlComponent->slider().getValue()*vlc->GetLength()/10000.));
		m_videoPlayerEngine->saveCurrentMediaTime();
		sliderUpdating =false;
	}
}

void VideoComponent::setMenuTreeVisibleAndUpdateMenuButtonIcon(bool visible)
{
	m_optionsMenu->setShown(visible);
	m_fileMenu->setShown(false);
	controlComponent->menuButton().setImages(m_optionsMenu->isShown()?openSettingsImage.get():settingsImage.get());
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
		setMenuTreeVisibleAndUpdateMenuButtonIcon(!m_optionsMenu->isShown());
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
	}
#endif

}

void VideoComponent::updateOptionMenuBounds()
{
	int w =  getWidth();
	int h =  getHeight();

	int optionItemHeight = m_optionsMenu->getItemHeight();
	int hMargin = (int)(optionItemHeight/2.);
	int optionPopupWidth = std::min(m_optionsMenu->preferredWidth(), w/2);
	int controlHeight = 3*(int)optionItemHeight;
	int optionHeight = std::min(h-controlHeight-hMargin, m_optionsMenu->preferredHeight());

	m_optionsMenu->asComponent()->setBounds (w-optionPopupWidth,h-optionHeight-controlHeight,optionPopupWidth, optionHeight);
}

void VideoComponent::updateSubComponentsBounds()
{
	updateOptionMenuBounds();

	int w =  getWidth();
	int h =  getHeight();

	int frontpageMargin = (int)m_optionsMenu->getItemHeight();
    m_fileMenu->asComponent()->setBounds (frontpageMargin, frontpageMargin, w-2*frontpageMargin, h-2*frontpageMargin);

	int controlHeight = 3*(int)m_optionsMenu->getItemHeight();
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
        titleBar->setVisible (m_optionsMenu->isShown() || !isFullScreen());
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
	int time = m_videoPlayerEngine->getMediaSavedTime(name);
	if(time>0)
	{
		forceSetVideoTime(time*1000);
	}
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
	m_videoPlayerEngine->saveCurrentMediaTime();
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
	m_videoPlayerEngine->saveCurrentMediaTime(true);
	vlc->Stop();
	m_fileMenu->forceMenuRefresh();
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

void VideoComponent::componentMovedOrResized(Component &, bool wasMoved, bool wasResized)
{
	if(wasResized)
	{
		resized();
	}
	if(wasMoved)
	{
		vlcNativePopupComponent->setBounds(getScreenX(), getScreenY(), getWidth(), getHeight());
	}
}
void VideoComponent::componentVisibilityChanged(Component &)
{
    resized();
}

#endif

void VideoComponent::setupVolumeSlider(double value)
{
	controlComponent->setupVolumeSlider(
		std::bind<void>(&VLCWrapper::setVolume, vlc.get(), _1),value, 1., 200., .1);
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
	if(!vlc->hasMouseInputCallBack())
	{
		vlc->setMouseInputCallBack(this);
	}
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::updateTimeAndSlider,this, newTime));
}

void VideoComponent::updateTimeAndSlider(int64_t newTime)
{
	if(!sliderUpdating)
	{
		int64_t len = vlc->GetLength();

		videoUpdating = true;
		controlComponent->slider().setValue(newTime*10000./len, juce::sendNotificationSync);

		titleBar->setTime(newTime, len);
		controlComponent->setTime(::toString(newTime) + "/" + ::toString(len));
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
		titleBar->setVisible(showControls);
		titleBar->allowDrag(!isFullScreen());
	}
	else
	{
		//DBG ( (long)timeFromLastMouseMove  << "->" << (long)timeFromLastMouseMove-DISAPEAR_DELAY_MS << "/" << DISAPEAR_SPEED_MS << " -> hide() " );
		setMenuTreeVisibleAndUpdateMenuButtonIcon(false);
		controlComponent->setVisible(false);
		titleBar->setVisible(false);
	}
	if(m_videoPlayerEngine->isAutoSubtitlesHeight())
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
	titleBar->setTitle(juce::URL::removeEscapeChars(vlc->getCurrentPlayListItem().c_str()).toUTF8().getAddress());
	if(invokeLater)invokeLater->queuef(std::bind(&ControlComponent::showPlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::startedSynchronous,this));
}
void VideoComponent::vlcStopped()
{
	titleBar->setTitle(std::string());
	if(invokeLater)invokeLater->queuef(std::bind(&ControlComponent::hidePlayingControls,controlComponent.get()));
	if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::stoppedSynchronous,this));
}

void VideoComponent::vlcPopupCallback(bool /*rightClick*/)
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
void VideoComponent::vlcMouseMove(int /*x*/, int /*y*/, int /*button*/)
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
void VideoComponent::vlcMouseClick(int /*x*/, int /*y*/, int button)
{
	//DBG ( "vlcMouseClick " );

	lastMouseMoveMovieTime = juce::Time::currentTimeMillis ();

	switch(button)
	{
		case 1:
			if(invokeLater)invokeLater->queuef(std::bind(&VideoComponent::switchPlayPause, this));
		break;
		case 4:
			if(invokeLater)invokeLater->queuef([this]{this->setupVolumeSlider(vlc->getVolume()+VOLUME_SCROLL_STEP);});
		break;
		case 5:
			if(invokeLater)invokeLater->queuef([this]{this->setupVolumeSlider(vlc->getVolume()-VOLUME_SCROLL_STEP);});
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
	m_videoPlayerEngine->initFromMediaDependantSettings();
}
void VideoComponent::stoppedSynchronous()
{
	m_videoPlayerEngine->saveCurrentMediaTime(true);
	if(vlcNativePopupComponent->isVisible())
	{
		setAlpha(1.f);
		vlcNativePopupComponent->setVisible(false);
		setMenuTreeVisibleAndUpdateMenuButtonIcon(false);
		getPeer()->getComponent().removeComponentListener(this);
	}
	m_fileMenu->forceMenuRefresh();
}
void VideoComponent::playPlayListItem(int index, std::string const& name)
{
#ifdef BUFFER_DISPLAY
	img = new juce::Image(img->rescaled(getWidth(), getHeight()));
	ptr = new juce::Image::BitmapData (*img, juce::Image::BitmapData::readWrite);
	vlc->SetBufferFormat(img->getWidth(), img->getHeight(), ptr->lineStride);
#else
    resized();
#endif
	//force mouseinput to be set again when the media starts to play
	vlc->setMouseInputCallBack(nullptr);
    vlc->SetEventCallBack(this);

	vlc->playPlayListItem(index);

	forceSetVideoTime(name);
}

