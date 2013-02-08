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
class SmartTreeViewItem;
class AbstractAction
{
public:
	virtual ~AbstractAction () { }

	/** Calls the functor.

		This executes during the queue's call to synchronize().
	*/
	virtual void operator() (SmartTreeViewItem& parent) = 0;
};
//==============================================================================
  //template <class Functor>
  class Action : public AbstractAction
  {
	  typedef void (*Functor)(SmartTreeViewItem&);
  public:
    explicit Action (Functor const& f) : m_f (f) { }
    void operator() (SmartTreeViewItem& parent) { m_f (parent); }

  private:
    Functor m_f;
  };
#define ACTION(f) new Action(&f)
//#define ACTION(f) new Action(std::bind(&f,std::placeholders::_1))
//==============================================================================
  class BigFileTreeComponent;
class SmartTreeViewItem  : public TreeViewItem
{
	BigFileTreeComponent* owner;
	bool shortcutDisplay;
public:
    SmartTreeViewItem (BigFileTreeComponent* owner)
		:owner(owner)
		,shortcutDisplay(false)
    {
	}
    SmartTreeViewItem (SmartTreeViewItem& p)
		:owner(p.owner)
		,shortcutDisplay(false)
    {
	}
    
	virtual ~SmartTreeViewItem()
	{
	}
    virtual int getItemHeight() const                               
	{
		return (int)owner->getItemHeight(); 
	}
	BigFileTreeComponent* getOwner()
	{
		return owner;
	}
	void setShortcutDisplay(bool shortCut = true)
	{
		shortcutDisplay = shortCut;
		setDrawsInLeftMargin(shortCut);
	}
    virtual int getIndentX() const noexcept
	{
		int x = owner->isRootItemVisible() ? 1 : 0;

		if (! owner->areOpenCloseButtonsVisible())
			--x;
		if(shortcutDisplay)
		{
			return x;
		}
		bool px = 0;
		for (TreeViewItem* p = getParentItem(); p != nullptr; p = p->getParentItem())
		{
			
			SmartTreeViewItem* bindParent = dynamic_cast<SmartTreeViewItem*>(p);
			if(!bindParent || !bindParent->shortcutDisplay)
			{
				++x;
			}
			else
			{
				px = 1;
			}

		}
			x+=px;

		return x * getOwnerView()->getIndentSize();
	}
    void paintItem (Graphics& g, int width, int height)
    {
        owner->getLookAndFeel().drawFileBrowserRow (g, width, height,
                                                  getUniqueName(),
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
class BindTreeViewItem  : public SmartTreeViewItem
{
	String name;
	ScopedPointer<AbstractAction> action;
public:
	
    BindTreeViewItem (BigFileTreeComponent* owner, String name, AbstractAction* action)
		:SmartTreeViewItem(owner)
		,name(name)
		,action(action)
    {
	}
    BindTreeViewItem (SmartTreeViewItem& p, String name, AbstractAction* action)
		:SmartTreeViewItem(p)
		,name(name)
		,action(action)
    {
	}
    
	virtual ~BindTreeViewItem()
	{
	}
	
    String getUniqueName() const
    {
        return name;
    }

    virtual bool mightContainSubItems()
    {
        return true;
    }

	void itemSelectionChanged(bool isSelected)
	{
		if(isSelected)
		{
			action->operator()(*this);
		}
		
	}

};
class FileTreeViewItem  : public SmartTreeViewItem
{
    File const& file;
public:
    FileTreeViewItem (BigFileTreeComponent* owner, 
                      File const& file_)
		:SmartTreeViewItem(owner)
		,file(file_)
    {
	}
    FileTreeViewItem (SmartTreeViewItem& p, 
                      File const& file_)
		:SmartTreeViewItem(p)
		,file(file_)
    {
	}
	virtual ~FileTreeViewItem()
	{
	}
    String getUniqueName() const
    {
        return file.getFullPathName();
    }
	bool mightContainSubItems()                 
	{ 
		return file.isDirectory();
	}
	File const& getFile() const
	{
		return file;
	}
    void paintItem (Graphics& g, int width, int height)
    {
		getOwner()->getLookAndFeel().drawFileBrowserRow (g, width, height,
			file.getParentDirectory() == File::nonexistent ?(file.getFileName()+String(" ")+file.getVolumeLabel()):file.getFileName(),
            nullptr, "", "",file.isDirectory(), isSelected(),
            0, *(DirectoryContentsDisplayComponent*)0);
	}
	void itemSelectionChanged(bool isSelected)
	{
		if(isSelected)
		{
			//owner->send;
		}
		
	}
};

BigFileTreeComponent::BigFileTreeComponent(DirectoryContentsList& p) 
	: thread("FileList")
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
void prepare(SmartTreeViewItem& parent)
{
	parent.clearSubItems();
	TreeViewItem* item = &parent;
	while(item != nullptr)
	{
		isolate(item);
		SmartTreeViewItem* bindParent = dynamic_cast<SmartTreeViewItem*>(item);
		if(bindParent)
		{
			bindParent->setShortcutDisplay();
		}
		item=item->getParentItem();
	}
}
void nop(SmartTreeViewItem& parent)
{
}
void listFiles(SmartTreeViewItem& parent)
{
	prepare(parent);
    FileTreeViewItem* fileParent = dynamic_cast<FileTreeViewItem*>(&parent);
	if(fileParent)
	{
		String filters = "*.*";
		bool selectsFiles = true;
		bool selectsDirectories = true;

        WildcardFileFilter* wildcard = new WildcardFileFilter(selectsFiles ? filters : String::empty,
                                     selectsDirectories ? "*" : String::empty,
                                     String::empty);

		DirectoryContentsList fileList(wildcard, fileParent->getOwner()->getFilesThread());

		for(int i=0;i<fileList.getNumFiles();++i)
		{
			parent.addSubItem(new FileTreeViewItem (parent, fileList.getFile(i)));
		}
	}
	else
	{
		Array<File> destArray;
		File::findFileSystemRoots(destArray);
		for(int i=0;i<destArray.size();++i)
		{
			parent.addSubItem(new FileTreeViewItem (parent, destArray[i]));
		}
	}
}
void pin(SmartTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "Pin", ACTION(nop)));
}

void soundOptions(SmartTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "Volume", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "Shift", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "Mute", ACTION(pin)));
}
void crop(SmartTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "free", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "16/9", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "4/3", ACTION(pin)));
}
void ratio(SmartTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "free", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "16/9", ACTION(pin)));
	parent.addSubItem(new BindTreeViewItem (parent, "4/3", ACTION(pin)));
}
void videoOptions(SmartTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "FullScreen", ACTION(nop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Crop", ACTION(crop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Ratio", ACTION(ratio)));
}
void exit(SmartTreeViewItem& parent)
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(SmartTreeViewItem& parent)
{
	prepare(parent);
	parent.addSubItem(new BindTreeViewItem (parent, "Open", ACTION(listFiles)));
	parent.addSubItem(new BindTreeViewItem (parent, "Select subtitle", ACTION(nop)));
	parent.addSubItem(new BindTreeViewItem (parent, "Add subtitle", ACTION(listFiles)));
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