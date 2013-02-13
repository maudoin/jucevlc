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

    virtual void onCrop (double ratio) = 0;
    virtual void onSetAspectRatio(const juce::String& ratio) = 0;
    virtual void onShiftAudio(const juce::String& ratio) = 0;
    virtual void onShiftSubtitles(const juce::String& ratio) = 0;
};

class VLCMenuTreeListenerDispatcher
{
    juce::ListenerList <VLCMenuTreeListener> listeners;
public:
    //==============================================================================
    /** Destructor. */
	virtual ~VLCMenuTreeListenerDispatcher(){}

    //==============================================================================
	
	juce::ListenerList <VLCMenuTreeListener>& getListeners (){return listeners;}

	virtual void onOpen (const juce::File& file, const juce::MouseEvent& e){ listeners.call(&VLCMenuTreeListener::onOpen, file, e);};
    virtual void onOpenSubtitle (const juce::File& file, const juce::MouseEvent& e){ listeners.call(&VLCMenuTreeListener::onOpenSubtitle, file, e);};
    virtual void onOpenPlaylist (const juce::File& file, const juce::MouseEvent& e){ listeners.call(&VLCMenuTreeListener::onOpenPlaylist, file, e);};

    virtual void onCrop (double ratio){ listeners.call(&VLCMenuTreeListener::onCrop, ratio);};
    virtual void onSetAspectRatio(const juce::String& ratio){ listeners.call(&VLCMenuTreeListener::onSetAspectRatio, ratio);};
    virtual void onShiftAudio(const juce::String& ratio){ listeners.call(&VLCMenuTreeListener::onShiftAudio, ratio);};
    virtual void onShiftSubtitles(const juce::String& ratio){ listeners.call(&VLCMenuTreeListener::onShiftSubtitles, ratio);};
};
//==============================================================================
class VLCMenuTree : public virtual juce::TreeView, public AppProportionnalComponent, public VLCMenuTreeListenerDispatcher
{
    juce::ScopedPointer<juce::Drawable> itemImage;
    juce::ScopedPointer<juce::Drawable> folderImage;
    juce::ScopedPointer<juce::Drawable> folderShortcutImage;
    juce::ScopedPointer<juce::Drawable> audioImage;
    juce::ScopedPointer<juce::Drawable> displayImage;
    juce::ScopedPointer<juce::Drawable> subtitlesImage;
    juce::ScopedPointer<juce::Drawable> exitImage;
public:
	VLCMenuTree();
	virtual ~VLCMenuTree();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (juce::Graphics& g);

	juce::Drawable const* getItemImage() const { return itemImage; };
	juce::Drawable const* getFolderImage() const { return folderImage; };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage; };
	juce::Drawable const* getAudioImage() const { return audioImage; };
	juce::Drawable const* getDisplayImage() const { return displayImage; };
	juce::Drawable const* getSubtitlesImage() const { return subtitlesImage; };
	juce::Drawable const* getExitImage() const { return exitImage; };
};

#endif