
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "VLCMenuTree.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>


//==============================================================================
class VideoComponent   : public Component, public DisplayCallback, juce::Slider::Listener, public VLCMenuTreeListener
{
	juce::Image img;
    juce::CriticalSection imgCriticalSection;
    ScopedPointer<Slider> slider;
    ScopedPointer<VLCMenuTree> tree;
public:
    VideoComponent():img(Image::PixelFormat::RGB, 160, 160, false)
    {    
		const GenericScopedLock<CriticalSection> lock (imgCriticalSection);
		Graphics g(img);
		g.setColour(Colour::fromRGB(255, 0, 255));
		g.drawText("Init", Rectangle<int>(0, 0, 160, 16), Justification::bottomLeft, true);
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
		
	}
    ~VideoComponent()
    {    
		slider->removeListener(this);
		{
			vlc.Stop();
			vlc.SetDisplayCallback(nullptr);
		}
		slider = nullptr;
		tree = nullptr;
    }
	
	void setScaleComponent(Component* scaleComponent)
	{
		tree->setScaleComponent(scaleComponent);
	}
    void paint (Graphics& g)
    {
		const GenericScopedLock<CriticalSection> lock (imgCriticalSection);
		g.drawImage(img, 0, 0, getWidth(), getHeight(), 0, 0, img.getWidth(), img.getHeight());
	}
	
	int imageWidth()
	{
		return img.getWidth();
	}
	virtual int imageHeight()
	{
		return img.getHeight();
	}
	virtual int imageStride()
	{
		Image::BitmapData ptr(img, Image::BitmapData::readWrite);
		return ptr.lineStride;
	}
    void resized()
    {
		int w =  getWidth();
		int h =  getHeight();
        tree->setBounds (3*w/4, 0,w/4, 0.925*h);
		
		slider->setBounds (0.1*w, 0.95*h, 0.8*w, 0.025*h);

		//rebuild buffer
		bool restart(vlc.isPaused());
		vlc.Pause();

		const GenericScopedLock<CriticalSection> lock (imgCriticalSection);
		img = img.rescaled(getWidth(), getHeight());

		std::ostringstream oss;
		oss << "VLC "<< vlc.getInfo()<<"\n";
		oss << getWidth()<<"x"<< getHeight();
		Graphics g(img);
		g.fillAll(Colour::fromRGB(0, 0, 0));
		g.setColour(Colour::fromRGB(255, 0, 255));
		g.drawText(oss.str().c_str(), Rectangle<int>(0, 0, 160, 16), Justification::bottomLeft, true);

		if(restart)
		{
			vlc.Play();
		}
	}


	void play(char* path)
	{
		vlc.SetDisplayCallback(this);
		vlc.OpenMedia(path);
		vlc.Play();

		slider->setValue(0);
	}
	

	void *lock(void **p_pixels)
	{
		imgCriticalSection.enter();
		Image::BitmapData ptr(img, Image::BitmapData::readWrite);
		*p_pixels = ptr.getLinePointer(0);
		return NULL; /* picture identifier, not needed here */
	}

	void unlock(void *id, void *const *p_pixels)
	{
		imgCriticalSection.exit();

		jassert(id == NULL); /* picture identifier, not needed here */
	}

	void display(void *id)
	{
		vf::MessageThread::getInstance().queuef(std::bind  (&VideoComponent::frameReady,this));
		jassert(id == NULL);
	}
	void frameReady()
	{
		repaint();
		if(!sliderUpdating)
		{
			videoUpdating = true;
			slider->setValue(vlc.GetTime()*1000./vlc.GetLength(), sendNotificationSync);
			videoUpdating =false;
		}
	}
    void sliderValueChanged (Slider* slider)
	{
		if(!videoUpdating)
		{
			sliderUpdating = true;
			vlc.SetTime(slider->getValue()*vlc.GetLength()/1000.);
			sliderUpdating =false;
		}
	}
	Slider* getSlider()
	{
		return slider.get();
	}
	//MenuTreeListener
    virtual void onOpen (const File& file, const MouseEvent& e)
	{
		vf::MessageThread::getInstance();
		play(file.getFullPathName().getCharPointer().getAddress());
	}
    virtual void onOpenSubtitle (const File& file, const MouseEvent& e)
	{
	}
    virtual void onOpenPlaylist (const File& file, const MouseEvent& e)
	{
	}

    virtual void onCrop (const String& ratio)
	{
	}
    virtual void onSetAspectRatio(const String& ratio)
	{
	}
    virtual void onShiftAudio(const String& ratio)
	{
	}
    virtual void onShiftSubtitles(const String& ratio)
	{
	}
private:
	VLCWrapper vlc;
	bool sliderUpdating;
	bool videoUpdating;
};

#endif //VIDEO_COMPONENT
