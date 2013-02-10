#ifndef BIGFILETREE
#define BIGFILETREE


#include "juce.h"
#include "AppProportionnalComponent.h"
#include <modules\juce_gui_basics\filebrowser\juce_FileBrowserComponent.h>
	
class VLCMenuTreeListener
{
public:
    //==============================================================================
    /** Destructor. */
	virtual ~VLCMenuTreeListener(){}

    //==============================================================================
	
    virtual void onOpen (const juce::File& file, const juce::MouseEvent& e) = 0;
    virtual void onOpenSubtitle (const juce::File& file, const juce::MouseEvent& e) = 0;
    virtual void onOpenPlaylist (const juce::File& file, const juce::MouseEvent& e) = 0;

    virtual void onCrop (const juce::String& ratio) = 0;
    virtual void onSetAspectRatio(const juce::String& ratio) = 0;
    virtual void onShiftAudio(const juce::String& ratio) = 0;
    virtual void onShiftSubtitles(const juce::String& ratio) = 0;
};

//==============================================================================
class VLCMenuTree : public virtual juce::TreeView, public AppProportionnalComponent
{
    juce::ListenerList <VLCMenuTreeListener> listeners;
public:
	VLCMenuTree();
	virtual ~VLCMenuTree();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (juce::Graphics& g);
	
	juce::ListenerList <VLCMenuTreeListener>& getListeners (){return listeners;}
};

#endif