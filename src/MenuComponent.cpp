#include "MenuComponent.h"


void NullAbstractAction(AbstractMenuItem&){};

class MenuItem
{
protected:
	int index;
	juce::String name;
	const juce::Drawable* icon;
	AbstractAction action;
public:

    MenuItem (int index = -1, juce::String const& name = "", AbstractAction action = NullAbstractAction, const juce::Drawable* icon = nullptr)
	:index(index)
	,name(name)
	,icon(icon)
	,action(action){}

	virtual ~MenuItem()
	{
	}

	virtual const juce::Drawable* getIcon(){ return icon;}
	virtual juce::String const& getName(){ return name;}
	virtual void execute(AbstractMenuItem & m){ return action(m);}
	
};
class DefaultModel : public juce::ListBoxModel
{
protected:
	juce::Array<MenuItem> items;
	bool isShortcut;
public:
	DefaultModel(bool isShortcut):isShortcut(isShortcut)
	{
	}
	virtual ~DefaultModel()
	{
		items.clear();
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
};
class RecentList : public DefaultModel, public virtual juce::ListBox
{
	AbstractMenuItem &parent;
public:
	RecentList(AbstractMenuItem &parent)
		: DefaultModel(true)
		, juce::ListBox("RecentList", this)
		, parent(parent)
	{
		setColour(backgroundColourId, juce::Colour());
	}

	virtual ~RecentList()
	{
		items.clear();
        updateContent();
	}
	
	void paint (juce::Graphics& g)
	{
	}
	
    void selectedRowsChanged (int /*lastRowselected*/)
    {
		int rowNumber = getSelectedRow();
        if (rowNumber < 0 || rowNumber>= items.size())
		{
			return;
		}
		MenuItem& item = items[rowNumber];
		item.execute(parent);

		//menu is likely to have changed
		items.removeRange(rowNumber +1, items.size()-1-rowNumber);
        updateContent();
    }
    virtual void add(MenuItem const& item)
	{
		items.add(item);
        updateContent();
	}
	
};
class MenuList : public DefaultModel, public virtual juce::ListBox, public AbstractMenuItem
{
public:
	MenuList()
		: DefaultModel(false)
		, juce::ListBox("MenuList", this)
	{
		setColour(backgroundColourId, juce::Colour());
	}

	virtual ~MenuList()
	{
        updateContent();
	}
	
	void paint (juce::Graphics& g)
	{
	}
    // implements the ListBoxModel method
    int getNumRows()
    {
        return items.size();
    }

	
    virtual void add(MenuItem const& item)
	{
		items.add(item);
        updateContent();
	}
    virtual AbstractMenuItem* addAction(juce::String const& name, AbstractAction action, const juce::Drawable* icon = nullptr)
	{
		add(MenuItem(items.size(), name, action, icon));
		return this;
	}
	virtual void focusItemAsMenuShortcut()
	{
		items.clear();
        updateContent();
	}
	virtual void forceSelection(bool force = true)
	{
		//??
	}
	virtual void forceParentSelection(bool force = true)
	{
		//??
	}
	virtual bool isMenuShortcut()
	{
		return false;
	}
    void selectedRowsChanged (int /*lastRowselected*/)
    {
		int rowNumber = getSelectedRow();
        if (rowNumber < 0 || rowNumber>= items.size())
		{
			return;
		}
		MenuItem& item = items[rowNumber];
		item.execute(*this);
    }
};
MenuComponent::MenuComponent() 
	: menuList(new MenuList())
	, recentList(new RecentList(*menuList))
{
    addAndMakeVisible (recentList.get());
    addAndMakeVisible (menuList.get());
	setOpaque(false);
}
MenuComponent::~MenuComponent()
{
}


void MenuComponent::fillWith(AbstractAction rootAction_)
{
	if(!rootAction_.empty())
	{
		rootAction_(*menuList);
        menuList->updateContent();
		recentList->add(MenuItem(0, "Menu", rootAction_, itemImage));
	}
	
}

void MenuComponent::paint (juce::Graphics& g)
{
	MenuBase::paintMenuBackGround(g);
}
void MenuComponent::resized()
{
	menuList->setRowHeight((int)getItemHeight());

	int recentSize = recentList->getNumRows()*(int)getItemHeight()+2;
	if(recentSize > getHeight()/2)
	{
		recentSize = getHeight()/2;
	}
	
	recentList->setBounds(0, 0, getWidth(), recentSize);
	menuList->setBounds((int)getItemHeight(), recentSize, getWidth()-(int)getItemHeight(), getHeight()-recentSize);
	Component::resized();
}