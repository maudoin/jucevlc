#ifndef BIGFILETREE
#define BIGFILETREE


#include "juce.h"
#include "AppProportionnalComponent.h"
	
class MenuTreeListener
{
public:
    //==============================================================================
    /** Destructor. */
	virtual ~MenuTreeListener(){}

    //==============================================================================
	typedef void (MenuTreeListener::*FileMethod)(juce::File);

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
class MenuTreeItem;
class AbstractAction
{
public:
	virtual ~AbstractAction () { }

	/** Calls the functor.

		This executes during the queue's call to synchronize().
	*/
	virtual void operator() (MenuTreeItem& parent) = 0;
};
//==============================================================================
class MenuTreeItem
{
public:
	virtual ~MenuTreeItem(){}
    virtual void addAction(juce::String name, AbstractAction* action, const juce::Drawable* icon = nullptr) = 0;
	virtual void addFiles(juce::Array<juce::File> const& destArray, MenuTreeListener::FileMethod fileMethod) = 0;
};

//==============================================================================
class MenuTree : public virtual juce::TreeView, public AppProportionnalComponent
{
    juce::ScopedPointer<juce::Drawable> itemImage;
    juce::ScopedPointer<juce::Drawable> folderImage;
    juce::ScopedPointer<juce::Drawable> folderShortcutImage;
    juce::ScopedPointer<juce::Drawable> audioImage;
    juce::ScopedPointer<juce::Drawable> displayImage;
    juce::ScopedPointer<juce::Drawable> subtitlesImage;
    juce::ScopedPointer<juce::Drawable> exitImage;
    juce::ListenerList <MenuTreeListener> listeners;
public:
	MenuTree();
	virtual ~MenuTree();
	virtual void refresh();
	void setInitialMenu();
	
	void paint (juce::Graphics& g);
	
	juce::ListenerList <MenuTreeListener>& getListeners (){return listeners;}
	juce::Drawable const* getItemImage() const { return itemImage; };
	juce::Drawable const* getFolderImage() const { return folderImage; };
	juce::Drawable const* getFolderShortcutImage() const { return folderShortcutImage; };
	juce::Drawable const* getAudioImage() const { return audioImage; };
	juce::Drawable const* getDisplayImage() const { return displayImage; };
	juce::Drawable const* getSubtitlesImage() const { return subtitlesImage; };
	juce::Drawable const* getExitImage() const { return exitImage; };
};

#endif