
#include "VideoComponent.h"
#include "Icons.h"
#include "MenuTree.h"
#include "VLCMenu.h"

namespace
{

	void setIcon(juce::DrawableButton& but, juce::Drawable &im)
	{
		but.setImages(&im);
	}
	
	juce::String getTimeString(int64_t time, int64_t len)
	{
		int h = (int)(time/(1000*60*60) );
		int m = (int)(time/(1000*60) - 60*h );
		int s = (int)(time/(1000) - 60*m - 60*60*h );
	
		int dh = (int)(len/(1000*60*60) );
		int dm = (int)(len/(1000*60) - 60*dh );
		int ds = (int)(len/(1000) - 60*dm - 60*60*dh );
	
		return juce::String::formatted("%02d:%02d:%02d/%02d:%02d:%02d", h, m, s, dh, dm, ds);
	}
}
//////////////////////////////////////////////////////
ControlComponent::ControlComponent()
{
	slider = new juce::Slider("media time");
	slider->setRange(0, 1000);
	slider->setSliderStyle (juce::Slider::LinearBar);
	slider->setAlpha(1.f);
	slider->setOpaque(true);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

	
    playPauseButton = new juce::DrawableButton("playPause", juce::DrawableButton::ImageFitted);
	playPauseButton->setOpaque(false);
    stopButton = new juce::DrawableButton("stop", juce::DrawableButton::ImageFitted);
	stopButton->setOpaque(false);
	
    playImage = juce::Drawable::createFromImageData (play_svg, play_svgSize);
    pauseImage = juce::Drawable::createFromImageData (pause_svg, pause_svgSize);
    stopImage = juce::Drawable::createFromImageData (stop_svg, stop_svgSize);
	
	setIcon(*playPauseButton, *playImage);
	setIcon(*stopButton, *stopImage);


	
	addAndMakeVisible(slider);
    addAndMakeVisible(playPauseButton);
    addAndMakeVisible(stopButton);

	
	setOpaque(false);
}
ControlComponent::~ControlComponent()
{
	slider = nullptr;
	playPauseButton = nullptr;
	stopButton = nullptr;
	playImage = nullptr;
	pauseImage = nullptr;
	stopImage = nullptr;
}
void ControlComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();

	
	int hMargin = 0.025*w;
	int buttonWidth = 0.03*w;
	int sliderHeight = 0.3*h;
	slider->setBounds (hMargin, h-sliderHeight-buttonWidth, w-2*hMargin, sliderHeight);

	playPauseButton->setBounds (hMargin, h-buttonWidth, buttonWidth, buttonWidth);
	stopButton->setBounds (hMargin+buttonWidth, h-buttonWidth, buttonWidth, buttonWidth);
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

void ControlComponent::setTimeString(juce::String const& s)
{
	timeString = s;
}

void ControlComponent::showPausedControls()
{
	setIcon(*playPauseButton, *playImage);
	
	setVisible(true);
}
void ControlComponent::showPlayingControls()
{
	setIcon(*playPauseButton, *pauseImage);
	
	setVisible(true);
}
void ControlComponent::hidePlayingControls()
{
	setVisible(false);
}
//////////////////////////////////////////////////////

OverlayComponent::OverlayComponent()
: controlComponent( new ControlComponent () )
, tree( new MenuTree () )
{
	
    addAndMakeVisible (tree);
    addChildComponent(controlComponent);
}
void OverlayComponent::setScaleComponent(juce::Component* scaleComponent)
{
	tree->setScaleComponent(scaleComponent);
	controlComponent->setScaleComponent(scaleComponent);
}
OverlayComponent::~OverlayComponent()
{
	tree = nullptr;
	controlComponent = nullptr;
}
void OverlayComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();

	
	int hMargin = 0.025*w;
	int treeWidth = w/4;
	int controlHeight = 0.06*w;

    tree->setBounds (w-treeWidth, hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2);
	controlComponent->setBounds (hMargin, h-controlHeight, w-2*hMargin, controlHeight);
}
//////////////////////////////////////////////////////
class EmptyComponent   : public juce::Component
{
public:
	EmptyComponent(const juce::String& componentName):juce::Component(componentName){}
	void paint (juce::Graphics& g){}
};
	
//////////////////////////////////////////////////////
VideoComponent::VideoComponent()
#ifdef BUFFER_DISPLAY
	:img(new juce::Image(juce::Image::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
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


	setOpaque(true);
		
	
	overlayComponent = new OverlayComponent();
	overlayComponent->tree->setItemImage(getItemImage());
	overlayComponent->tree->setFolderImage(getFolderImage());
	overlayComponent->tree->setFolderShortcutImage(getFolderShortcutImage());

	overlayComponent->controlComponent->slider->addListener(this);
	overlayComponent->controlComponent->playPauseButton->addListener(this);
	overlayComponent->controlComponent->stopButton->addListener(this);

	sliderUpdating = false;
	videoUpdating = false;

		
	
    addAndMakeVisible (overlayComponent);
	
	
	//after set Size
	vlc = new VLCWrapper();

#ifdef BUFFER_DISPLAY

	vlc->SetDisplayCallback(this);
#else

	videoComponent = new EmptyComponent("video");
	videoComponent->setOpaque(true);
    videoComponent->addToDesktop(juce::ComponentPeer::windowIsTemporary);  

	

	vlc->SetOutputWindow(videoComponent->getWindowHandle());

#endif

    vlc->SetEventCallBack(this);

	overlayComponent->tree->setRootAction(getVideoRootMenu(*this));
		
}
VideoComponent::~VideoComponent()
{    
	overlayComponent->controlComponent->slider->removeListener(this);
	overlayComponent->controlComponent->playPauseButton->removeListener(this);
	overlayComponent->controlComponent->stopButton->removeListener(this);
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
	stopTimer();
	getPeer()->getComponent().removeComponentListener(this);
	videoComponent = nullptr;
#endif
	overlayComponent = nullptr;
}
	
void VideoComponent::buttonClicked (juce::Button* button)
{
	if(!vlc)
	{
		return;
	}
	if(button == overlayComponent->controlComponent->playPauseButton)
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
	if(button == overlayComponent->controlComponent->stopButton)
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
	g.fillAll (juce::Colours::black);
#endif
	
}
	
void VideoComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();
		
#ifdef BUFFER_DISPLAY
	overlayComponent->setBounds(0, 0, w, h);
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
	int x=overlayComponent->getParentComponent()?0:getScreenX();
	int y=overlayComponent->getParentComponent()?0:getScreenY();
	videoComponent->setBounds(x, y, w, h);
	overlayComponent->setBounds(x, y, w, h);
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
    startTimer(1);
#endif

	play();
}

void VideoComponent::play()
{
	if(!vlc)
	{
		return;
	}
	overlayComponent->controlComponent->slider->setValue(1000, juce::sendNotificationSync);

	vlc->Play();

	overlayComponent->controlComponent->slider->setValue(0);
	
	setIcon(*overlayComponent->controlComponent->playPauseButton, *overlayComponent->controlComponent->pauseImage);
}
	
void VideoComponent::pause()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
	setIcon(*overlayComponent->controlComponent->playPauseButton, *overlayComponent->controlComponent->playImage);
}
void VideoComponent::stop()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
	overlayComponent->controlComponent->slider->setValue(1000, juce::sendNotificationSync);
	vlc->Stop();
	setIcon(*overlayComponent->controlComponent->playPauseButton, *overlayComponent->controlComponent->playImage);
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
void VideoComponent::timerCallback()
{
	if (vlc->isStopping() || vlc->isStopped() )
    {
        videoComponent->setVisible(false);
		addAndMakeVisible (overlayComponent);

		getPeer()->getComponent().removeComponentListener(this);
        return;
    }

	if(vlc->isPlaying() && !videoComponent->isVisible())
	{
        videoComponent->setVisible(true);
		overlayComponent->addToDesktop(juce::ComponentPeer::windowIsTemporary);  

		getPeer()->getComponent().removeComponentListener(this);
		getPeer()->getComponent().addComponentListener(this);
    }
    if(!getPeer()->isMinimised())
	{
      getPeer()->toBehind(videoComponent->getPeer());
    }
    if(getPeer()->isFocused())
	{
      //videoComponent->getPeer()->toFront(false);
      //overlayComponent->toFront(true);
      videoComponent->getPeer()->toBehind(overlayComponent->getPeer());
	  overlayComponent->toFront(true);
    }
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
		vlc->SetTime(overlayComponent->controlComponent->slider->getValue()*vlc->GetLength()/1000.);
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
	//getPeer()->setFullScreen(fs);
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
		overlayComponent->controlComponent->slider->setValue(vlc->GetTime()*1000./vlc->GetLength(), juce::sendNotificationSync);
		overlayComponent->controlComponent->setTimeString(::getTimeString(vlc->GetTime(), vlc->GetLength()));
		vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::repaint,overlayComponent->controlComponent.get()));
		videoUpdating =false;
	}
	
}
void VideoComponent::paused()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::showPausedControls,overlayComponent->controlComponent.get()));
}
void VideoComponent::started()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::showPlayingControls,overlayComponent->controlComponent.get()));
}
void VideoComponent::stopped()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::hidePlayingControls,overlayComponent->controlComponent.get()));
}