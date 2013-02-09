#include "VLCMenuTree.h"
#include <modules\vf_core\vf_core.h>

namespace juce
{

Image juce_createIconForFile (const File& file);

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
  class VLCMenuTree;
class SmartTreeViewItem  : public TreeViewItem
{
	VLCMenuTree* owner;
	bool shortcutDisplay;
public:
    SmartTreeViewItem (VLCMenuTree* owner)
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
	VLCMenuTree* getOwner()
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
        if (!isNowOpen)
        {
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
	
    BindTreeViewItem (VLCMenuTree* owner, String name, AbstractAction* action)
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

void listFiles(SmartTreeViewItem& item);
class FileTreeViewItem  : public SmartTreeViewItem
{
    File file;
public:
    FileTreeViewItem (VLCMenuTree* owner, 
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
		File p = file.getParentDirectory();
		getOwner()->getLookAndFeel().drawFileBrowserRow (g, width, height,
			p.getFullPathName() == file.getFullPathName() ?(file.getFileName()+String(" (")+file.getVolumeLabel()+String(")")):file.getFileName(),
            nullptr, "", "",file.isDirectory(), isSelected(),
            0, *(DirectoryContentsDisplayComponent*)0);
	}
	void itemClicked(const MouseEvent& e)
	{
		if(!file.isDirectory())
		{
			getOwner()->getListeners().call(&VLCMenuTreeListener::onOpen, file, e);
		}
	}
	void itemSelectionChanged(bool isSelected)
	{
		if(isSelected)
		{
			if(file.isDirectory())
			{
				listFiles(*this);
			}
			else
			{
				
			}
		}
		
	}

};

VLCMenuTree::VLCMenuTree() 
{
	setIndentSize(0);
	setDefaultOpenness(true);
	refresh();
}
VLCMenuTree::~VLCMenuTree()
{
    deleteRootItem();
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
void prepare(SmartTreeViewItem& item)
{
	item.clearSubItems();
	TreeViewItem* current = &item;
	while(current != nullptr)
	{
		isolate(current);
		SmartTreeViewItem* bindParent = dynamic_cast<SmartTreeViewItem*>(current);
		if(bindParent)
		{
			bindParent->setShortcutDisplay();
		}
		current=current->getParentItem();
	}
}
void nop(SmartTreeViewItem& parent)
{
}
void listFiles(SmartTreeViewItem& item)
{
	prepare(item);

	Array<File> destArray;

    FileTreeViewItem* fileParent = dynamic_cast<FileTreeViewItem*>(&item);
	if(fileParent)
	{
		String filters = "*.*";
		bool selectsFiles = true;
		bool selectsDirectories = true;

		fileParent->getFile().findChildFiles(destArray, File::findFilesAndDirectories|File::ignoreHiddenFiles, false);
	}
	else
	{
		File::findFileSystemRoots(destArray);
	}
	//add folders
	for(int i=0;i<destArray.size();++i)
	{
		if(destArray[i].isDirectory())
		{
			item.addSubItem(new FileTreeViewItem (item, destArray[i]));
		}
	}
	//add files
	for(int i=0;i<destArray.size();++i)
	{
		if(!destArray[i].isDirectory())
		{
			item.addSubItem(new FileTreeViewItem (item, destArray[i]));
		}
	}
}
void pin(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "Pin", ACTION(nop)));
}

void soundOptions(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "Volume", ACTION(pin)));
	item.addSubItem(new BindTreeViewItem (item, "Shift", ACTION(pin)));
	item.addSubItem(new BindTreeViewItem (item, "Mute", ACTION(pin)));
}
void crop(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "free", ACTION(pin)));
	item.addSubItem(new BindTreeViewItem (item, "16/9", ACTION(pin)));
	item.addSubItem(new BindTreeViewItem (item, "4/3", ACTION(pin)));
}
void ratio(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "free", ACTION(pin)));
	item.addSubItem(new BindTreeViewItem (item, "16/9", ACTION(pin)));
	item.addSubItem(new BindTreeViewItem (item, "4/3", ACTION(pin)));
}
void videoOptions(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "FullScreen", ACTION(nop)));
	item.addSubItem(new BindTreeViewItem (item, "Crop", ACTION(crop)));
	item.addSubItem(new BindTreeViewItem (item, "Ratio", ACTION(ratio)));
}
void exit(SmartTreeViewItem& item)
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "Open", ACTION(listFiles)));
	item.addSubItem(new BindTreeViewItem (item, "Select subtitle", ACTION(nop)));
	item.addSubItem(new BindTreeViewItem (item, "Add subtitle", ACTION(listFiles)));
	item.addSubItem(new BindTreeViewItem (item, "Video options", ACTION(videoOptions)));
	item.addSubItem(new BindTreeViewItem (item, "Sound options", ACTION(soundOptions)));
	item.addSubItem(new BindTreeViewItem (item, "Exit", ACTION(exit)));

}
void VLCMenuTree::refresh()
{

	deleteRootItem();

	TreeViewItem* const root
		= new BindTreeViewItem (this, "Menu", ACTION(getRootITems));

	setRootItem (root);

	root->setSelected(true, true);
	
}
void VLCMenuTree::paint (Graphics& g)
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