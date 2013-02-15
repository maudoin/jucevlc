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
	typedef void (VLCMenuTreeListener::*FileMethod)(juce::File);

    virtual void onOpen (juce::File file) = 0;
    virtual void onOpenSubtitle (juce::File file) = 0;
    virtual void onOpenPlaylist (juce::File file) = 0;
	
    virtual void onCrop (float ratio) = 0;
    virtual void onRate (float rate) = 0;
    virtual void onSetAspectRatio(juce::String ratio) = 0;
    virtual void onAudioVolume(int volume) = 0;
    virtual void onShiftAudio(float ratio) = 0;
    virtual void onShiftSubtitles(float ratio) = 0;

    virtual void onFullscreen(bool fs) = 0;
};

//==============================================================================
class AbstractAction;
class VLCMenuTreeItem
{
public:
	virtual ~VLCMenuTreeItem(){}
    virtual void addAction(juce::String name, AbstractAction* action, const juce::Drawable* icon = nullptr) = 0;
	virtual void addFiles(juce::Array<juce::File> const& destArray, VLCMenuTreeListener::FileMethod fileMethod) = 0;
};

//==============================================================================
class VLCMenuTree : public virtual juce::TreeView, public AppProportionnalComponent
{
    juce::ScopedPointer<juce::Drawable> itemImage;
    juce::ScopedPointer<juce::Drawable> folderImage;
    juce::ScopedPointer<juce::Drawable> folderShortcutImage;
    juce::ScopedPointer<juce::Drawable> audioImage;
    juce::ScopedPointer<juce::Drawable> displayImage;
    juce::ScopedPointer<juce::Drawable> subtitlesImage;
    juce::ScopedPointer<juce::Drawable> exitImage;
    juce::ListenerList <VLCMenuTreeListener> listeners;
public:
	VLCMenuTree();
	virtual ~VLCMenuTree();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (juce::Graphics& g);
	
	juce::ListenerList <VLCMenuTreeListener>& getListeners (){return listeners;}
	juce::Drawable const* getItemImage() const { return itemImage; };
	juce::Drawable const* getFolderImage() const { return folderImage; };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage; };
	juce::Drawable const* getAudioImage() const { return audioImage; };
	juce::Drawable const* getDisplayImage() const { return displayImage; };
	juce::Drawable const* getSubtitlesImage() const { return subtitlesImage; };
	juce::Drawable const* getExitImage() const { return exitImage; };
};

#endif