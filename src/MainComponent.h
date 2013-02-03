/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  21 Sep 2012 12:11:41pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_MAINCOMPONENT_MAINCOMPONENT_2983595F__
#define __JUCER_HEADER_MAINCOMPONENT_MAINCOMPONENT_2983595F__

//[Headers]     -- You can add your own extra header files here --

#include "Execute.h"
#include "VLCWrapper.h"
#include "juce.h"
#include <modules\juce_gui_basics\filebrowser\juce_FileBrowserComponent.h>
//[/Headers]


namespace juce
{
//==============================================================================
class BigFileTreeComponent : public virtual FileTreeComponent
{
public:
	BigFileTreeComponent(DirectoryContentsList& p);
	virtual ~BigFileTreeComponent();
	virtual void refresh();
};
}

//==============================================================================
class VideoComponent   : public Component
{
public:
    VideoComponent()
    {    
		setOpaque(true);
		addToDesktop(ComponentPeer::windowIsTemporary);
	}
    ~VideoComponent()
    {    
		vlc.SetOutputWindow(nullptr);
	}
    void paint (Graphics& g)
    {
	}
	void play(char* path)
	{
		vlc.SetOutputWindow(getWindowHandle());
		vlc.OpenMedia(path);
		vlc.Play();
	}
private:
	VLCWrapper vlc;
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
		

		

        addAndMakeVisible (tree);
        //addAndMakeVisible (&videoComponent);

		
		setSize (600, 300);
		
		tree->addListener (this);
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
#define BORDER 10
    void resized()
    {
		int w =  getWidth() - 2*BORDER;
		int h =  getHeight() - 2*BORDER;
        tree->setBounds (3*w/4, BORDER,w/4, h);
		
		videoComponent.setBounds (BORDER, BORDER, w-2*BORDER, h-2*BORDER);
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


#endif   // __JUCER_HEADER_MAINCOMPONENT_MAINCOMPONENT_2983595F__
