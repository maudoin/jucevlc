
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
	:img(new juce::Image(juce::Image::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
{    
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);


	setOpaque(true);
		


	slider = new juce::Slider("media time");
	slider->setRange(0, 1000);
	slider->addListener(this);
	slider->setSliderStyle (juce::Slider::LinearBar);
	slider->setAlpha(1.f);
	//static LookAndFeel lnf;
	//slider->setLookAndFeel(&lnf);
	slider->setOpaque(true);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

	sliderUpdating = false;
	videoUpdating = false;

		
	tree = new VLCMenuTree ();
	tree->getListeners().add(this);
	tree->setOpenCloseButtonsVisible(false);
	tree->setIndentSize(50);
	
    mediaTimeLabel = new juce::Label("mediaTime");
	mediaTimeLabel->setColour(juce::Label::textColourId, juce::Colours::white);
	mediaTimeLabel->setOpaque(false);
	mediaTimeLabel->setText("00:00:00/00:00:00", true);
	mediaTimeLabel->setJustificationType(juce::Justification::right);
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
    addChildComponent(mediaTimeLabel);
		

	setSize (600, 300);

	//after set Size
	vlc = new VLCWrapper();
	vlc->SetDisplayCallback(this);
    vlc->SetEventCallBack(this);
		
}
VideoComponent::~VideoComponent()
{    
	slider->removeListener(this);
	playPauseButton->removeListener(this);
	stopButton->removeListener(this);
	{
		vlc->Pause();
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
		vlc = nullptr;
	}
	slider = nullptr;
	tree = nullptr;
	ptr = nullptr;
	img = nullptr;
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
	
	if(!vlc || vlc->isPaused())
	{
		return;
	}

	int w =  getWidth();
	int h =  getHeight();
	

	int hMargin = 0.025*w;
	int treeWidth = w/4;
	int buttonWidth = 0.03*w;
	int sliderHeight = 0.025*h;
	int roundness = 2.f;

	g.setGradientFill (juce::ColourGradient (juce::Colours::darkgrey.darker(),
										w/2, h-sliderHeight-buttonWidth,
										juce::Colour (0x8000),
										w/2, h,
										false));
	g.fillRoundedRectangle(hMargin/2,  h-sliderHeight-buttonWidth-hMargin/2, w-hMargin, sliderHeight-buttonWidth, roundness);

	g.setGradientFill (juce::ColourGradient (juce::Colours::darkgrey,
										w/2, h-sliderHeight-buttonWidth,
										juce::Colour (0x8000),
										w/2, h,
										false));
	g.drawRoundedRectangle(hMargin/2,  h-sliderHeight-buttonWidth-hMargin/2, w-hMargin, sliderHeight-buttonWidth, roundness,2.f);

	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
	g.drawImage(*img, 0, 0, getWidth(), getHeight(), 0, 0, img->getWidth(), img->getHeight());

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

    tree->setBounds (w-2*hMargin-treeWidth, hMargin,treeWidth, h-sliderHeight-buttonWidth-2*hMargin);
	
		
	slider->setBounds (hMargin, h-sliderHeight-buttonWidth, w-2*hMargin, sliderHeight);

	playPauseButton->setBounds (hMargin, h-buttonWidth, buttonWidth, buttonWidth);
	stopButton->setBounds (hMargin+buttonWidth, h-buttonWidth, buttonWidth, buttonWidth);

	//mediaTimeLabel->setBounds (hMargin+2*buttonWidth, h-buttonWidth, w-2*hMargin-2*buttonWidth, buttonWidth);

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
	
}


void VideoComponent::stop()
{
	if(!vlc)
	{
		return;
	}
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
	img = new juce::Image(img->rescaled(getWidth(), getHeight()));
	ptr = new juce::Image::BitmapData (*img, juce::Image::BitmapData::readWrite);
	vlc->SetBufferFormat(img->getWidth(), img->getHeight(), ptr->lineStride);

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
void VideoComponent::onOpen (const juce::File& file, const juce::MouseEvent& e)
{
	vf::MessageThread::getInstance();
	play(file.getFullPathName().getCharPointer().getAddress());
}
void VideoComponent::onOpenSubtitle (const juce::File& file, const juce::MouseEvent& e)
{
}
void VideoComponent::onOpenPlaylist (const juce::File& file, const juce::MouseEvent& e)
{
}

void VideoComponent::onCrop (const juce::String& ratio)
{
}
void VideoComponent::onSetAspectRatio(const juce::String& ratio)
{
}
void VideoComponent::onShiftAudio(const juce::String& ratio)
{
}
void VideoComponent::onShiftSubtitles(const juce::String& ratio)
{
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
	mediaTimeLabel->setText(getTimeString(), true);
	
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
	mediaTimeLabel->setVisible(true);
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
	mediaTimeLabel->setVisible(true);
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
	mediaTimeLabel->setVisible(false);
}