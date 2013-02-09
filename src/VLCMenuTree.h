#ifndef BIGFILETREE
#define BIGFILETREE


#include "juce.h"
#include "AppProportionnalComponent.h"
#include <modules\juce_gui_basics\filebrowser\juce_FileBrowserComponent.h>

namespace juce
{
	
class JUCE_API  VLCMenuTreeListener
{
public:
    //==============================================================================
    /** Destructor. */
	virtual ~VLCMenuTreeListener(){}

    //==============================================================================
	
    virtual void onOpen (const File& file, const MouseEvent& e) = 0;
    virtual void onOpenSubtitle (const File& file, const MouseEvent& e) = 0;
    virtual void onOpenPlaylist (const File& file, const MouseEvent& e) = 0;

    virtual void onCrop (const String& ratio) = 0;
    virtual void onSetAspectRatio(const String& ratio) = 0;
    virtual void onShiftAudio(const String& ratio) = 0;
    virtual void onShiftSubtitles(const String& ratio) = 0;
};

//==============================================================================
class VLCMenuTree : public virtual TreeView, public AppProportionnalComponent
{
    ListenerList <VLCMenuTreeListener> listeners;
public:
	VLCMenuTree();
	virtual ~VLCMenuTree();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (Graphics& g);
	
	ListenerList <VLCMenuTreeListener>& getListeners (){return listeners;}
};
}
#endif