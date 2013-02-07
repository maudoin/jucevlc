#include "BigFileTree.h"
#include <modules\vf_core\vf_core.h>

namespace juce
{

Image juce_createIconForFile (const File& file);
//==============================================================================

/*
class FileListTreeItem2   : public TreeViewItem,
                           private TimeSliceClient,
                           private AsyncUpdater,
                           private ChangeListener
{
public:
    FileListTreeItem2 (FileTreeComponent& owner_,
                      DirectoryContentsList* const parentContentsList_,
                      const int indexInContentsList_,
                      const File& file_,
                      TimeSliceThread& thread_)
        : file (file_),
          owner (owner_),
          parentContentsList (parentContentsList_),
          indexInContentsList (indexInContentsList_),
          subContentsList (nullptr, false),
          thread (thread_)
    {
        DirectoryContentsList::FileInfo fileInfo;

        if (parentContentsList_ != nullptr
             && parentContentsList_->getFileInfo (indexInContentsList_, fileInfo))
        {
            fileSize = File::descriptionOfSizeInBytes (fileInfo.fileSize);
            modTime = fileInfo.modificationTime.formatted ("%d %b '%y %H:%M");
            isDirectory = fileInfo.isDirectory;
        }
        else
        {
            isDirectory = true;
        }
    }

    virtual ~FileListTreeItem2()
    {
        thread.removeTimeSliceClient (this);
        clearSubItems();
    }

    //==============================================================================
    bool mightContainSubItems()                 { return isDirectory; }
    String getUniqueName() const                { return file.getFullPathName(); }
    virtual int getItemHeight() const                   { return 72; }

    var getDragSourceDescription()              { return owner.getDragAndDropDescription(); }

    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
        {
            clearSubItems();

            isDirectory = file.isDirectory();

            if (isDirectory)
            {
                if (subContentsList == nullptr)
                {
                    jassert (parentContentsList != nullptr);

                    DirectoryContentsList* const l = new DirectoryContentsList (parentContentsList->getFilter(), thread);
                    l->setDirectory (file, true, true);

                    setSubContentsList (l, true);
                }

                changeListenerCallback (nullptr);
            }
        }
    }

    void setSubContentsList (DirectoryContentsList* newList, const bool canDeleteList)
    {
        OptionalScopedPointer<DirectoryContentsList> newPointer (newList, canDeleteList);
        subContentsList = newPointer;
        newList->addChangeListener (this);
    }

    virtual void changeListenerCallback (ChangeBroadcaster*)
    {
        clearSubItems();

        if (isOpen() && subContentsList != nullptr)
        {
            for (int i = 0; i < subContentsList->getNumFiles(); ++i)
            {
                FileListTreeItem2* const item
                    = new FileListTreeItem2 (owner, subContentsList, i, subContentsList->getFile(i), thread);

                addSubItem (item);
            }
        }
    }

    void paintItem (Graphics& g, int width, int height)
    {
        if (file != File::nonexistent)
        {
            updateIcon (true);

            if (icon.isNull())
                thread.addTimeSliceClient (this);
        }

        owner.getLookAndFeel().drawFileBrowserRow (g, width, height,
                                                   file.getFileNameWithoutExtension(),
                                                   &icon, fileSize, modTime,
                                                   isDirectory, isSelected(),
                                                   indexInContentsList, owner);
    }

    void itemClicked (const MouseEvent& e)
    {
        owner.sendMouseClickMessage (file, e);
    }

    void itemDoubleClicked (const MouseEvent& e)
    {
        TreeViewItem::itemDoubleClicked (e);

        owner.sendDoubleClickMessage (file);
    }

    void itemSelectionChanged (bool)
    {
        owner.sendSelectionChangeMessage();
    }

    int useTimeSlice()
    {
        updateIcon (false);
        return -1;
    }

    void handleAsyncUpdate()
    {
        owner.repaint();
    }

    const File file;

protected:
    FileTreeComponent& owner;
    DirectoryContentsList* parentContentsList;
    int indexInContentsList;
    OptionalScopedPointer<DirectoryContentsList> subContentsList;
    bool isDirectory;
    TimeSliceThread& thread;
    Image icon;
    String fileSize, modTime;

    void updateIcon (const bool onlyUpdateIfCached)
    {
        if (icon.isNull())
        {
            const int hashCode = (file.getFullPathName() + "_iconCacheSalt").hashCode();
            Image im (ImageCache::getFromHashCode (hashCode));

            if (im.isNull() && ! onlyUpdateIfCached)
            {
                im = juce_createIconForFile (file);

                if (im.isValid())
                    ImageCache::addImageToCache (im, hashCode);
            }

            if (im.isValid())
            {
                icon = im;
                triggerAsyncUpdate();
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListTreeItem2)
};

class BigFileListTreeItem : public FileListTreeItem2
{
	BigFileTreeComponent& bigFileTreeComponent;
public:
    BigFileListTreeItem (BigFileTreeComponent& owner_,
                      DirectoryContentsList* const parentContentsList_,
                      const int indexInContentsList_,
                      const File& file_,
					  TimeSliceThread& thread_) 
		: FileListTreeItem2(owner_, parentContentsList_, indexInContentsList_, file_, thread_)
	, bigFileTreeComponent(owner_)
	{
	}
    virtual int getItemHeight() const                               
	{
		return (int)bigFileTreeComponent.getItemHeight(); 
	}
	
    void changeListenerCallback (ChangeBroadcaster*)
    {
        clearSubItems();

        if (isOpen() && subContentsList != nullptr)
        {
            for (int i = 0; i < subContentsList->getNumFiles(); ++i)
            {
                BigFileListTreeItem* const item
                    = new BigFileListTreeItem (bigFileTreeComponent, subContentsList, i, subContentsList->getFile(i), thread);

                addSubItem (item);
            }
        }
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BigFileListTreeItem)
};
*/
//==============================================================================
class BindTreeViewItem;
  class AbstractAction
  {
  public:
    virtual ~AbstractAction () { }

    /** Calls the functor.

        This executes during the queue's call to synchronize().
    */
    virtual void operator() (BindTreeViewItem& parent) = 0;
  };
//==============================================================================
  //template <class Functor>
  class Action : public AbstractAction
  {
	  typedef void (*Functor)(BindTreeViewItem&);
  public:
    explicit Action (Functor const& f) : m_f (f) { }
    void operator() (BindTreeViewItem& parent) { m_f (parent); }

  private:
    Functor m_f;
  };
#define ACTION(f) new Action(&f)
//#define ACTION(f) new Action(std::bind(&f,std::placeholders::_1))
//==============================================================================
  class BigFileTreeComponent;
class BindTreeViewItem  : public TreeViewItem
{
	String name;
	ScopedPointer<AbstractAction> action;
	BigFileTreeComponent* owner;
	bool shortcutDisplay;
public:
	
    BindTreeViewItem (BigFileTreeComponent* owner, String name, AbstractAction* action)
		:name(name)
		,action(action)
		,owner(owner)
		,shortcutDisplay(false)
    {
	}
    BindTreeViewItem (BindTreeViewItem& p, String name, AbstractAction* action)
		:name(name)
		,action(action)
		,owner(p.owner)
		,shortcutDisplay(false)
    {
	}
    
	virtual ~BindTreeViewItem()
	{
	}
	
    virtual int getItemHeight() const                               
	{
		return (int)owner->getItemHeight(); 
	}
	void setShortcutDisplay(bool shortCut = true)
	{
		shortcutDisplay = shortCut;
		setDrawsInLeftMargin(shortCut);
	}
    String getUniqueName() const
    {
        return name;
    }

    virtual bool mightContainSubItems()
    {
        return true;
    }

    void paintItem (Graphics& g, int width, int height)
    {
        owner->getLookAndFeel().drawFileBrowserRow (g, width, height,
                                                  name,
                                                   nullptr, "", "",shortcutDisplay, isSelected(),
                                                   0, *(DirectoryContentsDisplayComponent*)0);
		/*
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (Colours::blue.withAlpha (0.3f));

        g.setFont (height * 0.7f);

        // draw the xml element's tag name..
        g.drawText (name,
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);*/
    }
	void itemSelectionChanged(bool isSelected)
	{
		if(isSelected)
		{
			action->operator()(*this);
			setOpen(isSelected);
		}
		
	}
    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
        {
			//only one item expanded at a time
			TreeViewItem* parent = getParentItem();
			if(parent)
			{
				for(int i=0;i<parent->getNumSubItems();++i)
				{
					TreeViewItem* sibling = parent->getSubItem(i);
					if(sibling && !sibling->isSelected() )
					{
						sibling->setOpen(false);
					}
				}
			}
        }
        else
        {
			clearSubItems();
        }
    }

};

BigFileTreeComponent::BigFileTreeComponent(DirectoryContentsList& p) 
//	: FileTreeComponent(p)
{
	//setRootItemVisible(false);
	setIndentSize(0);
	setDefaultOpenness(true);
	refresh();
}
BigFileTreeComponent::~BigFileTreeComponent()
{
    deleteRootItem();
}
void BigFileTreeComponent::refresh()
{ 
	setInitialMenu();
	/*
	deleteRootItem();
	
	BigFileListTreeItem* const root
		= new BigFileListTreeItem (*this, nullptr, 0, fileList.getDirectory(),
								fileList.getTimeSliceThread());
								
	root->setSubContentsList (&fileList, false);
	setRootItem (root);*/
}

void isolate(TreeViewItem* item)
{
	if(!item)
	{
		return;
	}
	TreeViewItem* parent = item->getParentItem();
	if(parent)
	{
		for(int i=0;i<parent->getNumSubItems();++i)
		{
			TreeViewItem* sibling = parent->getSubItem(i);
			if(sibling && sibling!=item )
			{
				parent->removeSubItem(i);
				i--;
			}
		}
	}
}
void prepare(BindTreeViewItem& parent)
{
	parent.clearSubItems();
	TreeViewItem* item = &parent;
	while(item != nullptr)
	{
		isolate(item);
		BindTreeViewItem* bindParent = dynamic_cast<BindTreeViewItem*>(item);
		if(bindParent)
		{
			bindParent->setShortcutDisplay();
		}
		item=item->getParentItem();
	}
	parent.setShortcutDisplay(false);
}
void nop(BindTreeViewItem& parent)
{
}
void pin(BindTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "Pin", ACTION(nop)));
}

void soundOptions(BindTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "Volume", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "Shift", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "Mute", ACTION(pin)));
}
void crop(BindTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "free", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "16/9", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "4/3", ACTION(pin)));
}
void ratio(BindTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "free", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "16/9", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "4/3", ACTION(pin)));
}
void videoOptions(BindTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "FullScreen", ACTION(nop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Crop", ACTION(crop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Ratio", ACTION(ratio)));
}
void exit(BindTreeViewItem& parent)
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(BindTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "Open", ACTION(nop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Select subtitle", ACTION(nop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Add subtitle", ACTION(nop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Video options", ACTION(videoOptions)));
	parent.addSubItem(new BindTreeViewItem (parent, "Sound options", ACTION(soundOptions)));
	parent.addSubItem(new BindTreeViewItem (parent, "Exit", ACTION(exit)));

}
void BigFileTreeComponent::setInitialMenu()
{

	deleteRootItem();

	TreeViewItem* const root
		= new BindTreeViewItem (this, "Menu", ACTION(getRootITems));

	setRootItem (root);

	root->setSelected(true, true);
	
}
void BigFileTreeComponent::paint (Graphics& g)
{
	int width = getWidth();
	int height = getHeight();

	int roundness = 10;
	g.setColour(Colours::darkgrey.withAlpha(0.5f));
	g.fillRoundedRectangle(0, 0, width, height, roundness);
			
	g.setColour(Colours::lightgrey);
	g.drawRoundedRectangle(0, 0, width, height, roundness, 2);
}
}