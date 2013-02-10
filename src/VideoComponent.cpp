
#include "VideoComponent.h"


VideoComponent::VideoComponent()
	:img(new juce::Image(juce::Image::PixelFormat::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
{    
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);

	setOpaque(true);
		


	slider = new juce::Slider();
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
	
    mediaTimeLabel = new juce::Label();
    playPauseButton = new juce::ImageButton("");
    stopButton = new juce::ImageButton("");


    addAndMakeVisible (tree);
    addAndMakeVisible (slider);
		

	setSize (600, 300);

	//after set Size
	vlc.SetDisplayCallback(this);
		
}
VideoComponent::~VideoComponent()
{    
	slider->removeListener(this);
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
		slider->setValue(1000, juce::sendNotificationSync);
		vlc.SetDisplayCallback(nullptr);
	}
	slider = nullptr;
	tree = nullptr;
	ptr = nullptr;
	img = nullptr;
}
	
void VideoComponent::setScaleComponent(juce::Component* scaleComponent)
{
	tree->setScaleComponent(scaleComponent);
}
void VideoComponent::paint (juce::Graphics& g)
{
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
	g.drawImage(*img, 0, 0, getWidth(), getHeight(), 0, 0, img->getWidth(), img->getHeight());
}
	
void VideoComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();
    tree->setBounds (3*w/4, 0,w/4, 0.925*h);
		
	slider->setBounds (0.1*w, 0.95*h, 0.8*w, 0.025*h);

	//rebuild buffer
	bool restart(vlc.isPaused());

	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);

	std::ostringstream oss;
	oss << "VLC "<< vlc.getInfo()<<"\n";
	oss << getWidth()<<"x"<< getHeight();
	juce::Graphics g(*img);
	g.fillAll(juce::Colour::fromRGB(0, 0, 0));
	g.setColour(juce::Colour::fromRGB(255, 0, 255));
	g.drawText(oss.str().c_str(), juce::Rectangle<int>(0, 0, img->getWidth(), img->getHeight()/10), juce::Justification::bottomLeft, true);

}


void VideoComponent::play(char* path)
{
	slider->setValue(1000, juce::sendNotificationSync);

	img = new juce::Image(img->rescaled(getWidth(), getHeight()));
	ptr = new juce::Image::BitmapData (*img, juce::Image::BitmapData::readWrite);
	vlc.SetBufferFormat(img->getWidth(), img->getHeight(), ptr->lineStride);


	vlc.OpenMedia(path);
	vlc.Play();

	slider->setValue(0);
}
	

void *VideoComponent::lock(void **p_pixels)
{
	imgCriticalSection.enter();
	*p_pixels = ptr->getLinePointer(0);
	return NULL; /* picture identifier, not needed here */
}

void VideoComponent::unlock(void *id, void *const *p_pixels)
{
	imgCriticalSection.exit();

	jassert(id == NULL); /* picture identifier, not needed here */
}

void VideoComponent::display(void *id)
{
	vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::frameReady,this));
	jassert(id == NULL);
}
void VideoComponent::frameReady()
{
	repaint();
	if(!sliderUpdating)
	{
		videoUpdating = true;
		slider->setValue(vlc.GetTime()*1000./vlc.GetLength(), juce::sendNotificationSync);
		videoUpdating =false;
	}
}
void VideoComponent::sliderValueChanged (juce::Slider* slider)
{
	if(!videoUpdating)
	{
		sliderUpdating = true;
		vlc.SetTime(slider->getValue()*vlc.GetLength()/1000.);
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