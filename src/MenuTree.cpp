#include "MenuTree.h"

#include <modules\vf_core\vf_core.h>

class SmartTreeViewItem  : public juce::TreeViewItem, public AbstractMenuItem
{
	AbstractMenu& owner;
	bool m_isShortcut;
	juce::String name;
	const juce::Drawable* icon;
	AbstractAction action;
	ActionEffect actionEffect;
public:
    SmartTreeViewItem (AbstractMenu& owner, juce::String const& name, ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon = nullptr)
		:owner(owner)
		,m_isShortcut(false)
		,name(name)
		,icon(icon)
		,action(action)
		,actionEffect(actionEffect)
    {
	}

	virtual ~SmartTreeViewItem()
	{
	}

	virtual bool isMenuShortcut(){return m_isShortcut;}

	const juce::Drawable* getIcon(bool isItemSelected)
	{
		return  icon?icon:(m_isShortcut && !isItemSelected)?owner.getItemImage():nullptr;
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
		paintMenuItem(g, width, height, isSelected(), name, getIcon(isSelected()) ,m_isShortcut);
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
    AbstractMenuItem* addAction(juce::String const& name, ActionEffect actionEffect, AbstractAction action, const juce::Drawable* icon = nullptr)
	{
		SmartTreeViewItem* item = new SmartTreeViewItem(owner, name, actionEffect, action, icon);
		addSubItem(item);
		return item;
	}

	virtual void forceSelection(bool force)
	{
		setSelected(force, true);
	}
	virtual void forceParentSelection()
	{
		juce::TreeViewItem* parent = getParentItem();
		if(parent)
		{
			parent->setSelected(true, true, juce::sendNotificationAsync);
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
			if(!action.empty())
			{
				if(STORE_AND_OPEN_CHILDREN == actionEffect)
				{
					focusItemAsMenuShortcut();
				}
				action(*this);
				if(REFRESH_MENU == actionEffect)
				{
					forceParentSelection();
				}
			}
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


void MenuTree::fillWith(AbstractAction rootAction)
{

	deleteRootItem();

	if(!rootAction.empty())
	{
		juce::TreeViewItem* const root
			= new SmartTreeViewItem (*this, "Menu", AbstractMenuItem::STORE_AND_OPEN_CHILDREN, rootAction, itemImage);

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