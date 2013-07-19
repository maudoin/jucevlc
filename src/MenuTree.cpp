#include "MenuTree.h"

#include <modules\vf_core\vf_core.h>

class SmartTreeViewItem  : public juce::TreeViewItem, public MenuItemBase
{
public:
    SmartTreeViewItem (AbstractMenu& owner, juce::String name, AbstractAction action, const juce::Drawable* icon = nullptr)
		:MenuItemBase (owner, name, action, icon)
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
		return (int)owner.getItemHeight(); 
	}
	void setShortcutDisplay(bool shortCut = true)
	{
		m_isShortcut = shortCut;
		setDrawsInLeftMargin(shortCut);
	}
    virtual int getIndentX() const noexcept
	{
		int x = getOwnerView()->isRootItemVisible() ? 1 : 0;

		if (! getOwnerView()->areOpenCloseButtonsVisible())
			--x;
		if(m_isShortcut)
		{
			return x;
		}
		bool px = 0;
		for (TreeViewItem* p = getParentItem(); p != nullptr; p = p->getParentItem())
		{
			
			SmartTreeViewItem* bindParent = dynamic_cast<SmartTreeViewItem*>(p);
			if(!bindParent || !bindParent->m_isShortcut)
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
    void paintItem (juce::Graphics& g, int width, int height)
    {
		MenuItemBase::paintMenuItem(g, width, height, isSelected());
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
    AbstractMenuItem* addAction(juce::String const& name, AbstractAction action, const juce::Drawable* icon = nullptr)
	{
		SmartTreeViewItem* item = new SmartTreeViewItem(owner, name, action, icon);
		addSubItem(item);
		return item;
	}

	virtual void forceSelection(bool force)
	{
		setSelected(force, true);
	}
	virtual void forceParentSelection(bool force)
	{
		juce::TreeViewItem* parent = getParentItem();
		if(parent)
		{
			parent->setSelected(force, true);
		}
	}
    juce::String getUniqueName() const
    {
        return name;
    }

	void itemSelectionChanged(bool isSelected)
	{
		if(isSelected)
		{
			MenuItemBase::execute();
		}
		
	}    
	
    virtual bool mightContainSubItems()
    {
        return true;
    }


};


MenuTree::MenuTree() 
{

	setIndentSize(0);
	setDefaultOpenness(true);
	setOpenCloseButtonsVisible(false);
	setOpaque(false);

}
MenuTree::~MenuTree()
{
    deleteRootItem();
}


void MenuTree::fillWith(AbstractAction rootAction_)
{
	MenuBase::fillWith(rootAction_);

	deleteRootItem();

	if(!rootAction.empty())
	{
		juce::TreeViewItem* const root
			= new SmartTreeViewItem (*this, "Menu", rootAction, itemImage);

		setRootItem (root);

		root->setSelected(true, true);
	}
	
}
void MenuTree::resized()
{
	int h = (int)(1.5*getFontHeight());
	if(h != getIndentSize())
	{
		setIndentSize(h);//calls resized internally
	}
	else
	{
		juce::TreeView::resized();
	}

}