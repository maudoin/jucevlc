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
class BigFileTreeComponent : public virtual TreeView, public AppProportionnalComponent
{
    TimeSliceThread thread;
    ListenerList <VLCMenuTreeListener> listeners;
public:
	BigFileTreeComponent(DirectoryContentsList& p);
	virtual ~BigFileTreeComponent();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (Graphics& g);
	
	TimeSliceThread & getFilesThread(){return thread;}
	
	ListenerList <VLCMenuTreeListener>& getListeners (){return listeners;}
};
}
#endif