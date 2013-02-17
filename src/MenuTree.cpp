#include "MenuTree.h"

#include <modules\vf_core\vf_core.h>


//==============================================================================
class MenuTree;

class SmartTreeViewItem  : public juce::TreeViewItem, public MenuTreeItem
{
	MenuTree* owner;
protected:
	bool shortcutDisplay;
public:
    SmartTreeViewItem (MenuTree* owner)
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
	void isolate()
	{
		juce::TreeViewItem* parent = getParentItem();
		if(parent)
		{
			for(int i=0;i<parent->getNumSubItems();++i)
			{
				juce::TreeViewItem* sibling = parent->getSubItem(i);
				if(sibling && sibling!=this )
				{
					parent->removeSubItem(i);
					i--;
				}
			}
		}
	}
	void focusItemAsMenuShortcut()
	{
		clearSubItems();
		juce::TreeViewItem* current = this;
		while(current != nullptr)
		{
			SmartTreeViewItem* bindParent = dynamic_cast<SmartTreeViewItem*>(current);
			if(bindParent)
			{
				bindParent->isolate();
				bindParent->setShortcutDisplay();
			}
			current=current->getParentItem();
		}
	}
    virtual int getItemHeight() const                               
	{
		return (int)owner->getItemHeight(); 
	}
	MenuTree* getOwner()
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
				d->drawWithin (g, juce::Rectangle<float> (iconhborder, 0.0f, (float)iconSize, (float)iconSize),
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
    void addAction(juce::String name, AbstractAction* action, const juce::Drawable* icon = nullptr);
    void addFile(juce::File const& file_, AbstractFileAction* fileMethod_);
	
	void addFiles(juce::Array<juce::File> const& destArray, AbstractFileAction* fileMethod)
	{
		//add folders
		for(int i=0;i<destArray.size();++i)
		{
			if(destArray[i].isDirectory())
			{
				addFile(destArray[i], fileMethod->clone());
			}
		}
		//add files
		for(int i=0;i<destArray.size();++i)
		{
			if(!destArray[i].isDirectory())
			{
				addFile(destArray[i], fileMethod->clone());
			}
		}
	}
};
class ActionTreeViewItem  : public SmartTreeViewItem
{
	juce::String name;
	const juce::Drawable* icon;
	juce::ScopedPointer<AbstractAction> action;
public:
    ActionTreeViewItem (MenuTree* owner, juce::String name, AbstractAction* action, const juce::Drawable* icon = nullptr)
		:SmartTreeViewItem(owner)
		,name(name)
		,icon(icon)
		,action(action)
    {
	}
    ActionTreeViewItem (SmartTreeViewItem& p, juce::String name, AbstractAction* action, const juce::Drawable* icon = nullptr)
		:SmartTreeViewItem(p)
		,name(name)
		,icon(icon)
		,action(action)
    {
	}
    
	virtual ~ActionTreeViewItem()
	{
	}
	void itemSelectionChanged(bool isSelected)
	{
		if(isSelected)
		{
			if(action)
			{
				action->operator()(*this);
			}
		}
		
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


};

class FileTreeViewItem  : public SmartTreeViewItem
{
    juce::File file;
	juce::ScopedPointer<AbstractFileAction> fileMethod;
public:
    FileTreeViewItem (MenuTree* owner, 
                      juce::File const& file_, AbstractFileAction* fileMethod_)
		:SmartTreeViewItem(owner)
		,file(file_)
		,fileMethod(fileMethod_)
    {
	}
    FileTreeViewItem (SmartTreeViewItem& p, 
                      juce::File const& file_, AbstractFileAction* fileMethod_)
		:SmartTreeViewItem(p)
		,file(file_)
		,fileMethod(fileMethod_)
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
		if(!file.isDirectory() && fileMethod)
		{
			fileMethod->operator()(*this, file);
		}
	}

	void itemSelectionChanged(bool isSelected)
	{
		if(isSelected)
		{
			if(file.isDirectory())
			{
				focusItemAsMenuShortcut();
				
				const juce::String filters = "*.*";
				const bool selectsFiles = true;
				const bool selectsDirectories = true;
		
				juce::Array<juce::File> destArray;
				getFile().findChildFiles(destArray, juce::File::findFilesAndDirectories|juce::File::ignoreHiddenFiles, false);
				addFiles(destArray, fileMethod);
			}
		}
		
	}
	
};
void SmartTreeViewItem::addAction(juce::String name, AbstractAction* action, const juce::Drawable* icon)
{
	addSubItem(new ActionTreeViewItem(*this, name, action, icon));
}

void SmartTreeViewItem::addFile(juce::File const& file, AbstractFileAction* fileMethod)
{
	addSubItem(new FileTreeViewItem(*this, file, fileMethod));
}

MenuTree::MenuTree() : rootAction(nullptr), itemImage(nullptr)
{

	setIndentSize(0);
	setDefaultOpenness(true);
	setOpenCloseButtonsVisible(false);
	setIndentSize(50);
	setOpaque(false);

	refresh();
}
MenuTree::~MenuTree()
{
    deleteRootItem();
}


void MenuTree::refresh()
{

	deleteRootItem();

	if(rootAction)
	{
		juce::TreeViewItem* const root
			= new ActionTreeViewItem (this, "Menu", rootAction);

		setRootItem (root);

		root->setSelected(true, true);
	}
	
}
void MenuTree::paint (juce::Graphics& g)
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