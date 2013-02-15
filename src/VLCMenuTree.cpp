#include "VLCMenuTree.h"
#include "icons.h"
#include <modules\vf_core\vf_core.h>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>


//==============================================================================
class AbstractAction
{
public:
	virtual ~AbstractAction () { }

	/** Calls the functor.

		This executes during the queue's call to synchronize().
	*/
	virtual void operator() (VLCMenuTreeItem& parent) = 0;
};
//==============================================================================
class VLCMenuTree;
class Action : public AbstractAction
{
    typedef boost::function<void (VLCMenuTreeItem&)> Functor;
    Functor m_f;
public:
    explicit Action (Functor const& f) : m_f (f) { }
    void operator() (VLCMenuTreeItem& parent) { m_f (parent); }
	
	static Action* build (VLCMenuTree &tree, void (*f)(VLCMenuTree&, VLCMenuTreeItem&))
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1));
	}
	template <typename P1>
	static Action* build (VLCMenuTree &tree, void (*f)(VLCMenuTree&, VLCMenuTreeItem&, P1), P1 p1)
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1, p1));
	}
	template <typename P1, typename P2>
	static Action* build (VLCMenuTree &tree, void (*f)(VLCMenuTree&, VLCMenuTreeItem&, P1, P2), P1 p1, P2 p2)
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	static Action* build (VLCMenuTree &tree, void (*f)(VLCMenuTree&, VLCMenuTreeItem&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return new Action(boost::bind<void>(f, boost::ref(tree), _1, p1, p2, p3));
	}
};


//==============================================================================
class VLCMenuTree;

class SmartTreeViewItem  : public juce::TreeViewItem, public VLCMenuTreeItem
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
    void addFile(juce::File const& file_, VLCMenuTreeListener::FileMethod fileMethod_);
	
	void addFiles(juce::Array<juce::File> const& destArray, VLCMenuTreeListener::FileMethod fileMethod)
	{
		//add folders
		for(int i=0;i<destArray.size();++i)
		{
			if(destArray[i].isDirectory())
			{
				addFile(destArray[i], fileMethod);
			}
		}
		//add files
		for(int i=0;i<destArray.size();++i)
		{
			if(!destArray[i].isDirectory())
			{
				addFile(destArray[i], fileMethod);
			}
		}
	}
};
class ActionTreeViewItem  : public SmartTreeViewItem
{
	juce::ScopedPointer<AbstractAction> action;
public:
	
    ActionTreeViewItem (VLCMenuTree* owner, AbstractAction* action)
		:SmartTreeViewItem(owner)
		,action(action)
    {
	}
    ActionTreeViewItem (SmartTreeViewItem& p, AbstractAction* action)
		:SmartTreeViewItem(p)
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
			focusItemAsMenuShortcut();
			action->operator()(*this);
		}
		
	}
};
class BindTreeViewItem  : public ActionTreeViewItem
{
	juce::String name;
	const juce::Drawable* icon;
public:
	
    BindTreeViewItem (VLCMenuTree* owner, juce::String name, AbstractAction* action)
		:ActionTreeViewItem(owner, action)
		,name(name)
		,icon(nullptr)
    {
	}
	BindTreeViewItem (SmartTreeViewItem& p, juce::String name, AbstractAction* action, const juce::Drawable* icon = nullptr)
		:ActionTreeViewItem(p,action)
		,name(name)
		,icon(icon)
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


};

class FileTreeViewItem  : public SmartTreeViewItem
{
    juce::File file;
	VLCMenuTreeListener::FileMethod fileMethod;
public:
    FileTreeViewItem (VLCMenuTree* owner, 
                      juce::File const& file_, VLCMenuTreeListener::FileMethod fileMethod_)
		:SmartTreeViewItem(owner)
		,file(file_)
		,fileMethod(fileMethod_)
    {
	}
    FileTreeViewItem (SmartTreeViewItem& p, 
                      juce::File const& file_, VLCMenuTreeListener::FileMethod fileMethod_)
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
		if(!file.isDirectory())
		{
			getOwner()->getListeners().call(fileMethod, file);
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
	addSubItem(new BindTreeViewItem(*this, name, action, icon));
}

void SmartTreeViewItem::addFile(juce::File const& file, VLCMenuTreeListener::FileMethod fileMethod)
{
	addSubItem(new FileTreeViewItem(*this, file, fileMethod));
}

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
	setOpenCloseButtonsVisible(false);
	setIndentSize(50);
	setOpaque(false);

	refresh();
}
VLCMenuTree::~VLCMenuTree()
{
    deleteRootItem();
}


template <typename P1>
void dispatchToListeners (VLCMenuTree &tree, VLCMenuTreeItem& item, void (VLCMenuTreeListener::*callbackFunction) (P1), P1 p1)
{
	tree.getListeners().call(callbackFunction, p1);
}
template <typename P1, typename P2>
void dispatchToListeners (VLCMenuTree &tree, VLCMenuTreeItem& item, void (VLCMenuTreeListener::*callbackFunction) (P1, P2), P1 p1, P2 p2)
{
	tree.getListeners().call(callbackFunction, p1, p2);
}

void nop(VLCMenuTree &tree, VLCMenuTreeItem& parent)
{
}
void listFiles(VLCMenuTree &tree, VLCMenuTreeItem& item, VLCMenuTreeListener::FileMethod fileMethod)
{
	juce::Array<juce::File> destArray;
	juce::File::findFileSystemRoots(destArray);
	item.addFiles(destArray, fileMethod);
}


void pin(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction ("Pin", Action::build(tree, &nop));
}

void volume(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "free", Action::build(tree, &pin));
	item.addAction( "25%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 25));
	item.addAction( "50%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 50));
	item.addAction( "75%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 75));
	item.addAction( "100%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 100));
	item.addAction( "125%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 125));
	item.addAction( "150%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 150));
	item.addAction( "175%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 175));
	item.addAction( "200%", Action::build(tree, &dispatchToListeners<int>, &VLCMenuTreeListener::onAudioVolume, 200));
}
void soundOptions(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "Volume", Action::build(tree, &volume));
	item.addAction( "Shift", Action::build(tree, &pin));
	item.addAction( "Mute", Action::build(tree, &pin));
}

void crop(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "free", Action::build(tree, &pin));
	item.addAction( "16/10", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onCrop, 16.f/10.f));
	item.addAction( "16/9", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onCrop, 16.f/9.f));
	item.addAction( "4/3", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onCrop, 4.f/3.f));
}
void ratio(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "original", Action::build(tree, &dispatchToListeners<juce::String>, &VLCMenuTreeListener::onSetAspectRatio, juce::String("")));
	item.addAction( "16/10", Action::build(tree, &dispatchToListeners<juce::String>, &VLCMenuTreeListener::onSetAspectRatio, juce::String("16/10")));
	item.addAction( "16/9", Action::build(tree, &dispatchToListeners<juce::String>, &VLCMenuTreeListener::onSetAspectRatio, juce::String("16/9")));
	item.addAction( "4/3", Action::build(tree, &dispatchToListeners<juce::String>, &VLCMenuTreeListener::onSetAspectRatio, juce::String("4/3")));
	
}
void rate(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "free", Action::build(tree, &pin));
	item.addAction( "50%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, .5f));
	item.addAction( "100%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, 1.f));
	item.addAction( "150%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, 1.5f));
	item.addAction( "200%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, 2.f));
	item.addAction( "300%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, 3.f));
	item.addAction( "400%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, 4.f));
	item.addAction( "600%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, 6.f));
	item.addAction( "800%", Action::build(tree, &dispatchToListeners<float>, &VLCMenuTreeListener::onRate, 8.f));
}
void videoOptions(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "FullScreen", Action::build(tree, &dispatchToListeners<bool>, &VLCMenuTreeListener::onFullscreen, true));
	item.addAction( "Windowed", Action::build(tree, &dispatchToListeners<bool>, &VLCMenuTreeListener::onFullscreen, false));
	item.addAction( "Speed", Action::build(tree, &rate));
	item.addAction( "Crop", Action::build(tree, &crop));
	item.addAction( "Ratio", Action::build(tree, &ratio));
}
void subtitlesOptions(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "Select", Action::build(tree, &nop));
	item.addAction( "Add", Action::build(tree, &listFiles, &VLCMenuTreeListener::onOpenSubtitle));
}
void exitVLCFrontend(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(VLCMenuTree &tree, VLCMenuTreeItem& item)
{
	item.addAction( "Open", Action::build(tree, &listFiles, &VLCMenuTreeListener::onOpen), tree.getFolderShortcutImage());
	item.addAction( "Subtitle", Action::build(tree, &subtitlesOptions), tree.getSubtitlesImage());
	item.addAction( "Video options", Action::build(tree, &videoOptions), tree.getDisplayImage());
	item.addAction( "Sound options", Action::build(tree, &soundOptions), tree.getAudioImage());
	item.addAction( "Exit", Action::build(tree, &exitVLCFrontend), tree.getExitImage());

}
void VLCMenuTree::refresh()
{

	deleteRootItem();

	juce::TreeViewItem* const root
		= new BindTreeViewItem (this, "Menu", Action::build(*this, &getRootITems));

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