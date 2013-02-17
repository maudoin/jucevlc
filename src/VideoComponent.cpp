
#include "VideoComponent.h"
#include "Icons.h"
#include "MenuTree.h"
#include "VLCMenu.h"

//////////////////////////////////////////////////////
ControlComponent::ControlComponent()
{
	m_slider = new juce::Slider("media time");
	m_slider->setRange(0, 1000);
	m_slider->setSliderStyle (juce::Slider::LinearBar);
	m_slider->setAlpha(1.f);
	m_slider->setOpaque(true);
    m_slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

	
    m_playPauseButton = new juce::DrawableButton("playPause", juce::DrawableButton::ImageFitted);
	m_playPauseButton->setOpaque(false);
    m_stopButton = new juce::DrawableButton("stop", juce::DrawableButton::ImageFitted);
	m_stopButton->setOpaque(false);
	
    m_playImage = juce::Drawable::createFromImageData (play_svg, play_svgSize);
    m_pauseImage = juce::Drawable::createFromImageData (pause_svg, pause_svgSize);
    m_stopImage = juce::Drawable::createFromImageData (stop_svg, stop_svgSize);
	
	m_playPauseButton->setImages(m_playImage);
	m_stopButton->setImages(m_stopImage);


	
	addAndMakeVisible(m_slider);
    addAndMakeVisible(m_playPauseButton);
    addAndMakeVisible(m_stopButton);

	
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

	g.setColour (findColour (juce::DirectoryContentsDisplayComponent::textColourId));

	

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
//////////////////////////////////////////////////////
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
		//setInterceptsMouseClicks(false, false);
	}
	void paint (juce::Graphics& g){}
	/*bool hitTest (int x, int y)
	{
		return false;
	}
	void broughtToFront ()   
	{
		getPeer()->toBehind(m_overlayComponent.getPeer());
		m_overlayComponent.toFront(true);
	}*/
    virtual void mouseMove (const juce::MouseEvent& event)
	{
		getPeer()->toBehind(m_video.getPeer());
		//m_video.toFront(true);
	}
};
	
//////////////////////////////////////////////////////
VideoComponent::VideoComponent()
#ifdef BUFFER_DISPLAY
	:img(new juce::Image(juce::Image::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
#else
#endif
{    

    itemImage = juce::Drawable::createFromImageData (blue_svg, blue_svgSize);
    folderImage = juce::Drawable::createFromImageData (folder_svg, folder_svgSize);
    folderShortcutImage = juce::Drawable::createFromImageData (folderShortcut_svg, folderShortcut_svgSize);
    audioImage = juce::Drawable::createFromImageData (audio_svg, audio_svgSize);
    displayImage = juce::Drawable::createFromImageData (display_svg, display_svgSize);
    subtitlesImage = juce::Drawable::createFromImageData (sub_svg, sub_svgSize);
    exitImage = juce::Drawable::createFromImageData (exit_svg, exit_svgSize);


	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	setOpaque(false);
		
	controlComponent = new ControlComponent ();
    addChildComponent(controlComponent);

	tree = new MenuTree ();
	tree->setItemImage(getItemImage());
	tree->setFolderImage(getFolderImage());
	tree->setFolderShortcutImage(getFolderShortcutImage());
    addAndMakeVisible (tree);

	controlComponent->slider().addListener(this);
	controlComponent->playPauseButton().addListener(this);
	controlComponent->stopButton().addListener(this);

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

	tree->setRootAction(getVideoRootMenu(*this));
		
	////////////////
	tree->setScaleComponent(this);
	controlComponent->setScaleComponent(this);

	
    defaultConstrainer.setMinimumSize (100, 100);
    Component::addChildComponent (resizableBorder = new juce::ResizableBorderComponent (this, &defaultConstrainer));

	addKeyListener(this);
		

    // And show it!
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);
    addToDesktop(juce::ComponentPeer::windowAppearsOnTaskbar|juce::ComponentPeer::windowIsResizable|juce::ComponentPeer::windowIgnoresKeyPresses);  
	setAlwaysOnTop(true);
    setVisible (true);
}

VideoComponent::~VideoComponent()
{    
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

//==============================================================================

void VideoComponent::userTriedToCloseWindow()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
bool VideoComponent::keyPressed (const juce::KeyPress& key,
                            juce::Component* originatingComponent)
{
	if(key.isKeyCurrentlyDown(juce::KeyPress::returnKey) && key.getModifiers().isAltDown())
	{
		vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::switchFullScreen,this));
		return true;
	}
	return false;
	
}

bool VideoComponent::isFullScreen()const
{
	return juce::Desktop::getInstance().getKioskModeComponent() == getTopLevelComponent();
}

void VideoComponent::switchFullScreen()
{
	if (juce::Desktop::getInstance().getKioskModeComponent() == nullptr)
	{
		juce::Desktop::getInstance().setKioskModeComponent (getTopLevelComponent());
	}
	else
	{
		juce::Desktop::getInstance().setKioskModeComponent (nullptr);
	}
}
void VideoComponent::mouseDown (const juce::MouseEvent& e)
{
    dragger.startDraggingComponent (this, e);
}

void VideoComponent::mouseDrag (const juce::MouseEvent& e)
{
    dragger.dragComponent (this, e, 0);
}
//==============================================================================
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
#endif
	
}
	
void VideoComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();
	
    if (resizableBorder != nullptr)
    {
        resizableBorder->setVisible (! (isFullScreen() ));

        resizableBorder->setBorderThickness (juce::BorderSize<int> (2));
        resizableBorder->setSize (w, h);
        resizableBorder->toBack();
    }

	int hMargin = 0.025*w;
	int treeWidth = w/4;
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
	int x=getParentComponent()?0:getScreenX();
	int y=getParentComponent()?0:getScreenY();
	videoComponent->setBounds(x, y, w, h);
#endif
	
}



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

void *VideoComponent::lock(void **p_pixels)
{
	imgCriticalSection.enter();
	if(ptr)
	{
		*p_pixels = ptr->getLinePointer(0);
	}
	return NULL; /* picture identifier, not needed here */
}

void VideoComponent::unlock(void *id, void *const *p_pixels)
{
	imgCriticalSection.exit();

	jassert(id == NULL); /* picture identifier, not needed here */
}

void VideoComponent::display(void *id)
{
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::repaint,this));
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
/*
void VideoComponent::timerCallback()
{
	if (vlc->isStopping() || vlc->isStopped() && videoComponent->isVisible() )
	{
		videoComponent->setVisible(false);

		getPeer()->getComponent().removeComponentListener(this);
		return;
	}

	if(vlc->isPlaying() && !videoComponent->isVisible())
	{
		videoComponent->setVisible(true);

		getPeer()->getComponent().removeComponentListener(this);
		getPeer()->getComponent().addComponentListener(this);
		
		//getPeer()->toBehind(videoComponent->getPeer());
		videoComponent->getPeer()->toBehind(getPeer());
		toFront(true);

		resized();
	}
	if(!getPeer()->isMinimised())
	{
		getPeer()->toBehind(videoComponent->getPeer());
	}
	if(getPeer()->isFocused())
	{
		if(videoComponent->isVisible())
		{
			videoComponent->getPeer()->toBehind(getPeer());
		}
		toFront(true);
	}
}
*/

void VideoComponent::mouseMove (const juce::MouseEvent& event)
{
	getPeer()->toBehind(videoComponent->getPeer());
}
#endif

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
//MenuTreeListener
void VideoComponent::onOpen (MenuTreeItem& item, juce::File const& file)
{
	vf::MessageThread::getInstance();
	play(file.getFullPathName().toUTF8().getAddress());
}
void VideoComponent::onOpenSubtitle (MenuTreeItem& item, juce::File const& file)
{
	vlc->loadSubtitle(file.getFullPathName().toUTF8().getAddress());
}
void VideoComponent::onOpenPlaylist (MenuTreeItem& item, juce::File const& file)
{
}

void VideoComponent::onCrop (MenuTreeItem& item, float ratio)
{
	vlc->setCrop(ratio);
}
void VideoComponent::onRate (MenuTreeItem& item, float rate)
{
	vlc->setRate(rate);
}
void VideoComponent::onSetAspectRatio(MenuTreeItem& item, juce::String ratio)
{
	vlc->setAspect(ratio.getCharPointer().getAddress());
}
void VideoComponent::onShiftAudio(MenuTreeItem& item, float ms)
{
	vlc->shiftAudio(ms);
}
void VideoComponent::onShiftSubtitles(MenuTreeItem& item, float ms)
{
	vlc->shiftSubtitles(ms);
}
void VideoComponent::onAudioVolume(MenuTreeItem& item, int volume)
{
	vlc->SetVolume(volume);
}

void VideoComponent::onFullscreen(MenuTreeItem& item, bool fs)
{
	juce::Desktop::getInstance().setKioskModeComponent (fs?getTopLevelComponent():nullptr);
}
void VideoComponent::timeChanged()
{
	if(!vlc)
	{
		return;
	}
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::updateTimeAndSlider,this));
}
void VideoComponent::updateTimeAndSlider()
{
	if(!sliderUpdating)
	{
		videoUpdating = true;
		controlComponent->slider().setValue(vlc->GetTime()*1000./vlc->GetLength(), juce::sendNotificationSync);
		controlComponent->setTime(vlc->GetTime(), vlc->GetLength());
		vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::repaint,controlComponent.get()));
		videoUpdating =false;
	}
	
}
void VideoComponent::paused()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::showPausedControls,controlComponent.get()));
}
void VideoComponent::started()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::showPlayingControls,controlComponent.get()));
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::startedSynchronous,this));
}
void VideoComponent::stopped()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::hidePlayingControls,controlComponent.get()));
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::stoppedSynchronous,this));
}

void VideoComponent::startedSynchronous()
{
	
	if(!videoComponent->isVisible())
	{
		videoComponent->setVisible(true);

		getPeer()->getComponent().removeComponentListener(this);
		getPeer()->getComponent().addComponentListener(this);
		
		videoComponent->getPeer()->toBehind(getPeer());
		toFront(true);

		resized();
	}
}
void VideoComponent::stoppedSynchronous()
{
	
	if(videoComponent->isVisible())
	{
		videoComponent->setVisible(false);
		getPeer()->getComponent().removeComponentListener(this);
	}
}