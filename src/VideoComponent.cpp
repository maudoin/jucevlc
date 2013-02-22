
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
// 2ND SLIDER
//
////////////////////////////////////////////////////////////
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>

class ActionSlider   : public juce::Slider, public juce::SliderListener
{
public:
    typedef boost::function<void (double)> Functor;
private:
	Functor m_f;
	juce::String m_format;
	void nop(double)
	{
	}
public:
	ActionSlider(juce::String const& name="")
		:juce::Slider(name)
		,m_f(boost::bind(&ActionSlider::nop, this, _1))
	{
		setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
		setSliderStyle (juce::Slider::LinearBar);
		setAlpha(1.f);
		setOpaque(true);
		addListener(this);
	}
	virtual ~ActionSlider(){}
	void resetCallback(){m_f = boost::bind(&ActionSlider::nop, this, _1);}
	void setCallback(Functor f){m_f = f;}
	void setLabelFormat(juce::String const& f){m_format = f;}
	void paint(juce::Graphics& g)
	{		
		juce::Slider::paint(g);
		
		juce::Font f = g.getCurrentFont().withHeight(getHeight());
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);
		g.setColour (juce::Colours::black);
		g.drawFittedText (juce::String::formatted(m_format,getValue()),
							0, 0, getWidth(), getHeight(),
							juce::Justification::centred, 
							1, //1 line
							1.f//no h scale
							);
	}
    virtual void sliderValueChanged (juce::Slider* slider)
	{
		m_f(slider->getValue());
	}
	void setup(juce::String const& label, ActionSlider::Functor f, double value, double volumeMin, double volumeMax, double step)
	{
		setRange(volumeMin, volumeMax, step);
		setValue(value);
		setLabelFormat(label);
		setCallback(f);
	}
};
class AlternateControlComponent   : public juce::Component, public juce::Button::Listener
{
    juce::ScopedPointer<ActionSlider> m_slider;
    juce::ScopedPointer<juce::DrawableButton> m_leftButton;
    juce::ScopedPointer<juce::DrawableButton> m_rightButton;
    juce::ScopedPointer<juce::Drawable> m_leftImage;
    juce::ScopedPointer<juce::Drawable> m_rightImage;
	double m_buttonsStep;
public:
	AlternateControlComponent()
		:m_buttonsStep(0.)
	{
		setOpaque(true);
		m_leftImage = juce::Drawable::createFromImageData (left_svg, left_svgSize);
		m_rightImage = juce::Drawable::createFromImageData (right_svg, right_svgSize);

		m_leftButton = new juce::DrawableButton("m_leftButton", juce::DrawableButton::ImageFitted);
		m_leftButton->setOpaque(false);
		m_leftButton->setImages(m_leftImage);
		m_leftButton->addListener(this);

		m_rightButton = new juce::DrawableButton("m_rightButton", juce::DrawableButton::ImageFitted);
		m_rightButton->setOpaque(false);
		m_rightButton->setImages(m_rightImage);
		m_rightButton->addListener(this);

		m_slider = new ActionSlider("AlternateControlComponentSlider");
		addAndMakeVisible(m_slider);

		addChildComponent(m_leftButton);
		addChildComponent(m_rightButton);

		setOpaque(false);
	}
	virtual ~AlternateControlComponent()
	{
	}
	virtual void resized()
	{
		bool showButtons = m_buttonsStep>0.;
		int buttonSize = showButtons?getHeight():0;

		int leftButtonSize=0;
		if(m_leftButton->isVisible())
		{
			leftButtonSize=buttonSize;
			m_leftButton->setBounds(0, 0, buttonSize, getHeight());
		}
		int rightButtonSize=0;
		if(m_rightButton->isVisible())
		{
			rightButtonSize=buttonSize;
			m_rightButton->setBounds(getWidth()-buttonSize, 0, buttonSize, getHeight());
		}
		m_slider->setBounds(leftButtonSize, 0, getWidth()-leftButtonSize-rightButtonSize, getHeight());
	}
	
	void paint(juce::Graphics& g)
	{
	}
	void show(juce::String const& label, ActionSlider::Functor f, double value, double volumeMin, double volumeMax, double step, double buttonsStep = 0.f)
	{
		m_buttonsStep = buttonsStep;
		bool showButtons = m_buttonsStep>0.;
		m_leftButton->setVisible(showButtons);
		m_rightButton->setVisible(showButtons);
		m_slider->setup(label, f, value, volumeMin, volumeMax, step);
		resized();
		setVisible(true);
	}
	
	void buttonClicked (juce::Button* button)
	{
		double value = m_slider->getValue();
		double min = m_slider->getMinimum();
		double max = m_slider->getMaximum();
		double step = m_slider->getInterval();

		if( button == m_leftButton.get() )
		{
			m_slider->setRange(min-m_buttonsStep, max-m_buttonsStep, step);
			m_slider->setValue(value-m_buttonsStep);
		}
		else if( button == m_rightButton.get() )
		{
			m_slider->setRange(min+m_buttonsStep, max+m_buttonsStep, step);
			m_slider->setValue(value+m_buttonsStep);
		}
	}
};
////////////////////////////////////////////////////////////
//
// CONTROL COMPONENT
//
////////////////////////////////////////////////////////////
ControlComponent::ControlComponent()
{
	m_slider = new juce::Slider("media time");
	m_slider->setRange(0, 1000);
	m_slider->setSliderStyle (juce::Slider::LinearBar);
	m_slider->setAlpha(1.f);
	m_slider->setOpaque(true);
    m_slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

	
	
    m_playImage = juce::Drawable::createFromImageData (play_svg, play_svgSize);
    m_pauseImage = juce::Drawable::createFromImageData (pause_svg, pause_svgSize);
    m_stopImage = juce::Drawable::createFromImageData (stop_svg, stop_svgSize);

    m_playPauseButton = new juce::DrawableButton("playPause", juce::DrawableButton::ImageFitted);
	m_playPauseButton->setOpaque(false);
	m_playPauseButton->setImages(m_playImage);
    m_stopButton = new juce::DrawableButton("stop", juce::DrawableButton::ImageFitted);
	m_stopButton->setOpaque(false);
	m_stopButton->setImages(m_stopImage);
	

	

	m_alternateControlComponent = new AlternateControlComponent();
	
	addAndMakeVisible(m_slider);
    addAndMakeVisible(m_playPauseButton);
    addAndMakeVisible(m_stopButton);
    addChildComponent(m_alternateControlComponent);

	
	setOpaque(false);
}
ControlComponent::~ControlComponent()
{
	m_slider = nullptr;
	m_playPauseButton = nullptr;
	m_stopButton = nullptr;
	m_playImage = nullptr;
	m_pauseImage = nullptr;
	m_stopImage = nullptr;
}
void ControlComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();

	
	int hMargin = 0.025*w;
	int buttonWidth = 0.03*w;
	int sliderHeight = 0.3*h;
	m_slider->setBounds (hMargin, h-sliderHeight-buttonWidth, w-2*hMargin, sliderHeight);

	m_playPauseButton->setBounds (hMargin, h-buttonWidth, buttonWidth, buttonWidth);
	m_stopButton->setBounds (hMargin+buttonWidth, h-buttonWidth, buttonWidth, buttonWidth);

	m_alternateControlComponent->setBounds (hMargin+10*buttonWidth, h-buttonWidth+hMargin/4, w -20*buttonWidth -2* hMargin, buttonWidth-hMargin/2);
}
void ControlComponent::paint(juce::Graphics& g)
{
	int w =  getWidth();
	int h =  getHeight();

	
	int hMargin = 0.025*w;
	int buttonWidth = 0.03*w;
	int sliderHeight = 0.3*h;
	int roundness = hMargin/4;
	
	///////////////// CONTROL ZONE:	
	g.setGradientFill (juce::ColourGradient (juce::Colours::darkgrey.withAlpha(0.5f),
										w/2, h-sliderHeight-buttonWidth-hMargin/2,
										juce::Colours::black,
										w/2, h,
										false));
	g.fillRoundedRectangle(hMargin/2,  h-sliderHeight-buttonWidth-hMargin/2, w-hMargin, sliderHeight+buttonWidth+hMargin/2, roundness);

	g.setGradientFill (juce::ColourGradient (juce::Colours::lightgrey.withAlpha(0.5f),
										w/2, h-sliderHeight-buttonWidth-hMargin/2,
										juce::Colours::black,
										w/2, h-hMargin/2,
										false));
	g.drawRoundedRectangle(hMargin/2,  h-sliderHeight-buttonWidth-hMargin/2, w-hMargin, sliderHeight+buttonWidth+hMargin/2, roundness,2.f);
	
	///////////////// TIME:
	juce::Font f = g.getCurrentFont().withHeight(getFontHeight());
	f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);

	g.setColour (juce::Colours::white);

	

	g.drawFittedText (timeString,
						hMargin+2*buttonWidth, h-buttonWidth, w-2*hMargin-2*buttonWidth, buttonWidth,
						juce::Justification::topRight, 
						1, //1 line
						1.f//no h scale
						);
}

void ControlComponent::setTime(int64_t time, int64_t len)
{
	int h = (int)(time/(1000*60*60) );
	int m = (int)(time/(1000*60) - 60*h );
	int s = (int)(time/(1000) - 60*m - 60*60*h );
	
	int dh = (int)(len/(1000*60*60) );
	int dm = (int)(len/(1000*60) - 60*dh );
	int ds = (int)(len/(1000) - 60*dm - 60*60*dh );
	
	timeString = juce::String::formatted("%02d:%02d:%02d/%02d:%02d:%02d", h, m, s, dh, dm, ds);
}

void ControlComponent::showPausedControls()
{
	m_playPauseButton->setImages(m_playImage);

	setVisible(true);
}
void ControlComponent::showPlayingControls()
{
	m_playPauseButton->setImages(m_pauseImage);
	
	setVisible(true);
}
void ControlComponent::hidePlayingControls()
{
	setVisible(false);
}
////////////////////////////////////////////////////////////
//
// NATIVE VLC COMPONENT
//
////////////////////////////////////////////////////////////
class EmptyComponent   : public juce::Component
{
	VideoComponent &m_video;
public:
	EmptyComponent(VideoComponent &video, const juce::String& componentName)
		:juce::Component(componentName)
		,m_video(video)
	{
		setOpaque(true);
		addToDesktop(juce::ComponentPeer::windowIsTemporary);  
		setMouseClickGrabsKeyboardFocus(false);
	}
	virtual ~EmptyComponent(){}
	void paint (juce::Graphics& g)
	{
	}
    virtual void mouseEnter (const juce::MouseEvent& event)
	{
		//getPeer()->toBehind(m_video.getPeer());
		//m_video.toFront(true);
	}
    virtual void focusGained (FocusChangeType cause)
	{
		//getPeer()->toBehind(m_video.getPeer());
		//m_video.toFront(true);
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
	videoComponent = new EmptyComponent(*this, "video");
	vlc->SetOutputWindow(videoComponent->getWindowHandle());

#endif
	
    vlc->SetEventCallBack(this);

	tree->setRootAction(Action::build(*this, &VideoComponent::getRootITems));
		
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
	videoComponent = nullptr;
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
		vlc->SetTime(controlComponent->slider().getValue()*vlc->GetLength()/1000.);
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
	if(!videoComponent->isVisible())
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
	videoComponent->setBounds(getScreenX(), getScreenY(), w, h);

	if(videoComponent->getPeer() && getPeer())
	{
		if(getPeer())getPeer()->toBehind(videoComponent->getPeer());
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
void VideoComponent::onListFiles(MenuTreeItem& item, AbstractFileAction* fileMethod)
{
	setBrowsingFiles();

	juce::String path = m_settings.getValue(SETTINGS_LAST_OPEN_PATH);
	juce::File f(path);
	if(path.isEmpty() || !f.exists())
	{
		item.focusItemAsMenuShortcut();
		item.addAction( "Favorites", Action::build(*this, &VideoComponent::onListFavorites, fileMethod), getItemImage());
		item.addRootFiles(fileMethod);
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

void VideoComponent::onListFavorites(MenuTreeItem& item, AbstractFileAction* fileMethod)
{
	item.focusItemAsMenuShortcut();
	for(int i=0;i<m_shortcuts.size();++i)
	{
		juce::File path(m_shortcuts[i]);
		item.addFile(path.getVolumeLabel() + "-" + path.getFileName(), path, fileMethod->clone());
	}
}

void VideoComponent::onOpenFiles(MenuTreeItem& item, AbstractFileAction* fileMethod)
{
	onListFiles(item, fileMethod);
}

void VideoComponent::writeFavorites()
{
	juce::File shortcuts(juce::File::getCurrentWorkingDirectory().getChildFile(SHORTCUTS_FILE));
	m_shortcuts.removeDuplicates(true);
	m_shortcuts.removeEmptyStrings();
	shortcuts.replaceWithText(m_shortcuts.joinIntoString("\n"));
}

void VideoComponent::addFavorite(MenuTreeItem& item, juce::String path)
{
	m_shortcuts.add(path);
	writeFavorites();

	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}
void VideoComponent::removeFavorite(MenuTreeItem& item, juce::String path)
{
	m_shortcuts.removeString(path);
	writeFavorites();
	
	if(invokeLater)invokeLater->queuef(boost::bind<void>(&MenuTreeItem::forceParentSelection, &item, true));
}

void VideoComponent::onOpen (MenuTreeItem& item, juce::File const& file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		item.focusItemAsMenuShortcut();
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onOpen), juce::File::findDirectories|juce::File::ignoreHiddenFiles);
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onOpen), juce::File::findFiles|juce::File::ignoreHiddenFiles);

		if(!m_shortcuts.contains(file.getFullPathName()))
		{
			item.addAction("Add to favorites", Action::build(*this, &VideoComponent::addFavorite, 
				file.getFullPathName()), getItemImage());
		}
		else
		{
			item.addAction("Remove from favorites", Action::build(*this, &VideoComponent::removeFavorite, 
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
void VideoComponent::onSubtitleMenu(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	int cnt = vlc->getSubtitlesCount();
	int current = vlc->getCurrentSubtitleIndex();
	item.addAction( juce::String::formatted("No subtitles"), Action::build(*this, &VideoComponent::onSubtitleSelect, -1), -1==current?getItemImage():nullptr);
	for(int i = 0;i<cnt;++i)
	{
		item.addAction( juce::String::formatted("Slot %d", i+1), Action::build(*this, &VideoComponent::onSubtitleSelect, i), i==current?getItemImage():nullptr);
	}
	item.addAction( "Add...", Action::build(*this, &VideoComponent::onListFiles, FileAction::build(*this, &VideoComponent::onOpenSubtitle)));
	item.addAction( "Delay", Action::build(*this, &VideoComponent::onShiftSubtitlesSlider));
}
void VideoComponent::onSubtitleSelect(MenuTreeItem& item, int i)
{
	setBrowsingFiles(false);
	vlc->setSubtitleIndex(i);
}
void VideoComponent::onOpenSubtitle (MenuTreeItem& item, juce::File const& file)
{
	if(file.isDirectory())
	{
		setBrowsingFiles();
		m_settings.setValue(SETTINGS_LAST_OPEN_PATH, file.getFullPathName());

		item.focusItemAsMenuShortcut();
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onOpenSubtitle), juce::File::findDirectories|juce::File::ignoreHiddenFiles);
		item.addChildrenFiles(file, FileAction::build(*this, &VideoComponent::onOpenSubtitle), juce::File::findFiles|juce::File::ignoreHiddenFiles);
	}
	else
	{
		setBrowsingFiles(false);
		vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
	}
}
void VideoComponent::onOpenPlaylist (MenuTreeItem& item, juce::File const& file)
{
}

void VideoComponent::onCrop (MenuTreeItem& item, double ratio)
{
	setBrowsingFiles(false);
	vlc->setScale(ratio);

	showZoomSlider();
}
void VideoComponent::onCropSlider (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	
	showZoomSlider();
	
	item.focusItemAsMenuShortcut();
	item.addAction( "16/10", Action::build(*this, &VideoComponent::onCrop, 100.*16./10.));
	item.addAction( "16/9", Action::build(*this, &VideoComponent::onCrop, 100.*16./9.));
	item.addAction( "4/3", Action::build(*this, &VideoComponent::onCrop, 100.*4./3.));
}
void VideoComponent::onRate (MenuTreeItem& item, double rate)
{
	setBrowsingFiles(false);
	vlc->setRate(rate);

	showPlaybackSpeedSlider();

	item.forceParentSelection();
}
void VideoComponent::onRateSlider (MenuTreeItem& item)
{
	setBrowsingFiles(false);
	
	item.focusItemAsMenuShortcut();
	item.addAction( "50%", Action::build(*this, &VideoComponent::onRate, 50.), 50==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "100%", Action::build(*this, &VideoComponent::onRate, 100.), 100==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "125%", Action::build(*this, &VideoComponent::onRate, 125.), 125==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "150%", Action::build(*this, &VideoComponent::onRate, 150.), 150==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "200%", Action::build(*this, &VideoComponent::onRate, 200.), 200==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "300%", Action::build(*this, &VideoComponent::onRate, 300.), 300==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "400%", Action::build(*this, &VideoComponent::onRate, 400.), 400==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "600%", Action::build(*this, &VideoComponent::onRate, 600.), 600==(int)(vlc->getRate())?getItemImage():nullptr);
	item.addAction( "800%", Action::build(*this, &VideoComponent::onRate, 800.), 800==(int)(vlc->getRate())?getItemImage():nullptr);

}
void VideoComponent::onSetAspectRatio(MenuTreeItem& item, juce::String ratio)
{
	setBrowsingFiles(false);
	vlc->setAspect(ratio.getCharPointer().getAddress());
}
void VideoComponent::onShiftAudio(MenuTreeItem& item, double s)
{
	setBrowsingFiles(false);
	vlc->setAudioDelay((int64_t)(s*1000000.));
}
void VideoComponent::onShiftAudioSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show("Audio offset: %+.3fs",
		boost::bind<void>(&VideoComponent::onShiftAudio, boost::ref(*this), boost::ref(item), _1),
		vlc->getAudioDelay()/1000000., -2., 2., .01, 2.);
}
void VideoComponent::onShiftSubtitles(MenuTreeItem& item, double s)
{
	setBrowsingFiles(false);
	vlc->setSubtitleDelay((int64_t)(s*1000000.));
}
void VideoComponent::onShiftSubtitlesSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	controlComponent->alternateControlComponent().show("Subtitles offset: %+.3fs",
		boost::bind<void>(&VideoComponent::onShiftSubtitles, boost::ref(*this), boost::ref(item), _1),
		vlc->getSubtitleDelay()/1000000., -2., 2., .01, 2.);
}
void VideoComponent::onAudioVolume(MenuTreeItem& item, double volume)
{
	setBrowsingFiles(false);
	vlc->setVolume(volume);

	showVolumeSlider();

	item.forceParentSelection();

	m_settings.setValue(SETTINGS_VOLUME, vlc->getVolume());
}

void VideoComponent::onAudioVolumeSlider(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	showVolumeSlider();
	
	item.focusItemAsMenuShortcut();
	item.addAction( "10%", Action::build(*this, &VideoComponent::onAudioVolume, 10.), 10==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "25%", Action::build(*this, &VideoComponent::onAudioVolume, 25.), 25==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "50%", Action::build(*this, &VideoComponent::onAudioVolume, 50.), 50==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "75%", Action::build(*this, &VideoComponent::onAudioVolume, 75.), 75==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "100%", Action::build(*this, &VideoComponent::onAudioVolume, 100.), 100==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "125%", Action::build(*this, &VideoComponent::onAudioVolume, 125.), 125==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "150%", Action::build(*this, &VideoComponent::onAudioVolume, 150.), 150==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "175%", Action::build(*this, &VideoComponent::onAudioVolume, 175.), 175==(int)(vlc->getVolume())?getItemImage():nullptr);
	item.addAction( "200%", Action::build(*this, &VideoComponent::onAudioVolume, 200.), 200==(int)(vlc->getVolume())?getItemImage():nullptr);
}

void VideoComponent::onFullscreen(MenuTreeItem& item, bool fs)
{
	setBrowsingFiles(false);
	setFullScreen(fs);
	item.forceParentSelection();
}

void VideoComponent::onSoundOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "Volume", Action::build(*this, &VideoComponent::onAudioVolumeSlider));
	item.addAction( "Delay", Action::build(*this, &VideoComponent::onShiftAudioSlider));
}

void VideoComponent::onRatio(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "original", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("")));
	item.addAction( "1:1", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("1:1")));
	item.addAction( "4:3", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("4:3")));
	item.addAction( "16:10", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("16:10")));
	item.addAction( "16:9", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("16:9")));
	item.addAction( "2.21:1", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("2.21:1")));
	item.addAction( "2.35:1", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("2.35:1")));
	item.addAction( "2.39:1", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("2.39:1")));
	item.addAction( "5:4", Action::build(*this, &VideoComponent::onSetAspectRatio, juce::String("5:4")));
	
}
void VideoComponent::onVideoOptions(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "FullScreen", Action::build(*this, &VideoComponent::onFullscreen, true), isFullScreen()?getItemImage():nullptr);
	item.addAction( "Windowed", Action::build(*this, &VideoComponent::onFullscreen, false), isFullScreen()?nullptr:getItemImage());
	item.addAction( "Speed", Action::build(*this, &VideoComponent::onRateSlider));
	item.addAction( "Zoom", Action::build(*this, &VideoComponent::onCropSlider));
	item.addAction( "Aspect Ratio", Action::build(*this, &VideoComponent::onRatio));
}
void VideoComponent::onExit(MenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
void VideoComponent::getRootITems(MenuTreeItem& item)
{
	setBrowsingFiles(false);
	item.focusItemAsMenuShortcut();
	item.addAction( "Open", Action::build(*this, &VideoComponent::onOpenFiles, FileAction::build(*this, &VideoComponent::onOpen)), getFolderShortcutImage());
	item.addAction( "Subtitle", Action::build(*this, &VideoComponent::onSubtitleMenu), getSubtitlesImage());
	item.addAction( "Video options", Action::build(*this, &VideoComponent::onVideoOptions), getDisplayImage());
	item.addAction( "Sound options", Action::build(*this, &VideoComponent::onSoundOptions), getAudioImage());
	item.addAction( "Exit", Action::build(*this, &VideoComponent::onExit), getExitImage());

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
	//tree->setVisible(rightClick);
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
	
	if(!videoComponent->isVisible())
	{		
		setAlpha(1.f);
		setOpaque(false);
		videoComponent->addToDesktop(juce::ComponentPeer::windowIsTemporary); 
		controlComponent->setVisible(true);
		videoComponent->setVisible(true);

		getPeer()->getComponent().removeComponentListener(this);
		getPeer()->getComponent().addComponentListener(this);

		resized();
	}
}
void VideoComponent::stoppedSynchronous()
{
	
	if(videoComponent->isVisible())
	{
		setAlpha(1.f);
		setOpaque(true);
		videoComponent->setVisible(false);
		getPeer()->getComponent().removeComponentListener(this);
	}
}

void VideoComponent::initFromSettings()
{
	setFullScreen(m_settings.getBoolValue(SETTINGS_FULLSCREEN, true));
	vlc->setVolume(m_settings.getDoubleValue(SETTINGS_VOLUME, 100.));
	juce::File shortcuts(juce::File::getCurrentWorkingDirectory().getChildFile(SHORTCUTS_FILE));
	if(shortcuts.exists())
	{
		shortcuts.readLines(m_shortcuts);
	}
}

