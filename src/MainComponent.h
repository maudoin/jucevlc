
#ifndef MAINCOMPONENT
#define MAINCOMPONENT

#include "juce.h"
#include "Execute.h"
#include "VLCWrapper.h"
#include "BigFileTree.h"
#include <modules\vf_concurrent\vf_concurrent.h>
#include <sstream>


//==============================================================================
class VideoComponent   : public Component, public DisplayCallback, juce::Slider::Listener
{
	juce::Image img;
    juce::CriticalSection imgCriticalSection;
    ScopedPointer<Slider> slider;
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
	}
    ~VideoComponent()
    {    
		slider->removeListener(this);
		vlc.Stop();
		slider = nullptr;
		vlc.SetDisplayCallback(nullptr);
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
		vlc.Pause();

		const GenericScopedLock<CriticalSection> lock (imgCriticalSection);
		img = img.rescaled(getWidth(), getHeight());

		std::ostringstream oss;
		oss << getWidth()<<"x"<< getHeight();
		Graphics g(img);
		g.fillAll(Colour::fromRGB(0, 0, 0));
		g.setColour(Colour::fromRGB(255, 0, 255));
		g.drawText(oss.str().c_str(), Rectangle<int>(0, 0, 160, 16), Justification::bottomLeft, true);

		vlc.SetDisplayCallback(this);
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

private:
	VLCWrapper vlc;
	bool sliderUpdating;
	bool videoUpdating;
};
//==============================================================================
class MainComponent   : public Component, public FileBrowserListener
{
public:
    //==============================================================================
    MainComponent(File initialFileOrDirectory = File::nonexistent)
     : thread ("FileBrowser")
    {
		
		if (initialFileOrDirectory == File::nonexistent)
		{
			currentRoot = File::getCurrentWorkingDirectory();
		}
		else if (initialFileOrDirectory.isDirectory())
		{
			currentRoot = initialFileOrDirectory;
		}
		else
		{
			currentRoot = initialFileOrDirectory.getParentDirectory();
		}
		String filters = "*.*";
		bool selectsFiles = true;
		bool selectsDirectories = true;

        wildcard = new WildcardFileFilter(selectsFiles ? filters : String::empty,
                                     selectsDirectories ? "*" : String::empty,
                                     String::empty);

		fileList = new DirectoryContentsList (wildcard, thread);

		tree = new BigFileTreeComponent (*fileList);
		//tree->setInitialMenu();
		
		
		
		
        addAndMakeVisible (&videoComponent);
        addAndMakeVisible (tree);
        addAndMakeVisible (videoComponent.getSlider());

		setSize (600, 300);
		
		//tree->addListener (this);
		tree->setOpenCloseButtonsVisible(false);
		tree->setIndentSize(50);
		
		fileList->setDirectory (currentRoot, true, true);
		//setRoot(currentRoot);
		thread.startThread (4);

		
    }

    ~MainComponent()
    {    
		tree = nullptr;
		fileList = nullptr;
		thread.stopThread (10000);
    }
	
	void setScaleComponent(Component* scaleComponent)
	{
		tree->setScaleComponent(scaleComponent);
	}

	//==============================================================================
    /** Callback when the user selects a different file in the browser. */
    virtual void selectionChanged()
	{
		if (TreeViewItem* const firstSelected = tree->getSelectedItem (0)  )
			firstSelected->setOpen (! firstSelected->isOpen());
	}

    /** Callback when the user clicks on a file in the browser. */
    virtual void fileClicked (const File& file, const MouseEvent& e)
	{
		if(e.mods.isRightButtonDown())
		{
			resized();
			videoComponent.setVisible(true);
			videoComponent.play(file.getFullPathName().getCharPointer().getAddress());
		}
	}

    /** Callback when the user double-clicks on a file in the browser. */
    virtual void fileDoubleClicked (const File& file)
	{
		if(file.isDirectory())
		{
			return;
		}
		execute(file.getFullPathName().getCharPointer().getAddress(), file.getParentDirectory().getFullPathName().getCharPointer().getAddress());
	}

    /** Callback when the browser's root folder changes. */
    virtual void browserRootChanged (const File& newRoot)
	{
	}

    void paint (Graphics& g)
    {/*
        g.fillAll (Colours::black);
		

		g.setGradientFill (ColourGradient (Colour (0xff414553),
										   getWidth(), 0,
										   Colours::black,
										   0.8*getWidth(), 0,
										   true));
		g.fillRect (0.75*getWidth(), 0, getWidth(), 0.25*getHeight());*/
    }

    void resized()
    {
		int w =  getWidth();
		int h =  getHeight();
        tree->setBounds (3*w/4, 0,w/4, 0.925*h);
		
		videoComponent.setBounds (0, 0, w, h);
		videoComponent.getSlider()->setBounds (0.1*w, 0.95*h, 0.8*w, 0.025*h);
    }

private:

	File currentRoot;
    TimeSliceThread thread;
    ScopedPointer<WildcardFileFilter> wildcard;
    ScopedPointer<DirectoryContentsList> fileList;
    ScopedPointer<BigFileTreeComponent> tree;
	VideoComponent videoComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif //MAINCOMPONENT
