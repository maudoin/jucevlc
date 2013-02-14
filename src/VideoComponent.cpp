
#include "VideoComponent.h"
#include "Icons.h"

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


	
	addChildComponent(slider);
    addChildComponent(playPauseButton);
    addChildComponent(stopButton);
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
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	setOpaque(true);
		


	sliderUpdating = false;
	videoUpdating = false;

		
	tree = new VLCMenuTree ();
	tree->getListeners().add(this);
	tree->setOpenCloseButtonsVisible(false);
	tree->setIndentSize(50);
	tree->setOpaque(false);
	
	
	controlComponent = new ControlComponent();
	controlComponent->slider->addListener(this);
	controlComponent->playPauseButton->addListener(this);
	controlComponent->setOpaque(false);
	
    addAndMakeVisible (tree);
    addAndMakeVisible (controlComponent);
	
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
		
}
VideoComponent::~VideoComponent()
{    
	controlComponent->slider->removeListener(this);
	controlComponent->playPauseButton->removeListener(this);
	controlComponent->stopButton->removeListener(this);
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
	tree = nullptr;
}
	
void VideoComponent::buttonClicked (juce::Button* button)
{
	if(!vlc)
	{
		return;
	}
	if(button == controlComponent->playPauseButton)
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
	if(button == controlComponent->stopButton)
	{
		vlc->Stop();
	}
}
void VideoComponent::setScaleComponent(juce::Component* scaleComponent)
{
	tree->setScaleComponent(scaleComponent);
	controlComponent->setScaleComponent(scaleComponent);
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

	
	int hMargin = 0.025*w;
	int treeWidth = w/4;
	int controlHeight = 0.06*w;

		

		
#ifdef BUFFER_DISPLAY
    tree->setBounds (w-treeWidth, hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2);
	controlComponent->setBounds (hMargin, h-controlHeight, w-2*hMargin, controlHeight);
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
	int x=controlComponent->getParentComponent()?0:getScreenX();
	int y=controlComponent->getParentComponent()?0:getScreenY();
    tree->setBounds (x+w-treeWidth, y+hMargin/2,treeWidth, h-controlHeight-hMargin-hMargin/2);
	controlComponent->setBounds (x+hMargin, y+h-controlHeight, w-2*hMargin, controlHeight);
	videoComponent->setBounds(x, y, w, h);

	
	if(videoComponent->getPeer()!=nullptr)
	{
		videoComponent->getPeer()->toFront(false);
		if(controlComponent->getPeer()!=nullptr)
		{
			controlComponent->getPeer()->toBehind(videoComponent->getPeer());
		}
		if(tree->getPeer()!=nullptr)
		{
			tree->getPeer()->toBehind(videoComponent->getPeer());
		}
	}
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
    videoComponent->setBounds(getScreenX(),getScreenY(),getWidth(),getHeight());
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
	controlComponent->slider->setValue(1000, juce::sendNotificationSync);

	vlc->Play();

	controlComponent->slider->setValue(0);
	
	setIcon(*controlComponent->playPauseButton, *controlComponent->pauseImage);
}
	
void VideoComponent::pause()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
	setIcon(*controlComponent->playPauseButton, *controlComponent->playImage);
}
void VideoComponent::stop()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
	controlComponent->slider->setValue(1000, juce::sendNotificationSync);
	vlc->Stop();
	setIcon(*controlComponent->playPauseButton, *controlComponent->playImage);
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
        //stopTimer();
        videoComponent->setVisible(false);
        return;
    }

	if(vlc->isPlaying() && !videoComponent->isVisible())
	{
        videoComponent->setVisible(true);
		tree->addToDesktop(juce::ComponentPeer::windowIsTemporary);  
		controlComponent->addToDesktop(juce::ComponentPeer::windowIsTemporary);  
    }
    if(!getPeer()->isMinimised())
	{
      getPeer()->toBehind(videoComponent->getPeer());
    }
    if(getPeer()->isFocused())
	{
      videoComponent->getPeer()->toFront(false);
      tree->getPeer()->toFront(false);
      controlComponent->getPeer()->toFront(true);
	  //controlComponent->getPeer()->toBehind(videoComponent->getPeer());
      videoComponent->getPeer()->toBehind(tree->getPeer());
      videoComponent->getPeer()->toBehind(controlComponent->getPeer());
	  tree->getPeer()->toFront(true);
	  controlComponent->getPeer()->toFront(true);
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
		vlc->SetTime(controlComponent->slider->getValue()*vlc->GetLength()/1000.);
		sliderUpdating =false;
	}
}
//MenuTreeListener
void VideoComponent::onOpen (juce::File file)
{
	vf::MessageThread::getInstance();
	play(file.getFullPathName().getCharPointer().getAddress());
}
void VideoComponent::onOpenSubtitle (juce::File file)
{
}
void VideoComponent::onOpenPlaylist (juce::File file)
{
}

void VideoComponent::onCrop (float ratio)
{
	vlc->setCrop(ratio);
}
void VideoComponent::onRate (float rate)
{
	vlc->setRate(rate);
}
void VideoComponent::onSetAspectRatio(juce::String ratio)
{
	vlc->setAspect(ratio.getCharPointer().getAddress());
}
void VideoComponent::onShiftAudio(float ms)
{
	vlc->shiftAudio(ms);
}
void VideoComponent::onShiftSubtitles(float ms)
{
	vlc->shiftSubtitles(ms);
}
void VideoComponent::onAudioVolume(int volume)
{
	vlc->SetVolume(volume);
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
		controlComponent->slider->setValue(vlc->GetTime()*1000./vlc->GetLength(), juce::sendNotificationSync);
		controlComponent->setTimeString(::getTimeString(vlc->GetTime(), vlc->GetLength()));
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
}
void VideoComponent::stopped()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&ControlComponent::hidePlayingControls,controlComponent.get()));
}