
#include "VideoComponent.h"


VideoComponent::VideoComponent()
	:img(new juce::Image(Image::PixelFormat::RGB, 2, 2, false))
	,ptr(new juce::Image::BitmapData(*img, Image::BitmapData::readWrite))
{    
	const GenericScopedLock<CriticalSection> lock (imgCriticalSection);

	setOpaque(true);
		


	slider = new Slider();
	slider->setRange(0, 1000);
	slider->addListener(this);
	slider->setSliderStyle (Slider::LinearBar);
	slider->setAlpha(1.f);
	//static LookAndFeel lnf;
	//slider->setLookAndFeel(&lnf);
	slider->setOpaque(true);
    slider->setTextBoxStyle (Slider::NoTextBox, false, 0, 0);

	sliderUpdating = false;
	videoUpdating = false;

		
	tree = new VLCMenuTree ();
	tree->getListeners().add(this);
	tree->setOpenCloseButtonsVisible(false);
	tree->setIndentSize(50);

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
		const GenericScopedLock<CriticalSection> lock (imgCriticalSection);
		slider->setValue(1000, sendNotificationSync);
		vlc.SetDisplayCallback(nullptr);
	}
	slider = nullptr;
	tree = nullptr;
	ptr = nullptr;
	img = nullptr;
}
	
void VideoComponent::setScaleComponent(Component* scaleComponent)
{
	tree->setScaleComponent(scaleComponent);
}
void VideoComponent::paint (Graphics& g)
{
	const GenericScopedLock<CriticalSection> lock (imgCriticalSection);
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

	const GenericScopedLock<CriticalSection> lock (imgCriticalSection);

	std::ostringstream oss;
	oss << "VLC "<< vlc.getInfo()<<"\n";
	oss << getWidth()<<"x"<< getHeight();
	Graphics g(*img);
	g.fillAll(Colour::fromRGB(0, 0, 0));
	g.setColour(Colour::fromRGB(255, 0, 255));
	g.drawText(oss.str().c_str(), Rectangle<int>(0, 0, img->getWidth(), img->getHeight()/10), Justification::bottomLeft, true);

}


void VideoComponent::play(char* path)
{
	slider->setValue(1000, sendNotificationSync);

	img = new juce::Image(img->rescaled(getWidth(), getHeight()));
	ptr = new Image::BitmapData (*img, Image::BitmapData::readWrite);
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
		slider->setValue(vlc.GetTime()*1000./vlc.GetLength(), sendNotificationSync);
		videoUpdating =false;
	}
}
void VideoComponent::sliderValueChanged (Slider* slider)
{
	if(!videoUpdating)
	{
		sliderUpdating = true;
		vlc.SetTime(slider->getValue()*vlc.GetLength()/1000.);
		sliderUpdating =false;
	}
}
Slider* VideoComponent::getSlider()
{
	return slider.get();
}
//MenuTreeListener
void VideoComponent::onOpen (const File& file, const MouseEvent& e)
{
	vf::MessageThread::getInstance();
	play(file.getFullPathName().getCharPointer().getAddress());
}
void VideoComponent::onOpenSubtitle (const File& file, const MouseEvent& e)
{
}
void VideoComponent::onOpenPlaylist (const File& file, const MouseEvent& e)
{
}

void VideoComponent::onCrop (const String& ratio)
{
}
void VideoComponent::onSetAspectRatio(const String& ratio)
{
}
void VideoComponent::onShiftAudio(const String& ratio)
{
}
void VideoComponent::onShiftSubtitles(const String& ratio)
{
}