#include "VLCMenuTree.h"
#include "icons.h"
#include <modules\vf_core\vf_core.h>


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

class SmartTreeViewItem  : public juce::TreeViewItem
{
	VLCMenuTree* owner;
protected:
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
	virtual const juce::Drawable* getIcon()
	{
		return (shortcutDisplay && !isSelected())?owner->getItemImage():nullptr;
	}
    void paintItem (juce::Graphics& g, int width, int height)
    {
		bool isItemSelected=isSelected();


		
		float fontSize =  0.9f*height;
	
		float hborder = height/8.f;	
	
		float iconhborder = height/2.f;
		int iconSize = height;

		
		float xBounds = iconSize + iconhborder;

		juce::Rectangle<float> borderBounds(xBounds, hborder, width-xBounds, height-2.f*hborder);

		if(isItemSelected)
		{
			g.setGradientFill (juce::ColourGradient(juce::Colours::blue.darker(),
											   borderBounds.getX(), 0.f,
											   juce::Colours::black,
											   borderBounds.getRight(), 0.f,
											   false));

			g.fillRect(borderBounds);

			g.setGradientFill (juce::ColourGradient(juce::Colours::blue.brighter(),
											   borderBounds.getX(), 0.f,
											   juce::Colours::black,
											   borderBounds.getRight(), 0.f,
											   false));

			g.drawRect(borderBounds);
		}

		if(!shortcutDisplay)
		{
			g.setGradientFill (juce::ColourGradient(juce::Colours::lightgrey,
											   borderBounds.getX(), 0.f,
											   juce::Colours::black,
											   borderBounds.getRight(), 0.f,
											   false));

			g.drawLine(0, 0, (float)width, 0, 2.f);
		}
	
		g.setColour (juce::Colours::black);
		
		const juce::Drawable* d = getIcon();
		if (d != nullptr)
		{
				d->drawWithin (g, juce::Rectangle<float> (iconhborder, 0.0f, iconSize, iconSize),
							   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
		}
	
		juce::Font f = g.getCurrentFont().withHeight(fontSize);
		f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);

		g.setColour (juce::Colours::white);
		
		int xText = iconSize + 2*(int)iconhborder;
		g.drawFittedText (getUniqueName(),
							xText, 0, width - xText, height,
							juce::Justification::centredLeft, 
							1, //1 line
							1.f//no h scale
							);
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
	juce::String name;
	const juce::Drawable* icon;
	juce::ScopedPointer<AbstractAction> action;
public:
	
    BindTreeViewItem (VLCMenuTree* owner, juce::String name, AbstractAction* action)
		:SmartTreeViewItem(owner)
		,name(name)
		,icon(nullptr)
		,action(action)
    {
	}
    BindTreeViewItem (SmartTreeViewItem& p, juce::String name, AbstractAction* action)
		:SmartTreeViewItem(p)
		,name(name)
		,icon(nullptr)
		,action(action)
    {
	}
	BindTreeViewItem (SmartTreeViewItem& p, juce::String name, const juce::Drawable* icon, AbstractAction* action)
		:SmartTreeViewItem(p)
		,name(name)
		,icon(icon)
		,action(action)
    {
	}
    
	virtual ~BindTreeViewItem()
	{
	}
	
	virtual const juce::Drawable* getIcon()
	{
		return icon?icon:SmartTreeViewItem::getIcon();
	}
    juce::String getUniqueName() const
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
    juce::File file;
public:
    FileTreeViewItem (VLCMenuTree* owner, 
                      juce::File const& file_)
		:SmartTreeViewItem(owner)
		,file(file_)
    {
	}
    FileTreeViewItem (SmartTreeViewItem& p, 
                      juce::File const& file_)
		:SmartTreeViewItem(p)
		,file(file_)
    {
	}
	virtual ~FileTreeViewItem()
	{
	}
	virtual const juce::Drawable* getIcon()
	{
		return file.isDirectory()?shortcutDisplay?getOwner()->getFolderShortcutImage():getOwner()->getFolderImage():nullptr;
	}
    juce::String getUniqueName() const
    {
		juce::File p = file.getParentDirectory();
		return p.getFullPathName() == file.getFullPathName() ?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileName();
    }
	bool mightContainSubItems()                 
	{ 
		return file.isDirectory();
	}
	juce::File const& getFile() const
	{
		return file;
	}
	void itemClicked(const juce::MouseEvent& e)
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
    itemImage = juce::Drawable::createFromImageData (blue_svg, blue_svgSize);
    folderImage = juce::Drawable::createFromImageData (folder_svg, folder_svgSize);
    folderShortcutImage = juce::Drawable::createFromImageData (folderShortcut_svg, folderShortcut_svgSize);
    audioImage = juce::Drawable::createFromImageData (audio_svg, audio_svgSize);
    displayImage = juce::Drawable::createFromImageData (display_svg, display_svgSize);
    subtitlesImage = juce::Drawable::createFromImageData (sub_svg, sub_svgSize);
    exitImage = juce::Drawable::createFromImageData (exit_svg, exit_svgSize);

	setIndentSize(0);
	setDefaultOpenness(true);
	refresh();
}
VLCMenuTree::~VLCMenuTree()
{
    deleteRootItem();
}

void isolate(juce::TreeViewItem* item)
{
	if(!item)
	{
		return;
	}
	juce::TreeViewItem* parent = item->getParentItem();
	if(parent)
	{
		for(int i=0;i<parent->getNumSubItems();++i)
		{
			juce::TreeViewItem* sibling = parent->getSubItem(i);
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
	juce::TreeViewItem* current = &item;
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

	juce::Array<juce::File> destArray;

    FileTreeViewItem* fileParent = dynamic_cast<FileTreeViewItem*>(&item);
	if(fileParent)
	{
		juce::String filters = "*.*";
		bool selectsFiles = true;
		bool selectsDirectories = true;

		fileParent->getFile().findChildFiles(destArray, juce::File::findFilesAndDirectories|juce::File::ignoreHiddenFiles, false);
	}
	else
	{
		juce::File::findFileSystemRoots(destArray);
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
void subtitlesOptions(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "Select", ACTION(nop)));
	item.addSubItem(new BindTreeViewItem (item, "Add", ACTION(listFiles)));
}
void exit(SmartTreeViewItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(SmartTreeViewItem& item)
{
	prepare(item);
	item.addSubItem(new BindTreeViewItem (item, "Open", item.getOwner()->getFolderShortcutImage(), ACTION(listFiles)));
	item.addSubItem(new BindTreeViewItem (item, "Subtitle", item.getOwner()->getSubtitlesImage(), ACTION(subtitlesOptions)));
	item.addSubItem(new BindTreeViewItem (item, "Video options", item.getOwner()->getDisplayImage(), ACTION(videoOptions)));
	item.addSubItem(new BindTreeViewItem (item, "Sound options", item.getOwner()->getAudioImage(), ACTION(soundOptions)));
	item.addSubItem(new BindTreeViewItem (item, "Exit", item.getOwner()->getExitImage(), ACTION(exit)));

}
void VLCMenuTree::refresh()
{

	deleteRootItem();

	juce::TreeViewItem* const root
		= new BindTreeViewItem (this, "Menu", ACTION(getRootITems));

	setRootItem (root);

	root->setSelected(true, true);
	
}
void VLCMenuTree::paint (juce::Graphics& g)
{
	float w = (float)getWidth();
	float h = (float)getHeight();
	float hMargin = 0.025f*getParentWidth();
	float roundness = hMargin/4.f;
	
	///////////////// TREE ZONE:	
	g.setGradientFill (juce::ColourGradient (juce::Colours::darkgrey.withAlpha(0.5f),
										0, h/2.f,
										juce::Colours::black,
										w, h/2.f,
										false));
	g.fillRoundedRectangle(0, 0, w, h, roundness);

	g.setGradientFill (juce::ColourGradient (juce::Colours::lightgrey.withAlpha(0.5f),
										0, h/2.f,
										juce::Colours::black,
										w, h/2.f,
										false));
	g.drawRoundedRectangle(0, 0, w, h, roundness,2.f);
	

}