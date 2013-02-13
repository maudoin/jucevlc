
#include "VideoComponent.h"
#include "Icons.h"

namespace
{

	void setIcon(juce::DrawableButton& but, juce::Drawable &im)
	{
		but.setImages(&im);
	}
}

VideoComponent::VideoComponent()
#ifdef BUFFER_DISPLAY
	:img(new juce::Image(juce::Image::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
#endif
{    
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	setOpaque(true);
		


	slider = new juce::Slider("media time");
	slider->setRange(0, 1000);
	slider->addListener(this);
	slider->setSliderStyle (juce::Slider::LinearBar);
	slider->setAlpha(1.f);
	slider->setOpaque(true);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

	sliderUpdating = false;
	videoUpdating = false;

		
	tree = new VLCMenuTree ();
	tree->getListeners().add(this);
	tree->setOpenCloseButtonsVisible(false);
	tree->setIndentSize(50);
	
    playPauseButton = new juce::DrawableButton("playPause", juce::DrawableButton::ImageFitted);
	playPauseButton->setOpaque(false);
    stopButton = new juce::DrawableButton("stop", juce::DrawableButton::ImageFitted);
	stopButton->setOpaque(false);
	
    playImage = juce::Drawable::createFromImageData (play_svg, play_svgSize);
    pauseImage = juce::Drawable::createFromImageData (pause_svg, pause_svgSize);
    stopImage = juce::Drawable::createFromImageData (stop_svg, stop_svgSize);
	
	setIcon(*playPauseButton, *playImage);
	setIcon(*stopButton, *stopImage);

	playPauseButton->addListener(this);

    addAndMakeVisible (tree);

	addChildComponent(slider);
    addChildComponent(playPauseButton);
    addChildComponent(stopButton);
		
	setSize (600, 300);
#ifndef BUFFER_DISPLAY
	videoComponent = new juce::Component("video");
	videoComponent->setOpaque(true);
	addChildComponent(videoComponent);
    //videoComponent->addToDesktop(juce::ComponentPeer::windowIsTemporary);

#endif


	//after set Size
	vlc = new VLCWrapper();
#ifdef BUFFER_DISPLAY
	vlc->SetDisplayCallback(this);
#else
	vlc->SetOutputWindow(videoComponent->getWindowHandle());
	//vlc->SetOutputWindow(videoComponent->getPeer()->getNativeHandle());
#endif
    vlc->SetEventCallBack(this);
		
}
VideoComponent::~VideoComponent()
{    
	slider->removeListener(this);
	playPauseButton->removeListener(this);
	stopButton->removeListener(this);
	{
		vlc->SetTime(vlc->GetLength());
		vlc->Pause();
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
		vlc = nullptr;
	}
	slider = nullptr;
	tree = nullptr;;
#ifdef BUFFER_DISPLAY
	ptr = nullptr;
	img = nullptr;
#else
	videoComponent = nullptr;
#endif
	playImage = nullptr;
	pauseImage = nullptr;
	stopImage = nullptr;
	playPauseButton = nullptr;
	stopButton = nullptr;
}
	
void VideoComponent::buttonClicked (juce::Button* button)
{
	if(!vlc)
	{
		return;
	}
	if(button == playPauseButton)
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
	if(button == stopButton)
	{
		vlc->Stop();
	}
}
void VideoComponent::setScaleComponent(juce::Component* scaleComponent)
{
	tree->setScaleComponent(scaleComponent);
}
void VideoComponent::paint (juce::Graphics& g)
{
	
	int w =  getWidth();
	int h =  getHeight();
	

	int hMargin = 0.025*w;
	int treeWidth = w/4;
	int buttonWidth = 0.03*w;
	int sliderHeight = 0.025*h;
	int roundness = hMargin/4;

	///////////////// VIDEO:
	
#ifdef BUFFER_DISPLAY
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
	{
		g.drawImage(*img, 0, 0, getWidth(), getHeight(), 0, 0, img->getWidth(), img->getHeight());
	}
#else
    g.fillAll (juce::Colours::black);
#endif
	
	if(!vlc || vlc->isPaused())
	{
		return;
	}
	
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
	juce::Font f = g.getCurrentFont().withHeight(tree->getFontHeight());
	f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);

	g.setColour (findColour (juce::DirectoryContentsDisplayComponent::textColourId));

	

	g.drawFittedText (getTimeString(),
						hMargin+2*buttonWidth, h-buttonWidth, w-2*hMargin-2*buttonWidth, buttonWidth,
						juce::Justification::topRight, 
						1, //1 line
						1.f//no h scale
						);
}
	
void VideoComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();

	
	int hMargin = 0.025*w;
	int treeWidth = w/4;
	int buttonWidth = 0.03*w;
	int sliderHeight = 0.025*h;

    tree->setBounds (w-treeWidth, hMargin/2,treeWidth, h-sliderHeight-buttonWidth-hMargin-hMargin/2);
	
		
	slider->setBounds (hMargin, h-sliderHeight-buttonWidth, w-2*hMargin, sliderHeight);

	playPauseButton->setBounds (hMargin, h-buttonWidth, buttonWidth, buttonWidth);
	stopButton->setBounds (hMargin+buttonWidth, h-buttonWidth, buttonWidth, buttonWidth);

	if(vlc)
	{
		//rebuild buffer
		bool restart(vlc->isPaused());
		vlc->Pause();
		
#ifdef BUFFER_DISPLAY
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
#else
		videoComponent->setBounds(getScreenX(), getScreenY(), w, h);
#endif
	}
	
}


void VideoComponent::stop()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
	slider->setValue(1000, juce::sendNotificationSync);
	vlc->Stop();
	setIcon(*playPauseButton, *playImage);
}


void VideoComponent::pause()
{
	if(!vlc)
	{
		return;
	}
	vlc->Pause();
	setIcon(*playPauseButton, *playImage);
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
	videoComponent->getPeer()->toFront(false);
#endif

	play();
}


void VideoComponent::play()
{
	if(!vlc)
	{
		return;
	}
	slider->setValue(1000, juce::sendNotificationSync);

	vlc->Play();

	slider->setValue(0);
	
	setIcon(*playPauseButton, *pauseImage);
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
		vlc->SetTime(slider->getValue()*vlc->GetLength()/1000.);
		sliderUpdating =false;
	}
}
juce::Slider* VideoComponent::getSlider()
{
	return slider.get();
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
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::updateTimeAndSlider,this));
}
void VideoComponent::updateTimeAndSlider()
{
	if(!vlc)
	{
		return;
	}
	if(!sliderUpdating)
	{
		videoUpdating = true;
		slider->setValue(vlc->GetTime()*1000./vlc->GetLength(), juce::sendNotificationSync);
		videoUpdating =false;
	}
	
}
juce::String VideoComponent::getTimeString()const
{
	int64_t time = vlc->GetTime();
	int h = (int)(time/(1000*60*60) );
	int m = (int)(time/(1000*60) - 60*h );
	int s = (int)(time/(1000) - 60*m - 60*60*h );
	
	int64_t len = vlc->GetLength();
	int dh = (int)(len/(1000*60*60) );
	int dm = (int)(len/(1000*60) - 60*dh );
	int ds = (int)(len/(1000) - 60*dm - 60*60*dh );
	
	return juce::String::formatted("%02d:%02d:%02d/%02d:%02d:%02d", h, m, s, dh, dm, ds);
}
void VideoComponent::paused()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::showPausedControls,this));
}
void VideoComponent::showPausedControls()
{
	setIcon(*playPauseButton, *playImage);

	slider->setVisible(true);
	playPauseButton->setVisible(true);
	stopButton->setVisible(true);
}
void VideoComponent::started()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::showPlayingControls,this));
}
void VideoComponent::showPlayingControls()
{
	setIcon(*playPauseButton, *pauseImage);

	slider->setVisible(true);
	playPauseButton->setVisible(true);
	stopButton->setVisible(true);
}
void VideoComponent::stopped()
{
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::hidePlayingControls,this));
}
void VideoComponent::hidePlayingControls()
{
	slider->setVisible(false);
	playPauseButton->setVisible(false);
	stopButton->setVisible(false);
}