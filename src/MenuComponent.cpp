#include "MenuComponent.h"
#include <boost/bind.hpp>


void NullAbstractAction(AbstractMenuItem&){};

class MenuItem
{
protected:
	int index;
	juce::String name;
	const juce::Drawable* icon;
	AbstractAction action;
public:

    MenuItem (int index = -1, juce::String const& name = juce::String::empty, AbstractAction action = NullAbstractAction, const juce::Drawable* icon = nullptr)
	:index(index)
	,name(name)
	,icon(icon)
	,action(action)
	{
	}

    MenuItem (MenuItem const& other)
	:index(0)
	,name(juce::String::empty)
	,icon(nullptr)
	,action(NullAbstractAction)
	{
		operator=(other);
	}

	virtual ~MenuItem()
	{
	}
	
    MenuItem& operator= (MenuItem const& other)
	{
		index=other.index;
		name=other.name;
		icon=other.icon;
		action=other.action;
		return *this;
	}
	virtual const juce::Drawable* getIcon(){ return icon;}
	virtual juce::String const& getName(){ return name;}
	virtual void execute(AbstractMenuItem & m){ return action(m);}
	
};



class TransparentListBox : public virtual juce::ListBox
{
public:
	TransparentListBox(const juce::String& componentName = juce::String::empty,
             juce::ListBoxModel* model = nullptr) : juce::ListBox(componentName, model)
	{
		setColour(backgroundColourId, juce::Colour());
	}
	void paint (juce::Graphics& g)
	{
	}
};


class MenuItemList : public juce::ListBoxModel
{
public:
	typedef boost::function<void (int)> SelectionCallback;
private:
	TransparentListBox box;
	juce::Array<MenuItem> items;
	bool isShortcut;
	SelectionCallback listBoxSelectionCallback;
public:

	MenuItemList(juce::String const& name,SelectionCallback listBoxSelectionCallback, bool isShortcut)
		:box(name, this)
		,isShortcut(isShortcut)
		,listBoxSelectionCallback(listBoxSelectionCallback)
	{
	}
	virtual ~MenuItemList()
	{
	}

	
	juce::ListBox * getListBox()
	{
		return &box;
	}
	
    // implements the ListBoxModel method
    int getNumRows()
    {
        return items.size();
    }

    // implements the ListBoxModel method
    void paintListBoxItem (int rowNumber,
                           juce::Graphics& g,
                           int width, int height,
                           bool rowIsSelected)
    {
        if (rowNumber < 0 || rowNumber>= items.size())
		{
			return;
		}
		MenuItem& item = items[rowNumber];
		paintMenuItem(g, width, height, rowIsSelected, item.getName(), item.getIcon(), isShortcut);
    }
	
    void selectedRowsChanged (int lastRowselected)
	{
		listBoxSelectionCallback(lastRowselected);
	}
    virtual void add(MenuItem const& item)
	{
		items.add(item);
        box.updateContent();
	}
	
	MenuItem* getItem(int rowNumber)
	{
		if (rowNumber < 0 || rowNumber>= getNumRows())
		{
			return nullptr;
		}
		return &(items.getReference(rowNumber));
	}
	
	MenuItem* getSelectedItem()
	{
		return getItem(box.getSelectedRow());
	}
	void removeItemsAfter(int rowNumber)
	{
		//menu is likely to have changed
		items.removeRange(rowNumber +1, items.size()-1-rowNumber);
		box.updateContent();
	}
	void clear()
	{
		items.clear();
		box.updateContent();
	}
	
	void selectLastItem(juce::NotificationType type)
	{
		int index = getNumRows()-1;
		juce::SparseSet<int> set;
		set.addRange (juce::Range<int>(index, index));
		getListBox()->setSelectedRows(set, type);
	}
};
MenuComponent::MenuComponent() 
	: menuList(new MenuItemList("MenuList", boost::bind(&MenuComponent::menuItemSelected, this, _1),false))
	, recentList(new MenuItemList("RecentList", boost::bind(&MenuComponent::recentItemSelected, this, _1),true))
{
    addAndMakeVisible (recentList->getListBox());
    addAndMakeVisible (menuList->getListBox());
	setOpaque(false);
}
MenuComponent::~MenuComponent()
{
}


void MenuComponent::fillWith(AbstractAction rootAction_)
{
	if(!rootAction_.empty())
	{
		rootAction_(*this);
        menuList->getListBox()->updateContent();
		recentList->add(MenuItem(0, "Menu", rootAction_, itemImage));
	}
	
}

void MenuComponent::paint (juce::Graphics& g)
{
	MenuBase::paintMenuBackGround(g);
}
void MenuComponent::resized()
{
	menuList->getListBox()->setRowHeight((int)getItemHeight());
	recentList->getListBox()->setRowHeight((int)getItemHeight());

	int recentSize = recentList->getNumRows()*(int)getItemHeight()+2;
	if(recentSize > getHeight()/2)
	{
		recentSize = getHeight()/2;
	}
	
	recentList->getListBox()->setBounds(0, 0, getWidth(), recentSize);
	menuList->getListBox()->setBounds((int)getItemHeight(), recentSize, getWidth()-(int)getItemHeight(), getHeight()-recentSize);
	Component::resized();
}

void MenuComponent::recentItemSelected(int /*lastRowselected*/)
{
	int row = recentList->getListBox()->getSelectedRow();
	MenuItem* item = recentList->getItem(row);
    if (item != nullptr)
	{
		recentList->removeItemsAfter(row);
		recentList->getListBox()->setSelectedRows(juce::SparseSet<int>());

		item->execute(*this);

		resized();
	}
}
	
void MenuComponent::menuItemSelected (int /*lastRowselected*/)
{
	MenuItem* item = menuList->getSelectedItem();
    if (item != nullptr)
	{
		//if(item->moveToRecentList())
		{
			MenuItem copy = *item;
			recentList->add(copy);
			menuList->clear();
			copy.execute(*this);
		}/*
	    else
		{
			item->execute(*this)
			if(item->refreshMenu())
			{
				recentList->selectLastItem(juce::sendNotificationAsync);
			}
		}*/
		resized();
	}
}

AbstractMenuItem* MenuComponent::addAction(juce::String const& name, AbstractAction action, const juce::Drawable* icon)
{
	menuList->add(MenuItem(menuList->getNumRows(), name, action, icon));
	return this;
}
void MenuComponent::focusItemAsMenuShortcut(/*AbstractMenuItem& it*/)
{
	//it must not be deleted...
}
void MenuComponent::forceSelection(/*AbstractMenuItem& it*/bool force)
{
	//??
}
void MenuComponent::forceParentSelection(bool force)
{
	recentList->selectLastItem(juce::sendNotificationAsync);
}
bool MenuComponent::isMenuShortcut()
{
	return false;
}